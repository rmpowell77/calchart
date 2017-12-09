//
//  cc_show_tests.m
//  cc_show_tests
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_show.h"

@interface cc_show_tests : XCTestCase

@end

@implementation cc_show_tests

- (void)test_default_equal {
    auto empty_show = CalChart::show::Create_CC_show();
    XCTAssert(empty_show);
}

- (void)test_builtin {
    CalChart::show_UnitTests();
}

@end
