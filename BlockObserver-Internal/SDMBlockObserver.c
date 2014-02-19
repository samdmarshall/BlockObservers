//
//  SDMBlockObserver.c
//  BlockObserver
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#ifndef BlockObserver_SDMBlockObserver_c
#define BlockObserver_SDMBlockObserver_c

#include "SDMBlockObserver.h"
#include "cmap.h"

// SDM: SDMRuntimeBase.s calls
extern void SDMGenericGetSetInterceptor(void);

// SDM: this exists because we need the size of `Method`
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
typedef struct objc_method MethodStruct;
#pragma clang diagnostic pop

#define SDMGetSet (IMP)SDMGenericGetSetInterceptor

#define SDMCreateGetter(ReturnType, obj, selector) ((ReturnType (*)(id, SEL))SDMGenericGetSetInterceptor)(obj, selector)
#define SDMCallGetSetBlock(block) ((void (^)(id, SEL))block)

BOOL SDMCanRegisterForIvarInClass(char *ivarName, Class class);
BOOL SDMCanRegisterForPropertyInClass(char *propertyName, Class class);

extern IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd);

static cmap global_map = NULL;

#pragma mark -
#pragma mark Private Calls

BOOL SDMCanRegisterForIvarInClass(char *ivarName, Class class) {
	BOOL registerStatus = NO;
	Ivar ivar = class_getInstanceVariable(class, ivarName);
	if (ivar) {
		registerStatus = YES;
	}
	return registerStatus;
}

BOOL SDMCanRegisterForPropertyInClass(char *propertyName, Class class) {
	BOOL registerStatus = NO;
	objc_property_t property = class_getProperty(class, propertyName);
	if (property) {
		registerStatus = YES;
	}
	return registerStatus;
}

char* SDMGenerateGetterName(char *keyName) {
	unsigned long length = sizeof(char)*strlen(keyName);
	char *hasGet = calloc(0x1, length);
	memcpy(hasGet, keyName, length);
	return hasGet;
}

char* SDMGenerateObserver(char *selName) {
	char *observe = "Observer";
	unsigned long oLength = strlen(observe);
	unsigned long selLength = strlen(selName);
	unsigned long totalLength = sizeof(char)*(selLength+oLength+1);
	char *observerSelector = calloc(0x1, totalLength);
	memcpy(observerSelector, observe, oLength);
	memcpy(&(observerSelector[oLength]), selName, selLength);
	return observerSelector;
}

char* SDMGenerateSetterName(char *keyName) {
	unsigned long length = sizeof(char)*strlen(keyName);
	unsigned long totalLength = length+0x4;
	char *hasSet = calloc(0x1, totalLength);
	memcpy(hasSet, "set", 0x3);
	memcpy(&(hasSet[0x3]), keyName, length);
	hasSet[0x3] = toupper(hasSet[0x3]);
	memcpy(&(hasSet[totalLength-0x1]), ":", 0x1);
	return hasSet;
}

char* SDMGenerateMethodSignature(Method method) {
	unsigned int count = method_getNumberOfArguments(method);
	char *signature = calloc(0x1, sizeof(char)+0x2);
	method_getReturnType(method, signature, 0x2);
	
	for (unsigned int i = 0x0; i < count; i++) {
		unsigned int length = sizeof(char)*0x100;
		char *buffer = calloc(0x1, length);
		method_getArgumentType(method, i, buffer, length);
		unsigned long sigLength = sizeof(char)*(strlen(signature)+strlen(buffer));
		signature = realloc(signature, sigLength);
		strlcat(signature, buffer, sigLength);
		free(buffer);
	}
	return signature;
}

IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd) {
	char *originalSelector = (char*)sel_getName(_cmd);
	char *resolveSelector = NULL;
	BOOL observedSelector = NO;
	BOOL isGetter = NO;
	BOOL isSetter = NO;
	IMP originalImplementation;
	struct MethodCalls originalMethods;
	struct ObserverArray *observers = cmap_objectForKey(global_map, self);
	if (observers) {
		uint32_t observableCount = observers->count;
		uint32_t index;
		for (index = 0x0; index < observableCount; index++) {
			char *observableGetter = observers->array[index].getName;
			char *observableSetter = observers->array[index].setName;
			if (strncmp(originalSelector, observableGetter, strlen(originalSelector)) == 0x0) {
				observedSelector = isGetter = YES;
				break;
			}
			if (strncmp(originalSelector, observableSetter, strlen(originalSelector)) == 0x0) {
				observedSelector = isSetter = YES;
				break;
			}
		}
		if (isGetter) {
			originalMethods = observers->array[index].getImpCalls;
			SDMCallGetSetBlock(originalMethods->block)(self, _cmd);
		}
		
		if (isSetter) {
			originalMethods = observers->array[index].setImpCalls;
			SDMCallGetSetBlock(originalMethods->block)(self, _cmd);
		}
	}
	if (!observedSelector) {
		resolveSelector = SDMGenerateObserver(originalSelector);
		originalMethods.originalCall = class_getInstanceMethod((Class)object_getClass(self), sel_registerName(resolveSelector));
		originalMethods.switchCall = class_getInstanceMethod((Class)object_getClass(self), _cmd);
	}
	originalImplementation = method_getImplementation(originalMethods.originalCall);
	return originalImplementation;
}

