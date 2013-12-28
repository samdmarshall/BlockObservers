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
@property (nonatomic, readwrite) int customInt;
@property (nonatomic, readonly) float customFloat;
@end

@implementation MyClass
@end

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		
		ObserverGetBlock(getter, int){
			printf("testing property get\n");
			return 2;
		};
		
		ObserverSetBlock(setter, int){
			printf("testing property set\n");
		};
		
		MyClass *testClass = [MyClass new];
		BOOL result = SDMRegisterCallbacksForKeyInInstance(getter, setter, "customInt", testClass);
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		testClass.customInt = 5;
		if (result) {
			SDMRemoveCallbackForKeyInInstance("customInt", testClass);
		}
		NSLog(@"%i",testClass.customInt);
		MyClass *newTest = [MyClass new];
		newTest.customName = @"bye!";
	}
    return 0;
}

