//
//  main.m
//  KVO-Test
//
//  Created by Sam Marshall on 12/26/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SDMRuntimeBase.h"


@interface MyClass : NSObject
@property (nonatomic, strong) NSString *customName;
@property (getter = customIntWithNewGet, readwrite) int customInt;
@property (nonatomic, readwrite) struct teststruct mystruct;
@property (nonatomic, readonly) float customFloat;
@end

@implementation MyClass
@end

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		
		
		ObserverGetSetBlock(getter, MYSTRUCT){
			printf("testing property get %i\n",paramValue.number);
		};
		
		ObserverGetSetBlock(setter, MYSTRUCT){
			printf("testing property set from %i to %i\n",originalValue.number,paramValue.number);
		};
		
		MyClass *testClass = [MyClass new];
		BOOL result = SDMRegisterCallbacksForKeyInInstance(getter, setter, "mystruct", testClass);
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		__block struct teststruct hello;
		hello.name = "hello!";
		hello.number = 8;
		testClass.mystruct = hello;
		if (result) {
		//	SDMRemoveCallbackForKeyInInstance("customInt", testClass);
		}
		NSLog(@"%i",testClass.mystruct.number);
		MyClass *newTest = [MyClass new];
		newTest.customName = @"bye!";
	}
    return 0;
}