#pragma mark -
#pragma mark Public Calls

void SDMInitializeCallbackObservers() {
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		global_map = cmap_new();
	});
}

BOOL SDMRegisterCallbacksForKeyInInstance(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance) {
	BOOL registerStatus = NO;
	BOOL registerGetStatus = NO;
	BOOL registerSetStatus = NO;
	if ((getObserve && setObserve) && instance) {
		__block char *getName = 0x0;
		__block char *setName = 0x0;
		
		Class class = objc_getClass(object_getClassName(instance));
		BOOL isProperty = SDMCanRegisterForPropertyInClass(keyName, class);
		if (isProperty) {
			objc_property_t property = class_getProperty(class, keyName);
			if (property) {
				
				char *hasGet = property_copyAttributeValue(property, "G");
				if (!hasGet) {
					hasGet = SDMGenerateGetterName(keyName);
				}
				getName = hasGet;
				
				char *isImmutable = property_copyAttributeValue(property, "R");
				if (!isImmutable) {
					char *hasSet = property_copyAttributeValue(property, "S");
					if (!hasSet) {
						hasSet = SDMGenerateSetterName(keyName);
					}
					setName = hasSet;
				}
			}
		} else {
			BOOL isIVar = SDMCanRegisterForIvarInClass(keyName, class);
			if (isIVar) {
				getName = SDMGenerateGetterName(keyName);
				setName = SDMGenerateSetterName(keyName);
			}
		}
		
		__block struct MethodNames *originalMethods;
		
		
		__block struct ObserverArray *existingObservers = cmap_objectForKey(global_map, instance);
		__block uint32_t index = 0x0;
		if (existingObservers) {
			__block BOOL existingObserverForKey = NO;
			dispatch_sync(existingObservers->operationsQueue, ^{
				for (index = 0x0; index < existingObservers->count; index++) {
					char *key = existingObservers->array[index].keyName;
					if (strncmp(keyName, key, strlen(keyName)) == 0x0) {
						existingObserverForKey = YES;
						// SDM: we have an existing observer registered.
						break;
					}
				}
				if (!existingObserverForKey) {
					if (index == existingObservers->count) {
						originalMethods = calloc(0x1, sizeof(struct MethodNames));
						originalMethods->keyName = calloc(0x1, strlen(keyName));
						memcpy(originalMethods->keyName, keyName, strlen(keyName));
						originalMethods->getName = calloc(0x1, strlen(getName));
						memcpy(originalMethods->getName, getName, strlen(getName));
						originalMethods->setName = calloc(0x1, strlen(setName));
						memcpy(originalMethods->setName, setName, strlen(setName));
						
						existingObservers->array = realloc(existingObservers->array, sizeof(struct MethodNames)*(existingObservers->count+0x1));
						existingObservers->count++;
						memcpy(&(existingObservers->array[existingObservers->count-0x1]), originalMethods, sizeof(struct MethodNames));
					}
				}
			});
			if (!existingObserverForKey) {
				if (index != existingObservers->count) {
					originalMethods = &(existingObservers->array[index]);
					if (originalMethods->isEnabled) {
						return registerStatus;
					}
				}
			} else {
				return registerStatus;
			}
		} else {
			originalMethods = calloc(0x1, sizeof(struct MethodNames));
			originalMethods->keyName = calloc(0x1, strlen(keyName));
			memcpy(originalMethods->keyName, keyName, strlen(keyName));
			originalMethods->getName = calloc(0x1, strlen(getName));
			memcpy(originalMethods->getName, getName, strlen(getName));
			originalMethods->setName = calloc(0x1, strlen(setName));
			memcpy(originalMethods->setName, setName, strlen(setName));
			
			existingObservers = calloc(0x1, sizeof(struct ObserverArray));
			char *operationsQueueName = calloc(0x1, 0x100);
			snprintf(operationsQueueName,  0x100, "%s-observer-operations-queue",class_getName(class));
			existingObservers->operationsQueue = dispatch_queue_create(operationsQueueName, DISPATCH_QUEUE_SERIAL);
			existingObservers->array = originalMethods;
			existingObservers->count = 0x1;
			cmap_setObjectForKey(global_map, instance, existingObservers);
		}
		
		if (originalMethods->getName) {
			SEL realGetSelector = sel_registerName(originalMethods->getName);
			Method resolveGetter = class_getInstanceMethod(class, realGetSelector);
			SEL observerGetSelector;
			
			if (resolveGetter) {
				char *observerGetterName = SDMGenerateObserver(originalMethods->getName);
				observerGetSelector = sel_registerName(observerGetterName);
				char *getMethodSignature = SDMGenerateMethodSignature(resolveGetter);
				
				IMP getSelector = SDMGetSet;
				
				struct MethodCalls *methods = calloc(0x1, sizeof(struct MethodCalls));
				methods->block = getObserve;
				methods->switchCall = resolveGetter;
				BOOL addObserverGetter = class_addMethod(class, observerGetSelector, getSelector, getMethodSignature);
				if (addObserverGetter) {
					Method getter = class_getInstanceMethod(class, observerGetSelector);
					method_exchangeImplementations(resolveGetter, getter);
					Method observerGetter = calloc(0x1, sizeof(MethodStruct));
					memcpy(observerGetter, getter, sizeof(MethodStruct));
					methods->originalCall = observerGetter;
					memcpy(&(existingObservers->array[index].getImpCalls), methods, sizeof(struct MethodCalls));
				} else {
					Method getTest = class_getInstanceMethod(class, observerGetSelector);
					if (getTest) {
						method_exchangeImplementations(resolveGetter, getTest);
						methods->originalCall = getTest;
						memcpy(&(existingObservers->array[index].getImpCalls), methods, sizeof(struct MethodCalls));
						addObserverGetter = YES;
					}
				}
				registerGetStatus = addObserverGetter;
				free(observerGetterName);
			}
		}
		
		if (originalMethods->setName) {
			SEL realSetSelector = sel_registerName(originalMethods->setName);
			Method resolveSetter = class_getInstanceMethod(class, realSetSelector);
			
			if (resolveSetter) {
				char *observerSetterName = SDMGenerateObserver(originalMethods->setName);
				SEL observerSetSelector = sel_registerName(observerSetterName);
				char *setMethodSignature = SDMGenerateMethodSignature(resolveSetter);
				
				IMP setSelector = SDMGetSet;
				
				struct MethodCalls *methods = calloc(0x1, sizeof(struct MethodCalls));
				methods->block = setObserve;
				methods->switchCall = resolveSetter;
				BOOL addObserverSetter = class_addMethod(class, observerSetSelector, setSelector, setMethodSignature);
				if (addObserverSetter) {
					Method setter = class_getInstanceMethod(class, observerSetSelector);
					method_exchangeImplementations(resolveSetter, setter);
					Method observerSetter = calloc(0x1, sizeof(MethodStruct));
					memcpy(observerSetter, setter, sizeof(MethodStruct));
					methods->originalCall = observerSetter;
					memcpy(&(existingObservers->array[index].setImpCalls), methods, sizeof(struct MethodCalls));
				} else {
					Method setTest = class_getInstanceMethod(class, observerSetSelector);
					if (setTest) {
						method_exchangeImplementations(resolveSetter, setTest);
						methods->originalCall = setTest;
						memcpy(&(existingObservers->array[index].setImpCalls), methods, sizeof(struct MethodCalls));
						addObserverSetter = YES;
					}
				}
				registerSetStatus = addObserverSetter;
				free(observerSetterName);
			}
		}
		if (registerGetStatus) {
			registerStatus = YES;
			originalMethods->isEnabled = registerGetStatus;
		}
	}
	return registerStatus;
}

void SDMRemoveCallbacksForKeyInInstance(char *keyName, id instance) {
	__block struct ObserverArray *objectObservers = cmap_objectForKey(global_map, instance);
	if (objectObservers) {
		dispatch_sync(objectObservers->operationsQueue, ^{
			for (uint32_t i = 0x0; i < objectObservers->count; i++) {
				struct MethodNames *originalMethods = &(objectObservers->array[i]);
				char *observerKey = originalMethods->keyName;
				if (strncmp(keyName, observerKey, strlen(keyName)) == 0x0) {
					
					method_exchangeImplementations(objectObservers->array[i].getImpCalls->switchCall, objectObservers->array[i].getImpCalls->originalCall);
					
					method_exchangeImplementations(objectObservers->array[i].setImpCalls->switchCall, objectObservers->array[i].setImpCalls->originalCall);
					
					originalMethods->isEnabled = NO;
				}
			}
		});
	}
}

#endif
