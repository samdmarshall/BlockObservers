#import <Foundation/Foundation.h>
#import <objc/runtime.h>

extern void SDMGenericGetSetInterceptor_stret(void);
extern void SDMGenericGetSetInterceptor(void);

typedef struct {
	uint64_t a, b, c, d, e, f, g, h;
} BigStruct;

static int IntGetter(id self, SEL _cmd)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	return 1;
}

static void IntSetter(id self, SEL _cmd, int intVal)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	NSLog(@"set %d", intVal);
}

static double DoubleGetter(id self, SEL _cmd)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	return 2.0;
}

static void DoubleSetter(id self, SEL _cmd, double doubleVal)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	NSLog(@"set %f", doubleVal);
}

static BigStruct BigStructGetter(id self, SEL _cmd)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	return (BigStruct){ 0, 1, 2, 3, 4, 5, 6, 7 };
}

static void BigStructSetter(id self, SEL _cmd, BigStruct bigStruct)
{
	NSLog(@"%@ - %@ - %s", self, NSStringFromSelector(_cmd), __PRETTY_FUNCTION__);
	NSLog(@"set %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu", bigStruct.a, bigStruct.b, bigStruct.c, bigStruct.d, bigStruct.e, bigStruct.f, bigStruct.g, bigStruct.h);
}

static IMP whichIMP = NULL;

extern IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd);
IMP SDMFireGetterSetterNotificationsAndReturnIMP(id self, SEL _cmd)
{
	NSLog(@"Firing notifications for %@ - %@", self, NSStringFromSelector(_cmd));
	return whichIMP;
}

@interface FakeClass : NSObject
@property(nonatomic,assign) int intVal;
@property(nonatomic,assign) double doubleVal;
@property(nonatomic,assign) BigStruct bigStruct;
@end

@implementation FakeClass
@dynamic intVal, doubleVal, bigStruct;
@end

int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[])
{
	@autoreleasepool {
		FakeClass *obj = [[FakeClass alloc] init];
		
		class_addMethod([FakeClass class], @selector(intVal), (IMP)SDMGenericGetSetInterceptor, "i@:");
		class_addMethod([FakeClass class], @selector(setIntVal:), (IMP)SDMGenericGetSetInterceptor, "v@:i");
		class_addMethod([FakeClass class], @selector(doubleVal), (IMP)SDMGenericGetSetInterceptor, "d@:");
		class_addMethod([FakeClass class], @selector(setDoubleVal:), (IMP)SDMGenericGetSetInterceptor, "v@:d");
		class_addMethod([FakeClass class], @selector(bigStruct), (IMP)SDMGenericGetSetInterceptor, "{?=QQQQQQQQ}@:");
		class_addMethod([FakeClass class], @selector(setBigStruct:), (IMP)SDMGenericGetSetInterceptor, "v@:{?=QQQQQQQQ}");
		
		whichIMP = (IMP)IntGetter;
		int v = obj.intVal;
		NSLog(@"Value: %u", v);

		whichIMP = (IMP)IntSetter;
		obj.intVal = v;

		whichIMP = (IMP)DoubleGetter;
		double d = obj.doubleVal;
		NSLog(@"Value: %f", d);

		whichIMP = (IMP)DoubleSetter;
		obj.doubleVal = d;

		whichIMP = (IMP)BigStructGetter;
		BigStruct b = obj.bigStruct;
		NSLog(@"Values: %llu, %llu, %llu, %llu, %llu, %llu, %llu, %llu", b.a, b.b, b.c, b.d, b.e, b.f, b.g, b.h);

		whichIMP = (IMP)BigStructSetter;
		obj.bigStruct = b;

		return 0;
	}
}
