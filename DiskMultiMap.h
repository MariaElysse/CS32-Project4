//
// Created by jeremy on 3/7/16.
//

#ifndef PROJECT4_DISKMULTIMAP_H
#define PROJECT4_DISKMULTIMAP_H
#define NULLOFFSET 0
#include <string>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap {
public:
    struct MultiMapNode {
        char m_key[121];
        char m_value[121];
        char m_context[121];
        BinaryFile::Offset m_next;
        bool deleted;
    };
    class Iterator {
    public:
        Iterator();

        Iterator(BinaryFile::Offset offset, BinaryFile *file);
        // You may add additional constructors
        bool isValid() const; //wat

        Iterator &operator++();

        MultiMapTuple operator*();

    private:
        BinaryFile *m_file;
        MultiMapNode m_this;
        BinaryFile::Offset m_next;
        char m_key[121];

        // Your private member declarations will go here
    };

    DiskMultiMap();

    ~DiskMultiMap();

    bool createNew(const std::string &filename, unsigned int numBuckets);

    bool openExisting(const std::string &filename);

    void close();

    bool insert(const std::string &key, const std::string &value, const std::string &context);

    Iterator search(const std::string &key);

    int erase(const std::string &key, const std::string &value, const std::string &context);

private:
    // Your private member declarations will go here
    BinaryFile m_file;

    struct SuperBlock {
        SuperBlock(unsigned long int m_numBuckets);

        unsigned long int m_numBuckets;
        BinaryFile::Offset m_DataStart;
        BinaryFile::Offset m_firstDeleted;
        BinaryFile::Offset m_lastNode; //todo: use the length of the file.
    };

    SuperBlock m_superBlock;

    BinaryFile::Offset hash(const std::string &key);
};

#endif //PROJECT4_DISKMULTIMAP_H


