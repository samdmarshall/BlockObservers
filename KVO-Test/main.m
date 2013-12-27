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
@property (nonatomic, readonly) float customFloat;
@end

@implementation MyClass
@end

#define customNameKey "customName"

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		id getter = ^ObserverBlock(id self, ...){
			printf("hello world, property get\n");
			return nil;
		};
		
		id setter = ^ObserverBlock(id self, ...){
			printf("hello world, property set\n");
			return nil;
		};
		
		MyClass *testClass = [MyClass new];
		BOOL result = SDMRegisterCallbacksForKeyInInstance(getter, setter, customNameKey, testClass);
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		NSLog(@"%@",testClass.customName);
		MyClass *newTest = [MyClass new];
		newTest.customName = @"bye!";
	}
    return 0;
}

