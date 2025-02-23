﻿#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vector>
#include <cmath>

#define SAFE_RELEASE(p)\
{\
    if (p != nullptr)\
    {\
        p->Release();\
        p = nullptr;\
    }\
}