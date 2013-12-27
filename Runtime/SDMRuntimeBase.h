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
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>

typedef id(^ObserverBlock)(id self, ...);

struct MethodNames {
	char *getName;
	char *setName;
};

BOOL SDMRegisterCallbacksForKeyInInstance(ObserverBlock getBlock, ObserverBlock setBlock, char *keyName, id instance);
void SDMRemoveCallbackForKeyInInstance(char *keyName, id instance);

#endif
