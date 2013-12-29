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
#include <ctype.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>

//#define ObserverGetSetBlock(ObserverName, ReturnType) void (^ObserverName)(ReturnType) = ^(ReturnType arg)
#define ObserverGetSetBlock(ObserverName, ArgumentType) void (^ObserverName)(ArgumentType) = ^(ArgumentType arg)

typedef void (^BlockPointer)();

typedef char (^SDMcharBlock)(id self);
typedef int (^SDMintBlock)(id self);
typedef short (^SDMshortBlock)(id self);
typedef long (^SDMlongBlock)(id self);
typedef long long (^SDMlonglongBlock)(id self);
typedef unsigned char (^SDMunsignedcharBlock)(id self);
typedef unsigned int (^SDMunsignedintBlock)(id self);
typedef unsigned short (^SDMunsignedshortBlock)(id self);
typedef unsigned long (^SDMunsignedlongBlock)(id self);
typedef unsigned long long (^SDMunsignedlonglongBlock)(id self);
typedef float (^SDMfloatBlock)(id self);
typedef double (^SDMdoubleBlock)(id self);
typedef bool (^SDMboolBlock)(id self);
typedef char* (^SDMstringBlock)(id self);
typedef id (^SDMidBlock)(id self);
typedef Class (^SDMclassBlock)(id self);
typedef SEL (^SDMselBlock)(id self);
typedef Pointer (^SDMpointerBlock)(id self);

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
