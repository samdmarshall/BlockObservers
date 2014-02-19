//
//  cmap.cpp
//  KVO-Test
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#include "cmap.h"
#include <unordered_map>

cmap cmap_new(void) {
    return reinterpret_cast<cmap>(new std::unordered_map<void *, void *>);
}

void cmap_free(cmap *amap) {
    delete reinterpret_cast<std::unordered_map<void *, void *> *>(amap);
}

void* cmap_objectForKey(cmap *amap, void *key) {
    std::unordered_map<void *, void *> *realmap = reinterpret_cast<std::unordered_map<void *, void *> *>(amap);
    auto i = realmap->find(key);
    
    if (i != realmap->end()) {
        return (*i).second;
    }
    return NULL;
}

void cmap_setObjectForKey(cmap *amap, void *key, void *value) {
    std::unordered_map<void *, void *> *realmap = reinterpret_cast<std::unordered_map<void *, void *> *>(amap);
    auto i = realmap->find(key);
    
    if (i != realmap->end()) {
        if (value) {
            (*i).second = value;
        } else {
            realmap->erase(i);
        }
    } else {
        if (value) {
            realmap->insert(std::pair<void *, void *>(key, value));
        }
    }
}

size_t cmap_count(cmap *amap) {
    return reinterpret_cast<std::unordered_map<void *, void *> *>(amap)->size();
}