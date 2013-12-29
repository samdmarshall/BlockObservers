//
//  main.m
//  KVO-Test
//
//  Created by Sam Marshall on 12/26/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SDMRuntimeBase.h"


struct teststruct {
	char *name;
	int number;
} ATR_PACK;

typedef struct teststruct MYSTRUCT;

@interface MyClass : NSObject
@property (nonatomic, strong) NSString *customName;
@property (getter = customIntWithNewGet, readwrite) int customInt;
@property (nonatomic, readwrite) struct teststruct mystruct;
@property (nonatomic, readonly) float customFloat;
@end

@implementation MyClass {
	double testArray[5];
}
@end

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		
		ObserverGetSetBlock(getter, NSString*){
			NSLog(@"testing property get %@",paramValue);
		};
		
		ObserverGetSetBlock(setter, NSString*){
			NSLog(@"testing property set from %@ to %@",originalValue,paramValue);
		};
		
		MyClass *testClass = [MyClass new];
		BOOL result = SDMRegisterCallbacksForKeyInInstance(getter, setter, "customName", testClass);
		SDMRegisterCallbacksForKeyInInstance(getter, setter, "customName", testClass);
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		
		__block struct teststruct hello;
		hello.name = "hello!";
		hello.number = 8;
		testClass.mystruct = hello;
		
		if (result) {
			SDMRemoveCallbackForKeyInInstance("customName", testClass);
		}
		NSLog(@"%@",testClass.customName);
		MyClass *newTest = [MyClass new];
		newTest.customName = @"bye!";
	}
    return 0;
}

