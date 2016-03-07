//
// Created by jeremy on 3/7/16.
//

#ifndef PROJECT4_INTELWEB_H
#define PROJECT4_INTELWEB_H

#include "InteractionTuple.h"
#include <string>
#include <vector>

class IntelWeb {
public:
    IntelWeb();

    ~IntelWeb();

    bool createNew(const std::string &filePrefix, unsigned int maxDataItems);

    bool openExisting(const std::string &filePrefix);

    void close();

    bool ingest(const std::string &telemetryFile);

    unsigned int crawl(const std::vector<std::string> &indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string> &badEntitiesFound,
                       std::vector<InteractionTuple> &interactions
    );

    bool purge(const std::string &entity);

private:
    // Your private member declarations will go here
};

#endif //PROJECT4_INTELWEB_H
