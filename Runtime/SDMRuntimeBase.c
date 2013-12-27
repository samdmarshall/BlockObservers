//
//  SDMRuntimeBase.c
//  Daodan
//
//  Created by Sam Marshall on 12/26/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMRuntimeBase_c
#define Daodan_SDMRuntimeBase_c

#include "SDMRuntimeBase.h"
#include <ctype.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
typedef struct objc_method MethodStruct;
#pragma clang diagnostic pop

// SDM: wtf, why are the headers saying Class and id when different SDK target is set?
#ifdef MAC_OS_X_VERSION_10_9
#define LookupClassWithName(className) objc_lookUpClass(className)
#define GetClassWithName(className) objc_getClass(className)
#else
#define LookupClassWithName(className) object_getClass(objc_lookUpClass(className))
#define GetClassWithName(className) object_getClass(objc_getClass(className))
#endif

#define SDMCreateGetter(ReturnType, obj, selector) ((ReturnType (*)(id, SEL))SDMGenericGetSetInterceptor)(obj, selector);
#define SDMCreateSetter(ReturnType, obj, selector, setValue) ((void (*)(id, SEL, ReturnType))SDMGenericGetSetInterceptor)(obj, selector, setValue);

BOOL SDMIsClassValid(char *className);

BOOL SDMCanRegisterForIvarInClass(char *ivarName, Class class);
BOOL SDMCanRegisterForPropertyInClass(char *propertyName, Class class);

extern void SDMGenericGetSetInterceptor_stret(void);
extern void SDMGenericGetSetInterceptor(void);

#pragma mark -
#pragma mark Private Calls

BOOL SDMIsClassValid(char *className) {
	BOOL validClass = FALSE;
	Class lookupResult = LookupClassWithName(className);
	if (lookupResult) {
		// SDM: prior to 10.0 this call would terminate the app if called the class didn't exist.
		Class fetchedClass = GetClassWithName(className);
		if (fetchedClass) {
			validClass = TRUE;
		}
	}
	return validClass;
}

BOOL SDMCanRegisterForIvarInClass(char *ivarName, Class class) {
	BOOL registerStatus = FALSE;
	Ivar ivar = class_getInstanceVariable(class, ivarName);
	if (ivar) {
		registerStatus = TRUE;
	}
	return registerStatus;
}

BOOL SDMCanRegisterForPropertyInClass(char *propertyName, Class class) {
	BOOL registerStatus = FALSE;
	objc_property_t property = class_getProperty(class, propertyName);
	if (property) {
		registerStatus = TRUE;
	}
	return registerStatus;
}

IMP observerResolveInstanceMethod(id self, SEL _cmd, ...) {
	IMP resolved = nil;
	printf("we are being called!");
	return resolved;
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
	unsigned long totalLength = sizeof(char)*(selLength+oLength);
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

static IMP whichIMP = NULL;

IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd) {
	
	printf("Firing notifications for %08x - %08x\n", self, _cmd);
	return whichIMP;
}

#pragma mark -
#pragma mark Public Calls

