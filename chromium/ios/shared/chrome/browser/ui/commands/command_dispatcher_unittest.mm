// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "ios/shared/chrome/browser/ui/commands/command_dispatcher.h"

#import <Foundation/Foundation.h>

#include "base/macros.h"
#include "testing/gtest/include/gtest/gtest.h"

#if !defined(__has_feature) || !__has_feature(objc_arc)
#error "This file requires ARC support."
#endif

#pragma mark - Test handlers

@protocol ShowProtocol<NSObject>
- (void)show;
- (void)showMore;
@end

// A handler with methods that take no arguments.
@interface CommandDispatcherTestSimpleTarget : NSObject<ShowProtocol>

// Will be set to YES when the |-show| method is called.
@property(nonatomic, assign) BOOL showCalled;

// Will be set to YES when the |-showMore| method is called.
@property(nonatomic, assign) BOOL showMoreCalled;

// Will be set to YES when the |-hide| method is called.
@property(nonatomic, assign) BOOL hideCalled;

// Resets the above properties to NO.
- (void)resetProperties;

// Handler methods.
- (void)hide;

@end

@implementation CommandDispatcherTestSimpleTarget

@synthesize showCalled = _showCalled;
@synthesize showMoreCalled = _showMoreCalled;
@synthesize hideCalled = _hideCalled;

- (void)resetProperties {
  self.showCalled = NO;
  self.showMoreCalled = NO;
  self.hideCalled = NO;
}

- (void)show {
  self.showCalled = YES;
}

- (void)showMore {
  self.showMoreCalled = YES;
}

- (void)hide {
  self.hideCalled = YES;
}

@end

// A handler with methods that take various types of arguments.
@interface CommandDispatcherTestTargetWithArguments : NSObject

// Set to YES when |-methodWithInt:| is called.
@property(nonatomic, assign) BOOL intMethodCalled;

// The argument passed to the most recent call of |-methodWithInt:|.
@property(nonatomic, assign) int intArgument;

// Set to YES when |-methodWithObject:| is called.
@property(nonatomic, assign) BOOL objectMethodCalled;

// The argument passed to the most recent call of |-methodWithObject:|.
@property(nonatomic, strong) NSObject* objectArgument;

// Resets the above properties to NO or nil.
- (void)resetProperties;

// Handler methods.
- (void)methodWithInt:(int)arg;
- (void)methodWithObject:(NSObject*)arg;
- (int)methodToAddFirstArgument:(int)first toSecond:(int)second;

@end

@implementation CommandDispatcherTestTargetWithArguments

@synthesize intMethodCalled = _intMethodCalled;
@synthesize intArgument = _intArgument;
@synthesize objectMethodCalled = _objectMethodCalled;
@synthesize objectArgument = _objectArgument;

- (void)resetProperties {
  self.intMethodCalled = NO;
  self.intArgument = 0;
  self.objectMethodCalled = NO;
  self.objectArgument = nil;
}

- (void)methodWithInt:(int)arg {
  self.intMethodCalled = YES;
  self.intArgument = arg;
}

- (void)methodWithObject:(NSObject*)arg {
  self.objectMethodCalled = YES;
  self.objectArgument = arg;
}

- (int)methodToAddFirstArgument:(int)first toSecond:(int)second {
  return first + second;
}

@end

#pragma mark - Tests

// Tests handler methods with no arguments.
TEST(CommandDispatcherTest, SimpleTarget) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target forSelector:@selector(show)];
  [dispatcher startDispatchingToTarget:target forSelector:@selector(hide)];

  [dispatcher show];
  EXPECT_TRUE(target.showCalled);
  EXPECT_FALSE(target.hideCalled);

  [target resetProperties];
  [dispatcher hide];
  EXPECT_FALSE(target.showCalled);
  EXPECT_TRUE(target.hideCalled);
}

// Tests handler methods that take arguments.
TEST(CommandDispatcherTest, TargetWithArguments) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestTargetWithArguments* target =
      [[CommandDispatcherTestTargetWithArguments alloc] init];

  [dispatcher startDispatchingToTarget:target
                           forSelector:@selector(methodWithInt:)];
  [dispatcher startDispatchingToTarget:target
                           forSelector:@selector(methodWithObject:)];
  [dispatcher
      startDispatchingToTarget:target
                   forSelector:@selector(methodToAddFirstArgument:toSecond:)];

  const int int_argument = 4;
  [dispatcher methodWithInt:int_argument];
  EXPECT_TRUE(target.intMethodCalled);
  EXPECT_FALSE(target.objectMethodCalled);
  EXPECT_EQ(int_argument, target.intArgument);

  [target resetProperties];
  NSObject* object_argument = [[NSObject alloc] init];
  [dispatcher methodWithObject:object_argument];
  EXPECT_FALSE(target.intMethodCalled);
  EXPECT_TRUE(target.objectMethodCalled);
  EXPECT_EQ(object_argument, target.objectArgument);

  [target resetProperties];
  EXPECT_EQ(13, [dispatcher methodToAddFirstArgument:7 toSecond:6]);
  EXPECT_FALSE(target.intMethodCalled);
  EXPECT_FALSE(target.objectMethodCalled);
}

// Tests that messages are routed to the proper handler when multiple targets
// are registered.
TEST(CommandDispatcherTest, MultipleTargets) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* showTarget =
      [[CommandDispatcherTestSimpleTarget alloc] init];
  CommandDispatcherTestSimpleTarget* hideTarget =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:showTarget forSelector:@selector(show)];
  [dispatcher startDispatchingToTarget:hideTarget forSelector:@selector(hide)];

  [dispatcher show];
  EXPECT_TRUE(showTarget.showCalled);
  EXPECT_FALSE(hideTarget.showCalled);

  [showTarget resetProperties];
  [dispatcher hide];
  EXPECT_FALSE(showTarget.hideCalled);
  EXPECT_TRUE(hideTarget.hideCalled);
}

