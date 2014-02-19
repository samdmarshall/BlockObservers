//
//  SDMBlockObserverTypes.h
//  BlockObserver
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#ifndef BlockObserver_SDMBlockObserverTypes_h
#define BlockObserver_SDMBlockObserverTypes_h

#pragma mark -
#pragma mark Attributes
#define ATR_PACK __attribute__ ((packed))
#define ATR_FUNC(name) __attribute__ ((ifunc(name)))

#pragma mark -
#pragma mark System Includes
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>
#include <dispatch/dispatch.h>

#pragma mark -
#pragma mark Internal Types

typedef uintptr_t* Pointer;
typedef uintptr_t* (*FunctionPointer)();

#define k32BitMask 0xffffffff
#define k64BitMask 0xffffffffffffffff
#define k64BitMaskHigh 0xffffffff00000000
#define k64BitMaskLow 0x00000000ffffffff
#define PtrCastSmallPointer(a) (*(Pointer)&(a))
#define PtrHighPointer(a) (a & k64BitMaskHigh)
#define PtrLowPointer(a) (a & k64BitMaskLow)

#define Ptr(ptr) PtrCast(ptr,char*)
#define PtrCast(ptr, cast) ((cast)ptr)
#define PtrAdd(ptr, add) (Ptr(ptr) + (uint64_t)add)
#define PtrSub(ptr, sub) (Ptr(ptr) - (uint64_t)sub)

#define PtrDeref(type, pointer) (*((type) *)(pointer))

#pragma mark -
#pragma mark Return Blocks

typedef void (^BlockPointer)();

struct MethodCalls {
	Method originalCall;
	Method switchCall;
	__unsafe_unretained BlockPointer block;
};

struct ObserverArray {
	__unsafe_unretained dispatch_queue_t operationsQueue;
	struct MethodNames *array;
	uint32_t count;
};

struct MethodNames {
	char *keyName;
	char *getName;
	char *setName;
	BOOL isEnabled;
	struct MethodCalls getImpCalls;
	struct MethodCalls setImpCalls;
};


#endif
