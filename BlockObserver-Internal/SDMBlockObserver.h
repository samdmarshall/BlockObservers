//
//  SDMBlockObserver.h
//  BlockObserver
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#ifndef BlockObserver_SDMBlockObserver_h
#define BlockObserver_SDMBlockObserver_h

#include "SDMBlockObserverTypes.h"

void SDMInitializeCallbackObservers();
#define ObserverGetSetBlock(ObserverName) void (^ObserverName)(id, SEL) = ^(id self, SEL _cmd)
BOOL SDMRegisterCallbacksForKeyInInstance(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance);
void SDMRemoveCallbacksForKeyInInstance(char *keyName, id instance);

#endif