BOOL SDMRegisterCallbacksForKeyInInstance(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance) {
	BOOL registerStatus = FALSE;
	if ((getObserve && setObserve) && instance) {
		__block char *getName = 0x0;
		__block char *setName = 0x0;
		
		Class class = object_getClass(instance);
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
		
		struct MethodNames *originalMethods = calloc(0x1, sizeof(struct MethodNames));
		originalMethods->keyName = calloc(0x1, strlen(keyName));
		memcpy(originalMethods->keyName, keyName, strlen(keyName));
		originalMethods->getName = calloc(0x1, strlen(getName));
		memcpy(originalMethods->getName, getName, strlen(getName));
		originalMethods->setName = calloc(0x1, strlen(setName));
		memcpy(originalMethods->setName, setName, strlen(setName));
		
		id associatedObject = objc_getAssociatedObject(instance, instance);
		if (associatedObject) {
			BOOL existingObserverForKey = FALSE;
			struct ObserverArray *existingObservers = PtrCast(associatedObject, struct ObserverArray*);
			for (uint32_t i = 0x0; i < existingObservers->count; i++) {
				char *key = existingObservers->array[i].keyName;
				if (strncmp(keyName, key, strlen(keyName)) == 0x0) {
					existingObserverForKey = TRUE;
					// SDM: we have an existing observer registered.
					break;
				}
			}
			if (!existingObserverForKey) {
				existingObservers->array = realloc(existingObservers->array, sizeof(struct MethodNames)*(existingObservers->count+0x1));
				existingObservers->count++;
				memcpy(&(existingObservers->array[existingObservers->count-0x1]), originalMethods, sizeof(struct MethodNames));
			} else {
				return registerStatus;
			}
		} else {
			struct ObserverArray *observers = calloc(0x1, sizeof(struct ObserverArray));
			observers->array = originalMethods;
			observers->count = 0x1;
			objc_setAssociatedObject(instance, instance, PtrCast(observers, id), OBJC_ASSOCIATION_ASSIGN);
		}
		
		if (originalMethods->getName) {
			SEL realGetSelector = sel_registerName(originalMethods->getName);
			Method resolveGetter = class_getInstanceMethod(class, realGetSelector);
			
			//__block id originalGetter = imp_getBlock(method_getImplementation(resolveGetter));
			
			char *observerGetterName = SDMGenerateObserver(originalMethods->getName);
			SEL observerGetSelector = sel_registerName(observerGetterName);
			char *getMethodSignature = SDMGenerateMethodSignature(resolveGetter);
			
			id getSelectorBlock = (void*)^id(id self, ...){
				SEL originalGet = nil;
				struct ObserverArray *observers = (struct ObserverArray *)objc_getAssociatedObject(self, instance);
				if (observers && self == instance) {
					Method originalGetMethod = (Method)objc_getAssociatedObject(self, observers->array[0].setName);
					originalGet = method_getName(originalGetMethod);
					getObserve();
				}
				return ((id (*)(id, SEL))SDMGenericGetSetInterceptor)(self, originalGet);
			};
			
			IMP getSelector = imp_implementationWithBlock(getSelectorBlock);
			
			BOOL addObserverGetter = class_addMethod(class, observerGetSelector, getSelector, getMethodSignature);
			if (addObserverGetter) {
				Method getter = class_getInstanceMethod(class, observerGetSelector);
				Method observerGetter = calloc(0x1, sizeof(MethodStruct));
				memcpy(observerGetter, getter, sizeof(MethodStruct));
				method_exchangeImplementations(resolveGetter, observerGetter);
				objc_setAssociatedObject(instance, originalMethods->getName, PtrCast(observerGetter,id), OBJC_ASSOCIATION_ASSIGN);
			}
			registerStatus = addObserverGetter;
			free(observerGetterName);
		}

		
		if (originalMethods->setName) {
			SEL realSetSelector = sel_registerName(originalMethods->setName);
			Method resolveSetter = class_getInstanceMethod(class, realSetSelector);
			
			//__block id originalSetter = imp_getBlock(method_getImplementation(resolveSetter));
			
			char *observerSetterName = SDMGenerateObserver(originalMethods->setName);
			SEL observerSetSelector = sel_registerName(observerSetterName);
			char *setMethodSignature = SDMGenerateMethodSignature(resolveSetter);
			
			
			
			id setSelectorBlock = (void*)^id(id self, id arg){
				SEL originalSet = nil;
				struct ObserverArray *observers = (struct ObserverArray *)objc_getAssociatedObject(self, instance);
				if (observers && self == instance) {
					Method originalSetMethod = (Method)objc_getAssociatedObject(self, observers->array[0].setName);
					originalSet = method_getName(originalSetMethod);
					setObserve();
				}
				// ((void (*)(id, SEL, ))SDMGenericGetSetInterceptor)(self, originalSet, v);
				return nil;
			};
			
			IMP setSelector = imp_implementationWithBlock(setSelectorBlock);

			BOOL addObserverSetter = class_addMethod(class, observerSetSelector, setSelector, setMethodSignature);
			if (addObserverSetter) {
				Method setter = class_getInstanceMethod(class, observerSetSelector);
				Method observerSetter = calloc(0x1, sizeof(MethodStruct));
				memcpy(observerSetter, setter, sizeof(MethodStruct));
				method_exchangeImplementations(resolveSetter, observerSetter);
				objc_setAssociatedObject(instance, originalMethods->setName, PtrCast(observerSetter,id), OBJC_ASSOCIATION_ASSIGN);
			}
			registerStatus = addObserverSetter;
			free(observerSetterName);
		}
		
	}
	return registerStatus;
}

void SDMRemoveCallbackForKeyInInstance(char *keyName, id instance) {
	Class class = object_getClass(instance);
	id associatedObject = objc_getAssociatedObject(instance, instance);
	if (associatedObject) {
		struct ObserverArray *observers = PtrCast(associatedObject, struct ObserverArray*);
		
		for (uint32_t i = 0x0; i < observers->count; i++) {
			struct MethodNames *originalMethods = &(observers->array[i]);
			char *observerKey = originalMethods->keyName;
			if (strncmp(keyName, observerKey, strlen(keyName)) == 0x0) {
				objc_setAssociatedObject(instance, instance, nil, OBJC_ASSOCIATION_ASSIGN);
				
				id originalGetterValue = objc_getAssociatedObject(instance, originalMethods->getName);
				char *observerGetterName = PtrCast(originalGetterValue,char*);
				if (observerGetterName) {
					SEL observerGetSelector = sel_registerName(observerGetterName);
					Method observerGetter = class_getInstanceMethod(class, observerGetSelector);
					
					char *originalGetterName = SDMGenerateGetterName(keyName);
					SEL originalGetSelector = sel_registerName(originalGetterName);
					Method originalGetter = class_getInstanceMethod(class, originalGetSelector);
					
					method_exchangeImplementations(observerGetter, originalGetter);
					
					objc_setAssociatedObject(instance, originalMethods->getName, nil, OBJC_ASSOCIATION_ASSIGN);
					free(originalGetterName);
				}
				
				id originalSetterValue = objc_getAssociatedObject(instance, originalMethods->setName);
				char *observerSetterName = PtrCast(originalSetterValue,char*);
				if (observerSetterName) {
					SEL observerSetSelector = sel_registerName(observerSetterName);
					Method observerSetter = class_getInstanceMethod(class, observerSetSelector);
					
					char *originalSetterName = SDMGenerateSetterName(keyName);
					SEL originalSetSelector = sel_registerName(originalSetterName);
					Method originalSetter = class_getInstanceMethod(class, originalSetSelector);
					
					method_exchangeImplementations(observerSetter, originalSetter);
					
					objc_setAssociatedObject(instance, originalMethods->setName, nil, OBJC_ASSOCIATION_ASSIGN);
					free(originalSetterName);
				}
			}

		}
	}
}

#endif
