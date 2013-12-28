//
//  SDMObjcLexer.c
//  Daodan
//
//  Created by Sam Marshall on 11/3/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Daodan_SDMObjcLexer_c
#define Daodan_SDMObjcLexer_c

#include "SDMObjcLexer.h"


#define kObjcTypeEncodingCount 0x15

static char* ObjcTypeEncoding[kObjcTypeEncodingCount] = {
	kObjcCharEncoding,
	kObjcIntEncoding,
	kObjcShortEncoding,
	kObjcLongEncoding,
	kObjcLLongEncoding,
	kObjcUCharEncoding,
	kObjcUIntEncoding,
	kObjcUShortEncoding,
	kObjcULongEncoding,
	kObjcULLongEncoding,
	kObjcFloatEncoding,
	kObjcDoubleEncoding,
	kObjcBoolEncoding,
	kObjcVoidEncoding,
	kObjcStringEncoding,
	kObjcIdEncoding,
	kObjcClassEncoding,
	kObjcSelEncoding,
	kObjcBitEncoding,
	kObjcPointerEncoding,
	kObjcUnknownEncoding
};

static char* ObjcTypeEncodingNames[kObjcTypeEncodingCount] = {
	"char",
	"int",
	"short",
	"long",
	"long long",
	"unsigned char",
	"unsigned int",
	"unsigned short",
	"unsigned long",
	"unsigned long long",
	"float",
	"double",
	"bool",
	"void",
	"char*",
	"id",
	"Class",
	":",
	"bitmask",
	"*",
	"UnknownType"
};

#define kObjcContainerTypeEncodingCount 0x1

static char *ObjcContainerTypeEncodingNames[kObjcContainerTypeEncodingCount] = {
	"struct"
};

#define kObjcStackSizeCount 0xa

static char *ObjcStackSize[kObjcStackSizeCount] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9"
};

CoreRange SDMSTObjcStackSize(char *type, uint64_t offset, uint64_t *stackSize) {
	uint64_t counter = 0x0;
	bool findStackSize = true;
	while (findStackSize) {
		findStackSize = false;
		for (uint32_t i = 0x0; i < kObjcStackSizeCount; i++) {
			if (strncmp(&(type[offset+counter]), ObjcStackSize[i], sizeof(char)) == 0x0) {
				counter++;
				findStackSize = true;
				break;
			}
		}
	}
	CoreRange stackRange = CoreRangeCreate((uintptr_t)offset, counter);
	char *stack = calloc((uint32_t)stackRange.length+0x1, sizeof(char));
	memcpy(stack, &(type[offset]), (uint32_t)stackRange.length);
	*stackSize = atoi(stack);
	free(stack);
	return stackRange;
}

char* SDMSTObjcPointersForToken(struct SDMSTObjcLexerToken *token) {
	char *pointers = calloc(0x1, sizeof(char)*(token->pointerCount+1));
	if (token->pointerCount) {
		uint32_t i = 0x0;
		for (i = 0x0; i < token->pointerCount; i++) {
			pointers = strcat(pointers, "*");
		}
	}
	return pointers;
}

CoreRange SDMSTObjcGetTokenRangeFromOffset(char *type, uint64_t offset, char *token) {
	uint64_t counter = 0x0;
	while (strncmp(&(type[offset+counter]), token, strlen(token)) != 0x0 && offset+counter < strlen(type)) {
		counter++;
	}
	return CoreRangeCreate((uintptr_t)offset, counter);
}

