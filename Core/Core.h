//
//  Core.h
//  Core
//
//  Created by Sam Marshall on 12/8/13.
//  Copyright (c) 2013 Sam Marshall. All rights reserved.
//

#ifndef Core_Core_h
#define Core_Core_h

#pragma mark -
#pragma mark Attributes
#define ATR_PACK __attribute__ ((packed))
#define ATR_FUNC(name) __attribute__ ((ifunc(name)))

#pragma mark -
#pragma mark System Incues
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#pragma mark -
#pragma mark Core Includes
#include "Pointer.h"
#include "Range.h"
#include "Number.h"

#endif
