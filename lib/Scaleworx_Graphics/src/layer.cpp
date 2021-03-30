//
//  layer.cpp
//  
//
//  Created for Scaleworx on 11/17/15.
//
//

#include "layer.h"

template <class T>
void Layer<T>::add(Layer<T> & newNext) {
    Layer * lastLayer = this;
    while (lastLayer->next) {
        lastLayer = lastLayer->next;
    };
    lastLayer->next = &newNext;
};
