//
// Created by jeremy on 3/7/16.
//
#include "DiskMultiMap.h"
#include <functional>
#include <string.h>

DiskMultiMap::DiskMultiMap() : m_superBlock(1) { }

DiskMultiMap::Iterator::Iterator() {
    m_file = nullptr;
    m_next = NULLOFFSET;
    m_valid = false;
}

DiskMultiMap::Iterator::Iterator(BinaryFile::Offset offset, const std::string &key, BinaryFile *file) {
    m_file = file;
    m_file->read(m_this, offset);
    strcpy(m_key, key.c_str());
    m_next = m_this.m_next;
    //strcpy(m_key, m_this.m_key);
    while (strcmp(m_this.m_key, m_key) != 0) {
        if (m_next == NULLOFFSET) {
            m_valid = false;
            return;
        }
        m_file->read(m_this, m_next);
        m_next = m_this.m_next;
    }
    m_valid = true;
}

DiskMultiMap::Iterator &DiskMultiMap::Iterator::operator++() {
    do {
        m_file->read(m_this, m_next);
        if (m_next == NULLOFFSET) {
            m_valid = false;
            return *this;
        }
        m_next = m_this.m_next;
    } while (strcmp(m_this.m_key, m_key) != 0);
    return *this;
}


MultiMapTuple DiskMultiMap::Iterator::operator*() {
    MultiMapTuple toBeRetrned;
    toBeRetrned.key = m_this.m_key;
    toBeRetrned.value = m_this.m_value;
    toBeRetrned.context = m_this.m_context;
    return toBeRetrned;
}

bool DiskMultiMap::Iterator::isValid() const {
    return m_valid;
} //probably have a static variable (hasSomethingBeenDeletedOrAdded) and return true in these cases

bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets) {
    m_superBlock = SuperBlock(numBuckets);
    if (!m_file.createNew(filename)) {
        return false;
    }
    if (!m_file.write(m_superBlock, 0)) {
        return false;
    }
    for (int i = 0; i < numBuckets; i++) {
        BinaryFile::Offset emptyOffset = NULLOFFSET;
        if (!m_file.write(emptyOffset, (BinaryFile::Offset) (i * sizeof(BinaryFile::Offset) + sizeof(SuperBlock)))) {
            return false;
        }
    }
    return true;
}

DiskMultiMap::SuperBlock::SuperBlock(unsigned long int numBuckets) {
    m_numBuckets = numBuckets;
    m_DataStart = (BinaryFile::Offset) (sizeof(*this) + sizeof(BinaryFile::Offset) *
                                                        m_numBuckets); //size of this superblock, plus size of each bucket * length of hasharray.
    m_firstDeleted = NULLOFFSET;
}


bool DiskMultiMap::openExisting(const std::string &filename) {
    m_file.close();
    if (m_file.openExisting(filename)) {
        if (m_file.read(m_superBlock, 0)) {
            return true;
        } else {
            std::cout << "File has no superblock. This is not a DiskMultiMap file." << std::endl;
        }
    } else {
        std::cout << "No such file:" << filename << ". Exiting." << std::endl;
    }
    return false;
}

void DiskMultiMap::close() {
    m_file.write(m_superBlock, 0);
    m_file.close();
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context) {
    /*DiskMultiMap CAN and WILL happily write duplicates. Deal with this separately! */
    if (key.size() > 120 || value.size() > 120 || context.size() > 120) {
        return false;
    }
    BinaryFile::Offset hashOffset = hash(key);
    BinaryFile::Offset valueInTable;
    m_file.read(valueInTable, hashOffset);
    MultiMapNode toBeInserted;
    strcpy(toBeInserted.m_key, key.c_str());
    strcpy(toBeInserted.m_value, value.c_str());
    strcpy(toBeInserted.m_context, context.c_str());
    toBeInserted.deleted = false;//fill the node that is to be inserted
    BinaryFile::Offset pointWrittenTo;
    if (m_superBlock.m_firstDeleted == NULLOFFSET) {
        pointWrittenTo = m_file.fileLength(); //write this into the hashtable
    } else {
        pointWrittenTo = m_superBlock.m_firstDeleted;
        MultiMapNode n;
        m_file.read(n, m_superBlock.m_firstDeleted);
        m_superBlock.m_firstDeleted = n.m_next; //TODO: ???this seems wrong?
    }
    if (valueInTable == NULLOFFSET) { //this can be simplified
        toBeInserted.m_next = NULLOFFSET;
    } else {
        toBeInserted.m_next = valueInTable;
    }
    m_file.write(toBeInserted, pointWrittenTo);
    m_file.write(pointWrittenTo, hashOffset);
    return true;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key) {
    BinaryFile::Offset node;
    BinaryFile::Offset hashNumber = hash(key);

    if (m_file.read(node, hashNumber) && node != NULLOFFSET)
        return Iterator(node, key, &m_file);
    else
        return Iterator(); //isnotvalid????? default constructor should return an invalid iterator then?
}

BinaryFile::Offset DiskMultiMap::hash(const std::string &key) {
    const std::hash<std::string> stdhash = std::hash<std::string>(); //this may return a negative number? be aware
    return (BinaryFile::Offset) (std::abs((long long int)stdhash(key)) % (m_superBlock.m_numBuckets) + sizeof(m_superBlock)); //superblock occurs at the start of the file.
}

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context) {
    BinaryFile::Offset toBeDeleted;
    if (!m_file.read(toBeDeleted, hash(key)) || toBeDeleted==NULLOFFSET)
        return 0;
    MultiMapNode nodeToBeDeleted;
    m_file.read(nodeToBeDeleted, toBeDeleted);

    BinaryFile::Offset next = hash(key);
    BinaryFile::Offset prevLink = hash(key);
    int numDeleted = 0;
    while (next != NULLOFFSET) {
        next = nodeToBeDeleted.m_next;
        if (nodeToBeDeleted.m_key == key && nodeToBeDeleted.m_context == context && nodeToBeDeleted.m_value == value) {
            nodeToBeDeleted.m_next = m_superBlock.m_firstDeleted;
            m_superBlock.m_firstDeleted = toBeDeleted;
            nodeToBeDeleted.deleted = true;
            numDeleted++;
            m_file.write(nodeToBeDeleted, toBeDeleted);

        }
        m_file.read(nodeToBeDeleted, next);
        nodeToBeDeleted.m_next = prevLink;
        prevLink = next;
        m_file.write(nodeToBeDeleted, next);
    }





    //m_file.read(nodeToBeDeleted, nodeToBeDeleted.m_next);
    //toBeDeleted = nodeToBeDeleted.m_next;
    //next = nodeToBeDeleted.m_next;


    //if (toBeDeleted.m_value == value && toBeDeleted.m_context == context){
    //compare the keys, if they're equal
    //get prev, set prev's next to next
    //get LastDeleted, set lt this's next to lastdeleted, set lastdeleted to this.
    //if a thing gets deleted (i.e. if I modify the lastDeleted) increment a variable that holds numDeleted
    //}
    //}
    return numDeleted; //return numDeleted
}

DiskMultiMap::~DiskMultiMap() {
    if (m_file.isOpen())
        close();

}
