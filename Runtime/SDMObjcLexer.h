//
//  SDMObjcLexer.h
//  Daodan
//
//  Created by Sam Marshall on 11/3/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcLexer_h
#define Daodan_SDMObjcLexer_h

#include "SDMHeader.h"

#define kObjcCharEncoding "c"
#define kObjcIntEncoding "i"
#define kObjcShortEncoding "s"
#define kObjcLongEncoding "l"
#define kObjcLLongEncoding "q"
#define kObjcUCharEncoding "C"
#define kObjcUIntEncoding "I"
#define kObjcUShortEncoding "S"
#define kObjcULongEncoding "L"
#define kObjcULLongEncoding "Q"
#define kObjcFloatEncoding "f"
#define kObjcDoubleEncoding "d"
#define kObjcBoolEncoding "B"
#define kObjcVoidEncoding "v"
#define kObjcStringEncoding "*"
#define kObjcIdEncoding "@"
#define kObjcClassEncoding "#"
#define kObjcSelEncoding ":"
#define kObjcBitEncoding "b"
#define kObjcPointerEncoding "^"
#define kObjcUnknownEncoding "?"

#define kObjcConstEncoding "r"
#define kObjcInEncoding "n"
#define kObjcInOutEncoding "N"
#define kObjcOutEncoding "o"
#define kObjcByCopyEncoding "O"
#define kObjcByRefEncoding "R"
#define kObjcOnewayEncoding "V"

enum SDMSTObjcLexerTokenType {
	ObjcCharEncoding = 0x1,
	ObjcIntEncoding,
	ObjcShortEncoding,
	ObjcLongEncoding,
	ObjcLLongEncoding,
	ObjcUCharEncoding,
	ObjcUIntEncoding,
	ObjcUShortEncoding,
	ObjcULongEncoding,
	ObjcULLongEncoding,
	ObjcFloatEncoding,
	ObjcDoubleEncoding,
	ObjcBoolEncoding,
	ObjcVoidEncoding,
	ObjcStringEncoding,
	ObjcIdEncoding,
	ObjcClassEncoding,
	ObjcSelEncoding,
	ObjcBitEncoding,
	ObjcPointerEncoding,
	ObjcUnknownEncoding,
	ObjcStructEncoding,
	ObjcArrayEncoding
};

#define kObjcNameTokenStart "\""
#define kObjcNameTokenEnd "\""

#define kObjcArrayTokenStart "["
#define kObjcArrayTokenEnd "]"

#define kObjcStructTokenStart "{"
#define kObjcStructTokenEnd "}"

#define kObjcStructDefinitionToken "="

#define SDMObjcLexerConvertIndexToToken(a) (enum SDMSTObjcLexerTokenType)(a+0x1)
#define SDMObjcLexerConvertTokenToIndex(a) (uint32_t)(a-0x1)

struct SDMSTObjcLexerToken {
	char *type;
	char *typeName;
	enum SDMSTObjcLexerTokenType typeClass;
	struct SDMSTObjcLexerToken *children;
	uint32_t childrenCount;
	uint32_t pointerCount;
	uint32_t arrayCount;
} ATR_PACK SDMSTObjcLexerToken;

struct SDMSTObjcType {
	struct SDMSTObjcLexerToken *token;
	uint32_t tokenCount;
} ATR_PACK SDMSTObjcType;

struct SDMSTObjcType* SDMSTObjcDecodeTypeWithLength(char *type, uint64_t decodeLength);
struct SDMSTObjcType* SDMSTObjcDecodeType(char *type);
CoreRange SDMSTObjcGetTokenRangeFromOffset(char *type, uint64_t offset, char *token);
char* SDMSTObjcPointersForToken(struct SDMSTObjcLexerToken *token);
char* SDMSTObjcCreateMethodDescription(struct SDMSTObjcType *type, char *name);

#endif
