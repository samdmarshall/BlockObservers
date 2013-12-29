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
#include "SDMObjcLexer.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
typedef struct objc_method MethodStruct;
#pragma clang diagnostic pop

#define SDMCreateGetter(ReturnType, obj, selector) ((ReturnType (*)(id, SEL))SDMGenericGetSetInterceptor)(obj, selector)
#define SDMCreateSetter(ReturnType, obj, selector, setValue) ((void (*)(id, SEL, ReturnType))SDMGenericGetSetInterceptor)(obj, selector, setValue)

#define SDMObserverGetterBlock(BlockType, ReturnType, getObserve, instance, keyName) \
BlockType getSelectorBlock = ^ReturnType(id self){ \
	ReturnType getValue = 0x0; \
	SEL originalGet = nil; \
	struct ObserverArray *observers = (struct ObserverArray *)objc_getAssociatedObject(self, instance); \
	if (observers && self == instance) { \
		for (uint32_t i = 0x0; i < observers->count; i++) { \
			char *key = observers->array[i].keyName; \
			if (strncmp(keyName, key, strlen(keyName)) == 0x0) { \
				SEL realGetSelector = sel_registerName(observers->array[i].getName); \
				Method originalGetMethod = class_getInstanceMethod(class, realGetSelector); \
				originalGet = method_getName(originalGetMethod); \
				char *originalGetName = (char*)sel_getName(originalGet); \
				if (strncmp(observers->array[i].getName, originalGetName, strlen(observers->array[i].getName)) == 0x0) { \
					getValue = (ReturnType)SDMCreateGetter(ReturnType, self, originalGet); \
					getObserve(instance, getValue); \
				} \
			} \
		} \
	} \
	return getValue; \
}

#define SDMObserverSetterBlock(ReturnType, setObserve, instance, keyName) \
^(id self, ReturnType arg){ \
	SEL originalSet = nil; \
	struct ObserverArray *observers = (struct ObserverArray *)objc_getAssociatedObject(self, instance); \
	if (observers && self == instance) { \
		for (uint32_t i = 0x0; i < observers->count; i++) { \
			char *key = observers->array[i].keyName; \
			if (strncmp(keyName, key, strlen(keyName)) == 0x0) { \
				SEL realSetSelector = sel_registerName(observers->array[i].setName); \
				Method originalSetMethod = class_getInstanceMethod(class, realSetSelector); \
				originalSet = method_getName(originalSetMethod); \
				char *originalGetName = (char*)sel_getName(originalSet); \
				if (strncmp(observers->array[i].setName, originalGetName, strlen(observers->array[i].setName)) == 0x0) { \
					setObserve(instance, arg); \
					SDMCreateSetter(ReturnType, self, originalSet, arg); \
					break; \
				} else { \
					break; \
				} \
			} \
		} \
	} \
	return nil; \
}

BOOL SDMCanRegisterForIvarInClass(char *ivarName, Class class);
BOOL SDMCanRegisterForPropertyInClass(char *propertyName, Class class);

extern IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd);

// SDM: SDMRuntimeBase.s calls
extern void SDMGenericGetSetInterceptor_stret(void);
extern void SDMGenericGetSetInterceptor(void);

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

IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd) {
	char *originalSelector = (char*)sel_getName(_cmd);
	BOOL observedSelector = NO;
	BOOL isGetter = NO;
	BOOL isSetter = NO;
	IMP originalImplementation;
	Method originalMethod;
	struct ObserverArray *observers = (struct ObserverArray *)objc_getAssociatedObject(self, self);
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
			originalMethod = (Method)objc_getAssociatedObject(self, observers->array[index].getName);
		}
		
		if (isSetter) {
			originalMethod = (Method)objc_getAssociatedObject(self, observers->array[index].setName);
		}
	}
	if (!observedSelector) {
		originalMethod = class_getInstanceMethod(object_getClass(self), _cmd);
	}
	originalImplementation = method_getImplementation(originalMethod);	
	return originalImplementation;
}

#pragma mark -
#pragma mark Public Calls

