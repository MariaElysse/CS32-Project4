//
// Created by jeremy on 3/7/16.
//

#ifndef PROJECT4_INTERACTIONTUPLE_H
#define PROJECT4_INTERACTIONTUPLE_H

#include <string>

struct InteractionTuple {
    InteractionTuple() { }

    InteractionTuple(const std::string &f, const std::string &t, const std::string &c)
            : from(f), to(t), context(c) { }

    std::string from;
    std::string to;
    std::string context;
};

#endif //PROJECT4_INTERACTIONTUPLE_H
