//
//  main.m
//  BlockObserver
//
//  Created by Sam Marshall on 2/19/14.
//  Copyright (c) 2014 Sam Marshall. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "SDMBlockObserver.h"

struct teststruct {
	char *name;
	int number;
} ATR_PACK;

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
		
		SDMInitializeCallbackObservers();
		
		ObserverGetSetBlock(getter){
			NSLog(@"%@ property get",self);
		};
		
		ObserverGetSetBlock(setter){
			NSLog(@"%@ property set",self);
		};
		
		MyClass *testClass = [[MyClass alloc] init];
		
		BOOL result = SDMRegisterCallbacksForKeyInInstance(getter, setter, "mystruct", testClass);
		
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		
		struct teststruct hello;
		hello.name = "hello!";
		hello.number = 8;
		testClass.mystruct = hello;
		
		NSLog(@"%i",testClass.mystruct.number);
		
		if (result) {
			SDMRemoveCallbacksForKeyInInstance("mystruct", testClass);
		}
		
		NSLog(@"%s",testClass.mystruct.name);
		
		result = SDMRegisterCallbacksForKeyInInstance(getter, setter, "customName", testClass);
		
		testClass.customName = @"hello world!";
		if (result) {
			SDMRemoveCallbacksForKeyInInstance("customName", testClass);
		}
		NSLog(@"%@",testClass.customName);
	}
    return 0;
}
