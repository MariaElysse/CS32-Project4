//
// Created by jeremy on 3/7/16.
//

#ifndef PROJECT4_INTELWEB_H
#define PROJECT4_INTELWEB_H

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
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
    struct InteractionTupleLT{ //less object for ordering the set of bad interactions.
        bool operator() (const InteractionTuple& lhs, const InteractionTuple& rhs) const{
            if (lhs.context < rhs.context){ //if the machine is LT, the interactiontuple is LT
                return true;
            } else if (lhs.context == rhs.context) { //if the machine is EQ
                if (lhs.from < rhs.from){ //and the from is LT
                    return true; //the tuple is LT
                } else if (lhs.from == rhs.from){ //if the machine and the from are EQ
                    if (lhs.to < rhs.to){ //and the to is LT
                        return true; //the tuple is LT
                    }
                }
            }
            return false; //else the tuple is GTEQ.
        }
    };
    DiskMultiMap m_resultMap, m_actorMap;
    // Your private member declarations will go here
};

#endif //PROJECT4_INTELWEB_H
