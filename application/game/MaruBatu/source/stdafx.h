// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#ifdef _DEBUG
#define LOG_OUT_CLASS_SIZE
#define LOGGER_LEVEL LOGGER_LEVEL_DEBUGINFO
#else
#define LOGGER_LEVEL	LOGGER_LEVEL_NOTICE
#endif


// TODO: 在此处引用程序需要的其他头文件
#include <stdext.h>