char* SDMSTObjcCreateMethodDescription(struct SDMSTObjcType *type, char *name) {
	uint32_t nameLength = 0x1;
	if (name) {
		nameLength += strlen(name);
	} else {
		name = "";
	}
	char *description = calloc(0x1, sizeof(char)*(nameLength+0x3+strlen(type->token[0x0].type)));
	uint32_t counter = 0x0;
	uint32_t argCount = 0x0;
	for (uint32_t i = counter+0x3; i < type->tokenCount; i++) {
		argCount++;
	}
	if (counter != argCount) {
		sprintf(description,"(%s)",type->token[0x0].type);
		uint32_t offset = 0x0;
		while (counter < argCount) {
			if (offset) {
				description = realloc(description, sizeof(char)*(strlen(description)+0x2));
				sprintf(description,"%s ",description);
			}
			CoreRange methodArgRange = SDMSTObjcGetTokenRangeFromOffset(name, offset, kObjcSelEncoding);
			char *argName = calloc(0x1, sizeof(char)*((uint32_t)methodArgRange.length+0x1));
			memcpy(argName, &(name[offset]), (uint32_t)methodArgRange.length);
			uint32_t formatLength = (uint32_t)(0x8+strlen(argName)+strlen(type->token[counter+0x3].type)+GetDigitsOfNumber(counter));
			char *formatName = calloc(0x1, sizeof(char)*formatLength);
			sprintf(formatName,"%s:(%s)_arg%01i",argName,type->token[counter+0x3].type,counter);
			description = realloc(description, sizeof(char)*(strlen(description)+formatLength));
			memcpy(&(description[strlen(description)]), formatName, formatLength);
			free(formatName);
			free(argName);
			offset = offset + (uint32_t)methodArgRange.length + 0x1;
			counter++;
		}
	} else {
		sprintf(description,"(%s)%s",type->token[0x0].type,name);
	}
	description = realloc(description, sizeof(char)*(strlen(description)+0x2));
	sprintf(description,"%s;",description);
	return description;
}

CoreRange SDMSTObjcGetRangeFromTokens(char *startToken, char *endToken, char *type, uint64_t offset) {
	uint64_t stack = 0x1;
	uint64_t counter = 0x0;
	while (stack != 0x0) {
		if (strncmp(&(type[offset+counter]), startToken, sizeof(char)) == 0x0) {
			stack++;
		}
		if (strncmp(&(type[offset+counter]), endToken, sizeof(char)) == 0x0) {
			stack--;
		}
		counter++;
	}
	counter--;
	return CoreRangeCreate((uintptr_t)offset, counter);

}

CoreRange SDMSTObjcGetStructContentsRange(char *type, uint64_t offset) {
	return SDMSTObjcGetRangeFromTokens(kObjcStructTokenStart, kObjcStructTokenEnd, type, offset);
}

CoreRange SDMSTObjcGetArrayContentsRange(char *type, uint64_t offset) {
	return SDMSTObjcGetRangeFromTokens(kObjcArrayTokenStart, kObjcArrayTokenEnd, type, offset);
}

CoreRange SDMSTObjcGetStructNameRange(char *contents, uint64_t offset) {
	return SDMSTObjcGetTokenRangeFromOffset(contents, offset, kObjcStructDefinitionToken);
}

struct SDMSTObjcType* SDMSTMemberCountOfStructContents(char *structContents, CoreRange nameRange) {
	return SDMSTObjcDecodeTypeWithLength(structContents, nameRange.length);
}

