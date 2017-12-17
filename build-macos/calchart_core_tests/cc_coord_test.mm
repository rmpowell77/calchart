//
//  cc_coord_tests.m
//  cc_coord_tests
//
//  Created by Richard Powell on 12/9/17.
//

#import <XCTest/XCTest.h>
#import "cc_coord.h"

@interface cc_coord_tests : XCTestCase

@end

@implementation cc_coord_tests

- (void)test_builtin {
    CalChart::Coord_UnitTests();
}

@end
