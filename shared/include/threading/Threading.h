/*
 * File:   Threading.h
 * Author: Jehu Shaw
 *
 */

#ifndef _THREADING_H
#define _THREADING_H

// We need assertions.
#include "ShareErrors.h"

// Platform Specific Lock Implementation
#include "CriticalSection.h"
#include "ScopedLock.h"

// Platform Specific Thread Base
#include "ThreadBase.h"

// Thread Pool
#include "ThreadPool.h"

#endif