// Tests handlers registered via protocols.
TEST(CommandDispatcherTest, ProtocolRegistration) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target
                           forProtocol:@protocol(ShowProtocol)];

  [dispatcher show];
  EXPECT_TRUE(target.showCalled);
  [dispatcher showMore];
  EXPECT_TRUE(target.showCalled);
}

// Tests that handlers are no longer forwarded messages after selector
// deregistration.
TEST(CommandDispatcherTest, SelectorDeregistration) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target forSelector:@selector(show)];
  [dispatcher startDispatchingToTarget:target forSelector:@selector(hide)];

  [dispatcher show];
  EXPECT_TRUE(target.showCalled);
  EXPECT_FALSE(target.hideCalled);

  [target resetProperties];
  [dispatcher stopDispatchingForSelector:@selector(show)];
  bool exception_caught = false;
  @try {
    [dispatcher show];
  } @catch (NSException* exception) {
    EXPECT_EQ(NSInvalidArgumentException, [exception name]);
    exception_caught = true;
  }
  EXPECT_TRUE(exception_caught);

  [dispatcher hide];
  EXPECT_FALSE(target.showCalled);
  EXPECT_TRUE(target.hideCalled);
}

// Tests that handlers are no longer forwarded messages after protocol
// deregistration.
TEST(CommandDispatcherTest, ProtocolDeregistration) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target
                           forProtocol:@protocol(ShowProtocol)];
  [dispatcher startDispatchingToTarget:target forSelector:@selector(hide)];

  [dispatcher show];
  EXPECT_TRUE(target.showCalled);
  EXPECT_FALSE(target.showMoreCalled);
  EXPECT_FALSE(target.hideCalled);
  [target resetProperties];
  [dispatcher showMore];
  EXPECT_FALSE(target.showCalled);
  EXPECT_TRUE(target.showMoreCalled);
  EXPECT_FALSE(target.hideCalled);

  [target resetProperties];
  [dispatcher stopDispatchingForProtocol:@protocol(ShowProtocol)];
  bool exception_caught = false;
  @try {
    [dispatcher show];
  } @catch (NSException* exception) {
    EXPECT_EQ(NSInvalidArgumentException, [exception name]);
    exception_caught = true;
  }
  EXPECT_TRUE(exception_caught);
  exception_caught = false;
  @try {
    [dispatcher showMore];
  } @catch (NSException* exception) {
    EXPECT_EQ(NSInvalidArgumentException, [exception name]);
    exception_caught = true;
  }
  EXPECT_TRUE(exception_caught);

  [dispatcher hide];
  EXPECT_FALSE(target.showCalled);
  EXPECT_FALSE(target.showMoreCalled);
  EXPECT_TRUE(target.hideCalled);
}

// Tests that handlers are no longer forwarded messages after target
// deregistration.
TEST(CommandDispatcherTest, TargetDeregistration) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* showTarget =
      [[CommandDispatcherTestSimpleTarget alloc] init];
  CommandDispatcherTestSimpleTarget* hideTarget =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:showTarget forSelector:@selector(show)];
  [dispatcher startDispatchingToTarget:hideTarget forSelector:@selector(hide)];

  [dispatcher show];
  EXPECT_TRUE(showTarget.showCalled);
  EXPECT_FALSE(hideTarget.showCalled);

  [dispatcher stopDispatchingToTarget:showTarget];
  bool exception_caught = false;
  @try {
    [dispatcher show];
  } @catch (NSException* exception) {
    EXPECT_EQ(NSInvalidArgumentException, [exception name]);
    exception_caught = true;
  }
  EXPECT_TRUE(exception_caught);

  [dispatcher hide];
  EXPECT_FALSE(showTarget.hideCalled);
  EXPECT_TRUE(hideTarget.hideCalled);
}

// Tests that an exception is thrown when there is no registered handler for a
// given selector.
TEST(CommandDispatcherTest, NoTargetRegisteredForSelector) {
  id dispatcher = [[CommandDispatcher alloc] init];
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target forSelector:@selector(show)];

  bool exception_caught = false;
  @try {
    [dispatcher hide];
  } @catch (NSException* exception) {
    EXPECT_EQ(NSInvalidArgumentException, [exception name]);
    exception_caught = true;
  }

  EXPECT_TRUE(exception_caught);
}

// Tests that -respondsToSelector returns YES for methods once they are
// dispatched for.
// Tests handler methods with no arguments.
TEST(CommandDispatcherTest, RespondsToSelector) {
  id dispatcher = [[CommandDispatcher alloc] init];

  EXPECT_FALSE([dispatcher respondsToSelector:@selector(show)]);
  CommandDispatcherTestSimpleTarget* target =
      [[CommandDispatcherTestSimpleTarget alloc] init];

  [dispatcher startDispatchingToTarget:target forSelector:@selector(show)];
  EXPECT_TRUE([dispatcher respondsToSelector:@selector(show)]);

  [dispatcher stopDispatchingForSelector:@selector(show)];
  EXPECT_FALSE([dispatcher respondsToSelector:@selector(show)]);

  // Actual dispatcher methods should still always advertise that they are
  // responded to.
  EXPECT_TRUE([dispatcher
      respondsToSelector:@selector(startDispatchingToTarget:forSelector:)]);
  EXPECT_TRUE(
      [dispatcher respondsToSelector:@selector(stopDispatchingForSelector:)]);
}
