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

int main(int argc, const char * argv[]) {
	@autoreleasepool {
		
		ObserverGetBlock(getter, NSString*){
			printf("testing property get\n");
			return @"testing!";
		};
		
		ObserverSetBlock(setter, NSString*){
			printf("testing property set\n");
		};
		
		MyClass *testClass = [MyClass new];
		BOOL result = SDMRegisterCallbacksForKeyInInstance((BlockPointer)getter, (BlockPointer)setter, "customName", testClass);
		[testClass performSelector:@selector(setCustomName:) withObject:@"hi!"];
		if (result) {
			//SDMRemoveCallbackForKeyInInstance("customName", testClass);
		}
		NSLog(@"%@",testClass.customName);
		MyClass *newTest = [MyClass new];
		newTest.customName = @"bye!";
	}
    return 0;
}