uint32_t SDMSTParseToken(struct SDMSTObjcType *decode, char *type, uint64_t offset) {
	uint32_t parsedLength = 0x1;
	uint32_t index = k32BitMask;
	for (uint32_t i = 0x0; i < kObjcTypeEncodingCount; i++) {
		if (strncmp(&(type[offset]), ObjcTypeEncoding[i], sizeof(char)) == 0x0) {
			index = i;
			break;
		}
	}
	if (index != k32BitMask && index < kObjcTypeEncodingCount) {
		decode->token[decode->tokenCount].typeClass = SDMObjcLexerConvertIndexToToken(index);
		decode->token[decode->tokenCount].type = ObjcTypeEncodingNames[index];
		if (decode->token[decode->tokenCount].typeName == 0x0) {
			decode->token[decode->tokenCount].typeName = "";
		}
		switch (decode->token[decode->tokenCount].typeClass) {
			case ObjcCharEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcIntEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcShortEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcLongEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcLLongEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcUCharEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcUIntEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcUShortEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcULongEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcULLongEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcFloatEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcDoubleEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcBoolEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcVoidEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcStringEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcIdEncoding: {
				uint64_t next = offset+0x1;
				if (strncmp(&(type[next]), kObjcNameTokenStart, sizeof(char)) == 0x0) {
					CoreRange nameRange = SDMSTObjcGetTokenRangeFromOffset(type, next+0x1, kObjcNameTokenEnd);
					char *name = calloc(0x1, sizeof(char)*(0x3+(uint32_t)nameRange.length));
					char *objectProtocolTest = &(type[nameRange.offset]);
					if (strncmp(objectProtocolTest, "<", 0x1) == 0 && strncmp(objectProtocolTest+(uint32_t)(nameRange.length-0x1), ">", 0x1) == 0) {
						sprintf(&(name[0x0]),"id");
						memcpy(&(name[0x2]), &(type[nameRange.offset]), sizeof(char)*nameRange.length);
					} else {
						memcpy(name, &(type[nameRange.offset]), sizeof(char)*nameRange.length);
						sprintf(name,"%s*",name);
					}
					decode->token[decode->tokenCount].typeName = name;
					parsedLength += nameRange.length + 0x2;
				}
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcClassEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcSelEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcBitEncoding: {
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			case ObjcPointerEncoding: {
				decode->token[decode->tokenCount].pointerCount++;
				break;
			};
			case ObjcUnknownEncoding: {
				decode->token[decode->tokenCount].typeName = "";
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				break;
			};
			default: {
				break;
			};
		}
	} else {
		uint64_t stackSize;
		CoreRange stackRange = SDMSTObjcStackSize(type, offset, &stackSize);
		if (stackRange.length) {
			parsedLength = (uint32_t)stackRange.length;
		} else {
			if (strncmp(&(type[offset]), kObjcNameTokenStart, sizeof(char)) == 0x0) {
				CoreRange nameRange = SDMSTObjcGetTokenRangeFromOffset(type, offset+0x1, kObjcNameTokenEnd);
				char *name = calloc(0x1, sizeof(char)*((uint32_t)nameRange.length+0x100));
				memcpy(name, &(type[nameRange.offset]), sizeof(char)*nameRange.length);
				decode->token[decode->tokenCount].typeName = name;
				parsedLength += (uint32_t)nameRange.length + 0x1;
			}
			if (strncmp(&(type[offset]), kObjcPointerEncoding, sizeof(char)) == 0x0) {
				decode->token[decode->tokenCount].pointerCount++;
			}
			if (strncmp(&(type[offset]), kObjcUnknownEncoding, sizeof(char)) == 0x0) {
				decode->token[decode->tokenCount].typeName = "";
			}
			if (strncmp(&(type[offset]), kObjcStructTokenStart, sizeof(char)) == 0x0) {
				uint64_t next = offset+0x1;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				decode->token[decode->tokenCount].typeClass = ObjcStructEncoding;
				decode->token[decode->tokenCount].type = ObjcContainerTypeEncodingNames[0x0];
				CoreRange contentsRange = SDMSTObjcGetStructContentsRange(type, next);
				char *contents = calloc(0x1, sizeof(char)*((uint32_t)contentsRange.length+0x100));
				memcpy(contents, &(type[next]), contentsRange.length);
				CoreRange nameRange = SDMSTObjcGetStructNameRange(contents, 0x0);
				char *name = calloc(0x1, sizeof(char)*((uint32_t)nameRange.length+0x100));
				memcpy(name, &(contents[nameRange.offset]), sizeof(char)*nameRange.length);
				decode->token[decode->tokenCount].typeName = name;
				
				char *structContentString = &(contents[nameRange.offset+nameRange.length])+sizeof(char);
				CoreRange contentRange = CoreRangeCreate(0x0, strlen(structContentString));
				struct SDMSTObjcType *structContents = SDMSTMemberCountOfStructContents(structContentString, contentRange);
				decode->token[decode->tokenCount].childrenCount = structContents->tokenCount;
				decode->token[decode->tokenCount].children = calloc(structContents->tokenCount, sizeof(struct SDMSTObjcLexerToken));
				for (uint32_t i = 0x0; i < structContents->tokenCount; i++) {
					struct SDMSTObjcLexerToken *child = &(decode->token[decode->tokenCount].children[i]);
					struct SDMSTObjcLexerToken *structMember = &(structContents->token[i]);
					memcpy(child, structMember, sizeof(struct SDMSTObjcLexerToken));
				}
				parsedLength = (uint32_t)contentsRange.length + 0x1;
				free(structContents);
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
			}
			if (strncmp(&(type[offset]), kObjcArrayTokenStart, sizeof(char)) == 0x0) {
				uint64_t next = offset+0x1;
				uint64_t stackSize;
				CoreRange stackRange = SDMSTObjcStackSize(type, next, &stackSize);
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
				decode->token[decode->tokenCount].typeClass = ObjcArrayEncoding;
				decode->token[decode->tokenCount].arrayCount = (uint32_t)stackSize;
				next += stackRange.length;
				CoreRange arrayTypeRange = SDMSTObjcGetArrayContentsRange(type, next);
				char *arrayTypeString = calloc(0x1, sizeof(char)*(uint32_t)(arrayTypeRange.length+0x1));
				memcpy(arrayTypeString, &(type[arrayTypeRange.offset]), arrayTypeRange.length);
				struct SDMSTObjcType *arrayType = SDMSTObjcDecodeType(arrayTypeString);
				char *typeAssignment = ObjcTypeEncodingNames[SDMObjcLexerConvertTokenToIndex(ObjcUnknownEncoding)];
				if (arrayType->token[arrayType->tokenCount].type) {
					typeAssignment = arrayType->token[arrayType->tokenCount].type;
				}
				uint32_t typeLength = (uint32_t)strlen(typeAssignment);
				
				decode->token[decode->tokenCount].type = calloc(0x1, sizeof(char)*(typeLength+0x1));
				memcpy(decode->token[decode->tokenCount].type, typeAssignment, typeLength);
				
				decode->token[decode->tokenCount].childrenCount = 0x1;
				decode->token[decode->tokenCount].children = calloc(0x1, sizeof(struct SDMSTObjcLexerToken));
				memcpy(decode->token[decode->tokenCount].children, &(arrayType->token[0x0]), sizeof(struct SDMSTObjcLexerToken));
				uint64_t arrayTypeNameLength = strlen(arrayType->token[0x0].typeName);
				char *name = calloc(0x1, sizeof(char)*((uint32_t)arrayTypeNameLength+0x1));
				memcpy(name, arrayType->token[0x0].typeName, arrayTypeNameLength);
				decode->token[decode->tokenCount].typeName = name;
				parsedLength += arrayTypeRange.length + stackRange.length;
				free(arrayType);
				free(arrayTypeString);
				decode->tokenCount++;
				decode->token = realloc(decode->token, sizeof(struct SDMSTObjcLexerToken)*(decode->tokenCount+0x1));
			}
		}
	}
	return parsedLength;
}

struct SDMSTObjcType* SDMSTObjcDecodeTypeWithLength(char *type, uint64_t decodeLength) {
	struct SDMSTObjcType *decode = calloc(0x1, sizeof(struct SDMSTObjcType));
	decode->token = (struct SDMSTObjcLexerToken*)calloc(0x1, sizeof(struct SDMSTObjcLexerToken));
	uint64_t length = decodeLength;
	if (length) {
		uint64_t offset = 0x0;
		while (offset < length) {
			uint32_t parsedLength = SDMSTParseToken(decode, type, offset);
			offset = offset + parsedLength;
		}
		
	}
	return decode;
}

struct SDMSTObjcType* SDMSTObjcDecodeType(char *type) {
	return SDMSTObjcDecodeTypeWithLength(type, strlen(type));
}

#endif