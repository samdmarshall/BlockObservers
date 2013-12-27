//
//  SDMRuntimeBase.h
//  Daodan
//
//  Created by Sam Marshall on 12/26/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMRuntimeBase_h
#define Daodan_SDMRuntimeBase_h

#include "SDMHeader.h"
#include <ctype.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>

#define ObserverGetBlock(ObserverName, ReturnType) ReturnType (^ObserverName)() = ^ReturnType()
#define ObserverSetBlock(ObserverName, ArgumentType) void (^ObserverName)(ArgumentType) = ^(ArgumentType param)

typedef void (^BlockPointer)();

struct ObserverArray {
	struct MethodNames *array;
	uint32_t count;
};

struct MethodNames {
	char *keyName;
	char *getName;
	char *setName;
};

// SDM: always use this to hide the block pointer casts.
#define SDMRegisterCallbacksForKeyInInstance(getObserve, setObserve, keyName, instance) SDMRegisterCallbacksForKeyInInstanceInternal((BlockPointer)getObserve, (BlockPointer)setObserve, keyName, instance)
BOOL SDMRegisterCallbacksForKeyInInstanceInternal(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance);

void SDMRemoveCallbackForKeyInInstance(char *keyName, id instance);

#endif
