//
//  SDMRuntimeBase.h
//  Daodan
//
//  Created by Sam Marshall on 12/26/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMRuntimeBase_h
#define Daodan_SDMRuntimeBase_h

#include "Core.h"
#include "SDMTypeRegister.h"

#define ObserverGetSetBlock(ObserverName, ArgumentType) void (^ObserverName)(id, ArgumentType, ArgumentType) = ^(id self, ArgumentType paramValue, ArgumentType originalValue)

struct ObserverArray {
	struct MethodNames *array;
	uint32_t count;
};

struct MethodNames {
	char *keyName;
	char *getName;
	char *setName;
	BOOL isEnabled;
};

BOOL SDMRegisterCallbacksForKeyInInstance(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance);

void SDMRemoveCallbackForKeyInInstance(char *keyName, id instance);

#endif
