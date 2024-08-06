///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

//#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>

#ifdef _DEBUG
#define LOG_OUT_CLASS_SIZE
#define LOGGER_LEVEL LOGGER_LEVEL_DEBUGINFO
#else
#define LOGGER_LEVEL	LOGGER_LEVEL_NOTICE
#endif

// TODO: 在此处引用程序需要的其他头文件
#include <stdext.h>