BOOL SDMRegisterCallbacksForKeyInInstance(BlockPointer getObserve, BlockPointer setObserve, char *keyName, id instance) {
	BOOL registerStatus = NO;
	BOOL registerGetStatus = NO;
	BOOL registerSetStatus = NO;
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
			BOOL existingObserverForKey = NO;
			struct ObserverArray *existingObservers = PtrCast(associatedObject, struct ObserverArray*);
			for (uint32_t i = 0x0; i < existingObservers->count; i++) {
				char *key = existingObservers->array[i].keyName;
				if (strncmp(keyName, key, strlen(keyName)) == 0x0) {
					existingObserverForKey = YES;
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
						
			char *observerGetterName = SDMGenerateObserver(originalMethods->getName);
			SEL observerGetSelector = sel_registerName(observerGetterName);
			char *getMethodSignature = SDMGenerateMethodSignature(resolveGetter);
			
			IMP getSelector;
			struct SDMSTObjcType *decoded = SDMSTObjcDecodeType(PtrCast(method_getTypeEncoding(resolveGetter),char*)); \
			switch (decoded->token[0].typeClass) {
				case ObjcCharEncoding: {
					SDMObserverGetterBlock(SDMcharBlock, char, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcIntEncoding: {
					SDMObserverGetterBlock(SDMintBlock, int, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcShortEncoding: {
					SDMObserverGetterBlock(SDMshortBlock,short, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcLongEncoding: {
					SDMObserverGetterBlock(SDMlongBlock, long, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcLLongEncoding: {
					SDMObserverGetterBlock(SDMlonglongBlock, long long, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcUCharEncoding: {
					SDMObserverGetterBlock(SDMunsignedcharBlock, unsigned char, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcUIntEncoding: {
					SDMObserverGetterBlock(SDMunsignedintBlock, unsigned int, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcUShortEncoding: {
					SDMObserverGetterBlock(SDMunsignedshortBlock, unsigned short, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcULongEncoding: {
					SDMObserverGetterBlock(SDMunsignedlongBlock, unsigned long, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcULLongEncoding: {
					SDMObserverGetterBlock(SDMunsignedlonglongBlock, unsigned long long, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcFloatEncoding: {
					SDMObserverGetterBlock(SDMfloatBlock, float, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcDoubleEncoding: {
					SDMObserverGetterBlock(SDMdoubleBlock, double, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcBoolEncoding: {
					SDMObserverGetterBlock(SDMboolBlock, bool, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcStringEncoding: {
					SDMObserverGetterBlock(SDMstringBlock, char*, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcIdEncoding: {
					SDMObserverGetterBlock(SDMidBlock, id, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcClassEncoding: {
					SDMObserverGetterBlock(SDMclassBlock, Class, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcSelEncoding: {
					SDMObserverGetterBlock(SDMselBlock, SEL, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
				};
				case ObjcBitEncoding: {
					SDMObserverGetterBlock(SDMcharBlock, char, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
					
				};
				case ObjcPointerEncoding: {
					SDMObserverGetterBlock(SDMpointerBlock, Pointer, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
					
				};
				case ObjcStructEncoding: {
					break;
				};
				case ObjcArrayEncoding: {
					break;
				};
				default: {
					SDMObserverGetterBlock(SDMidBlock, id, getObserve, instance, keyName);
					getSelector = imp_implementationWithBlock(PtrCast(getSelectorBlock,id));
					break;
					break;
				};
			}
			
			BOOL addObserverGetter = class_addMethod(class, observerGetSelector, getSelector, getMethodSignature);
			if (addObserverGetter) {
				Method getter = class_getInstanceMethod(class, observerGetSelector);
				Method observerGetter = calloc(0x1, sizeof(MethodStruct));
				memcpy(observerGetter, getter, sizeof(MethodStruct));
				method_exchangeImplementations(resolveGetter, observerGetter);
				objc_setAssociatedObject(instance, originalMethods->getName, PtrCast(observerGetter,id), OBJC_ASSOCIATION_ASSIGN);
			}
			registerGetStatus = addObserverGetter;
			free(observerGetterName);
		}

		
		if (originalMethods->setName) {
			SEL realSetSelector = sel_registerName(originalMethods->setName);
			Method resolveSetter = class_getInstanceMethod(class, realSetSelector);
						
			char *observerSetterName = SDMGenerateObserver(originalMethods->setName);
			SEL observerSetSelector = sel_registerName(observerSetterName);
			char *setMethodSignature = SDMGenerateMethodSignature(resolveSetter);
			
			struct SDMSTObjcType *decoded = SDMSTObjcDecodeType(PtrCast(method_getTypeEncoding(resolveSetter),char*));
			void* setSelectorBlock;
			switch (decoded->token[0].typeClass) {
				case ObjcCharEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(char, setObserve, instance, keyName);
					break;
				};
				case ObjcIntEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(int, setObserve, instance, keyName);
					break;
				};
				case ObjcShortEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(short, setObserve, instance, keyName);
					break;
				};
				case ObjcLongEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(long, setObserve, instance, keyName);
					break;
				};
				case ObjcLLongEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(long long, setObserve, instance, keyName);
					break;
				};
				case ObjcUCharEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(unsigned char, setObserve, instance, keyName);
					break;
				};
				case ObjcUIntEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(unsigned int, setObserve, instance, keyName);
					break;
				};
				case ObjcUShortEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(unsigned short, setObserve, instance, keyName);
					break;
				};
				case ObjcULongEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(unsigned long, setObserve, instance, keyName);
					break;
				};
				case ObjcULLongEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(unsigned long long, setObserve, instance, keyName);
					break;
				};
				case ObjcFloatEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(float, setObserve, instance, keyName);
					break;
				};
				case ObjcDoubleEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(double, setObserve, instance, keyName);
					break;
				};
				case ObjcBoolEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(bool, setObserve, instance, keyName);
					break;
				};
				case ObjcVoidEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(id, setObserve, instance, keyName);
					break;
				};
				case ObjcStringEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(char*, setObserve, instance, keyName);
					break;
				};
				case ObjcIdEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(id, setObserve, instance, keyName);
					break;
				};
				case ObjcClassEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(Class, setObserve, instance, keyName);
					break;
				};
				case ObjcSelEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(SEL, setObserve, instance, keyName);
					break;
				};
				case ObjcBitEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(char, setObserve, instance, keyName);
					break;
				};
				case ObjcPointerEncoding: {
					setSelectorBlock = SDMObserverSetterBlock(Pointer, setObserve, instance, keyName);
					break;
				};
				case ObjcStructEncoding: {
					break;
				};
				case ObjcArrayEncoding: {
					break;
				};
				default: {
					setSelectorBlock = SDMObserverSetterBlock(id, setObserve, instance, keyName);
					break;
				};
			}
			IMP setSelector = imp_implementationWithBlock(setSelectorBlock);

			BOOL addObserverSetter = class_addMethod(class, observerSetSelector, setSelector, setMethodSignature);
			if (addObserverSetter) {
				Method setter = class_getInstanceMethod(class, observerSetSelector);
				Method observerSetter = calloc(0x1, sizeof(MethodStruct));
				memcpy(observerSetter, setter, sizeof(MethodStruct));
				method_exchangeImplementations(resolveSetter, observerSetter);
				objc_setAssociatedObject(instance, originalMethods->setName, PtrCast(observerSetter,id), OBJC_ASSOCIATION_ASSIGN);
			}
			registerSetStatus = addObserverSetter;
			free(observerSetterName);
		}
		if (registerGetStatus) {
			registerStatus = YES;
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
				
				id originalGetterValue = objc_getAssociatedObject(instance, originalMethods->getName);
				Method observerGetterMethod = PtrCast(originalGetterValue,Method);
				if (observerGetterMethod) {
					char *originalGetterName = SDMGenerateGetterName(keyName);
					SEL originalGetSelector;
					if (strncmp(originalGetterName, originalMethods->getName, strlen(originalMethods->getName)) == 0x0) {
						originalGetSelector = sel_registerName(originalGetterName);
					} else {
						originalGetSelector = sel_registerName(originalMethods->getName);
					}
					Method originalGetter = class_getInstanceMethod(class, originalGetSelector);
					
					method_exchangeImplementations(observerGetterMethod, originalGetter);
					
					objc_setAssociatedObject(instance, originalMethods->getName, nil, OBJC_ASSOCIATION_ASSIGN);
				}
				
				id originalSetterValue = objc_getAssociatedObject(instance, originalMethods->setName);
				Method observerSetterMethod = PtrCast(originalSetterValue,Method);
				if (observerSetterMethod) {
					char *originalSetterName = SDMGenerateSetterName(keyName);
					SEL originalSetSelector;
					if (strncmp(originalSetterName, originalMethods->setName, strlen(originalMethods->setName)) == 0x0) {
						originalSetSelector = sel_registerName(originalSetterName);
					} else {
						originalSetSelector = sel_registerName(originalMethods->setName);
					}
					Method originalSetter = class_getInstanceMethod(class, originalSetSelector);
					
					method_exchangeImplementations(observerSetterMethod, originalSetter);
					
					objc_setAssociatedObject(instance, originalMethods->setName, nil, OBJC_ASSOCIATION_ASSIGN);
				}
				
				// SDM: add keyName removal from associated object.
			}
		}
	}
}

#endif
