#pragma once

#pragma comment( lib, "libMinHook.x86.lib" )

//
// include defs
//

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define _HAS_EXCEPTIONS 0
#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_INVALID_PARAMETER
#define _CRT_SECURE_NO_WARNINGS

#define SPDLOG_WCHAR_TO_UTF8_SUPPORT

#define FINI_WIDE_SUPPORT

// no min(a,b) / max(a,b) macros
#define NOMINMAX

//
// macros, macro defs, etc
//

// try to force func to not be inlined by the compiler
#define NOINLINE __declspec( noinline )

// export func
#define EXPORT __declspec( dllexport )

// set dinput version (ver 8)
#define DIRECTINPUT_VERSION 0x0800

//
// types
//

using ulong_t = unsigned long;

//
// windows / stl includes / etc
//

#include <Windows.h>
#include <ShlObj.h>
#include <Psapi.h>
#include <intrin.h>
#include <cstdint>
#include <iostream>
#include <cctype>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <optional>
#include <filesystem>
#include <fstream>

// dinput
#include <dinput.h>
#include <olectl.h>

// dependencies
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "INIReader.h"
#include "MinHook.h"

//
// globals vars, etc
//

// expose filesystem namespace
namespace std_fs = std::filesystem;

// spdlog logging
extern std::shared_ptr< spdlog::logger > g_log;

//
// other includes
//

// dinput related
#include "dinput8_wrapper.h"

// misc
#include "utils.h"
#include "pattern_scan.h"
#include "detour.h"
#include "sdk.h"