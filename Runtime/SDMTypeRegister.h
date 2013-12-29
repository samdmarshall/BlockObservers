//
//  SDMTypeRegister.h
//  KVO-Test
//
//  Created by Sam Marshall on 12/29/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef KVO_Test_SDMTypeRegister_h
#define KVO_Test_SDMTypeRegister_h

#include <ctype.h>
#include <objc/objc.h>
#include <objc/message.h>
#include <objc/runtime.h>

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

#define SDMCreateBlockType(ReturnType) typedef ReturnType (^SDM##ReturnType##Block)(id self);

#pragma mark -
#pragma mark User Created Types

struct teststruct {
	char *name;
	int number;
} ATR_PACK;

typedef struct teststruct MYSTRUCT;

SDMCreateBlockType(MYSTRUCT);

#endif
