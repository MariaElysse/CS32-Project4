//
// Created by jeremy on 3/7/16.
//
#include "DiskMultiMap.h"
#include <functional>
#include <string.h>

DiskMultiMap::DiskMultiMap() : m_superBlock(0) { }

DiskMultiMap::Iterator::Iterator() {
    m_file = nullptr;
    m_next = NULLOFFSET;
    //this is only used when I'm creating an invalid iterator. How do I say that an iterator is invalid?
}

DiskMultiMap::Iterator::Iterator(BinaryFile::Offset offset, BinaryFile *file) {
    m_file = file;
    do {
        m_file->read(m_this, offset);
        m_next = m_this.m_next;
        strcpy(m_key, m_this.m_key);
    } while (strcmp(m_key, m_this.m_key) != 0);
}

DiskMultiMap::Iterator &DiskMultiMap::Iterator::operator++() {
    do {
        m_file->read(m_this, m_next);
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
    return m_next != NULLOFFSET; //lel wat this is definitely wrong
} //probably have a static variable (hasSomethingBeenDeletedOrAdded) and return true in these cases

bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets) {
    m_superBlock = SuperBlock(numBuckets);
    BinaryFile::Offset hasharray[numBuckets];
    if (!m_file.createNew(filename)) {
        return false;
    }
    if (!m_file.write(m_superBlock, 0)) {
        return false;
    }
    for (int i = 0; i < numBuckets; i++) {
        BinaryFile::Offset emptyOffset = 0;
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
    m_lastNode = m_DataStart;
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
    m_file.close();
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context) {
    if (key.size() > 120 || value.size() > 120 || context.size() > 120) {
        return false;
    }
    BinaryFile::Offset offset = hash(key);
    MultiMapNode toBeInserted;
    strcpy(toBeInserted.m_key, key.c_str());
    strcpy(toBeInserted.m_value, value.c_str());
    strcpy(toBeInserted.m_context, context.c_str());
    toBeInserted.deleted = false;
    toBeInserted.m_next = NULLOFFSET; //fill the node that is to be inserted
    MultiMapNode currentNode;
    m_file.read(currentNode, offset); //read the head of the hashtable linkedlist
    while (offset != NULLOFFSET) { //until we get to the end
        m_file.read(currentNode, offset); //follow the linked list
        offset = currentNode.m_next;
    }
    if (m_superBlock.m_firstDeleted != NULLOFFSET) { //if there is a deleted node we can use
        currentNode.m_next = m_superBlock.m_firstDeleted; //the next node in the hashtable linked list will be that one.
        MultiMapNode pastDeleted;
        m_file.write(toBeInserted,
                     m_superBlock.m_firstDeleted); //write to the address location of the next node in the llist of deleted nodes
        m_file.read(pastDeleted,
                    m_superBlock.m_firstDeleted); //read the next thing in the linked list of deleted things
        m_superBlock.m_firstDeleted = pastDeleted.m_next; //set the head of the deleted things linked list to the next thing in the deleted things linked list
        return m_file.write(toBeInserted, currentNode.m_next); //write the thing to be added to the file.
    } else { //if there is no deleted node
        BinaryFile::Offset lastNode = m_superBlock.m_lastNode;
        m_superBlock.m_lastNode += sizeof(toBeInserted);
        return m_file.write(toBeInserted, lastNode);
    }
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key) {
    MultiMapNode node;
    BinaryFile::Offset hashNumber = hash(key);
    if (m_file.read(node, hashNumber))
        return Iterator(hashNumber, &m_file);
    else
        return Iterator(); //isnotvalid????? default constructor should return an invalid iterator then?
}

BinaryFile::Offset DiskMultiMap::hash(const std::string &key) {
    const std::hash<std::string> stdhash = std::hash<std::string>();
    return (BinaryFile::Offset) (stdhash(key) % m_superBlock.m_numBuckets +
                                 sizeof(m_superBlock)); //superblock occurs at the start of the file.
}

int DiskMultiMap::erase(const std::string &key, const std::string &value, const std::string &context) {
    BinaryFile::Offset toBeDeleted;
    if (!m_file.read(toBeDeleted, hash(key)))
        return 0;
    BinaryFile::Offset next = toBeDeleted;
    int numDeleted = 0;
    while (next != NULLOFFSET) {
        MultiMapNode nodeToBeDeleted;
        m_file.read(nodeToBeDeleted, toBeDeleted);
        if (nodeToBeDeleted.m_key == key) {
            m_superBlock.m_firstDeleted = toBeDeleted;
            next = nodeToBeDeleted.m_next;
            nodeToBeDeleted.deleted = true;
            nodeToBeDeleted.m_next = m_superBlock.m_firstDeleted;
            numDeleted++;
        } else {
            next = nodeToBeDeleted.m_next;
        }
        //if (toBeDeleted.m_value == value && toBeDeleted.m_context == context){
        //compare the keys, if they're equal
        //get prev, set prev's next to next
        //get LastDeleted, set lt this's next to lastdeleted, set lastdeleted to this.
        //if a thing gets deleted (i.e. if I modify the lastDeleted) increment a variable that holds numDeleted
        //}
    }
    return numDeleted; //return numDeleted
}

DiskMultiMap::~DiskMultiMap() {
    if (m_file.isOpen())
        close();

}
