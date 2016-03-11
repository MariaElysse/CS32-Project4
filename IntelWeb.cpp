//
// Created by jeremy on 3/7/16.
//
#include "IntelWeb.h"
#include "DiskMultiMap.h"
#include <map>
#include <queue>
#include <set>
#include <algorithm>
#include <list>
bool IntelWeb::ingest(const std::string &telemetryFile) {
    //close diskmaps
    std::ifstream file;
    file.open(telemetryFile);
    if (!file.good()){
        return false;
    }
    std::string machine, actor, result;
    while (file >> machine >> actor >> result) {
        if(!m_actorMap.insert(actor, result, machine))
            return false;
        if(!m_resultMap.insert(result, actor, machine))
            return false;
    }
    return true;
}

IntelWeb::IntelWeb() {

}

IntelWeb::~IntelWeb() {
    close();
}

bool IntelWeb::createNew(const std::string &filePrefix, unsigned int maxDataItems) {
    close();
    unsigned int tableLength = maxDataItems * 2; //gives a load factor of 0.5.
    return m_actorMap.createNew(filePrefix + "_actors.data", tableLength) && m_resultMap.createNew(filePrefix + "_results.data", tableLength);
}

bool IntelWeb::openExisting(const std::string &filePrefix) {
    close();
    return m_resultMap.openExisting(filePrefix + "_results.data") &&  m_actorMap.openExisting(filePrefix + "_actors.data");
}

void IntelWeb::close() {
    m_actorMap.close();
    m_resultMap.close();
}

unsigned int IntelWeb::crawl(const std::vector<std::string> &indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string> &badEntitiesFound, std::vector<InteractionTuple> &interactions) {
    std::set<InteractionTuple, InteractionTupleLT> badInteractions;
    badEntitiesFound.clear();
    interactions.clear();
    //I didn't even know this was allowed.
    struct Prevalences { //pseudo-generator. saves previously wanted prevalences.
        unsigned int getPrevalence(const std::string& key){
            if(prevalences.count(key)==0){ //there's only need to calculate this once.
                prevalences[key]=0;
                for (auto i= m_resultMap->search(key); i.isValid(); ++i){
                    ++prevalences[key]; //number of times this item appears in the map
                }
                for (auto i= m_actorMap->search(key); i.isValid(); ++i){
                    ++prevalences[key]; //both as an actor and as a result.
                }
            }
            return prevalences[key]; //return the calculated prevalence of that key.
            // technically may be zero, but unlikely to be zero if someone's asking.
        }
        Prevalences(DiskMultiMap* resultMap, DiskMultiMap* actorMap): m_resultMap(resultMap), m_actorMap(actorMap){}
        std::map <std::string, unsigned int> prevalences;
        DiskMultiMap* m_resultMap;
        DiskMultiMap* m_actorMap;
    };


    Prevalences prevalences(&m_resultMap, &m_actorMap);
    unsigned int numBadInteractions=0;

    std::queue<std::string> processingQueue;
    for (auto item : indicators){ //addeverything that we know to be bad to the queue to be processed.
        processingQueue.push(item);
    }
    while (!processingQueue.empty()){ //run through the queue
        std::string currentlyProcessing = processingQueue.front();
        processingQueue.pop();
        if (prevalences.getPrevalence(currentlyProcessing) >= minPrevalenceToBeGood){
            continue; //it's good.
        }
        //from here on we can assume that anything I have my hands on is bad.
        // The only way to know something is good is if it's very prevalent. However, that's not the case now.
        if (prevalences.getPrevalence(currentlyProcessing)>0) {
            badEntitiesFound.push_back(currentlyProcessing);
        }
        for (auto item = m_resultMap.search(currentlyProcessing); item.isValid(); ++item) {
            if ((*item).value != currentlyProcessing)
                processingQueue.push((*item).value); //everything this bad actor touches may be bad.
            badInteractions.insert(InteractionTuple((*item).value, (*item).key, (*item).context)); //this was probably not a good interaction
            ++numBadInteractions;
        }
        for (auto item = m_actorMap.search(currentlyProcessing); item.isValid(); ++item){
            if ((*item).value != currentlyProcessing) {
                processingQueue.push((*item).value); //anything that touched this actor is probably bad
            }
            badInteractions.insert(InteractionTuple((*item).key, (*item).value, (*item).context )); //and this was probably not a good interaction
            ++numBadInteractions;
        }
    }
    std::sort(badEntitiesFound.begin(), badEntitiesFound.end()); //sort the bad entities
    for (auto interaction: badInteractions){
        interactions.push_back(interaction); //insert everything in the (pre-sorted) set of bad interactions into the vector
    }



    return numBadInteractions;
}

bool IntelWeb::purge(const std::string &entity) {
    bool purged = false;
    std::list<MultiMapTuple> mappings;
    for (auto i=m_resultMap.search(entity); i.isValid(); ++i){
        mappings.insert(mappings.begin(), *i);
    }
    for (auto i=m_actorMap.search(entity); i.isValid(); ++i){
        mappings.insert(mappings.begin(), *i);
    }
    for (auto i : mappings){
        m_resultMap.erase(i.key, i.value, i.context);
        m_actorMap.erase(i.key, i.value, i.context);
        m_resultMap.erase(i.value, i.key, i.context);
        m_actorMap.erase(i.value, i.key, i.context);
        purged = true;
    }
    return purged;
}

