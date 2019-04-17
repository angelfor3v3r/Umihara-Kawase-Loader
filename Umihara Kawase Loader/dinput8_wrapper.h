#pragma once

#include "includes.h"

namespace Dinput8Wrapper {

    //
    // export func prototypes
    //

    using DirectInput8Create_t  = decltype( ::DirectInput8Create ) *;
    using DllCanUnloadNow_t     = decltype( ::DllCanUnloadNow ) *;
    using DllGetClassObject_t   = decltype( ::DllGetClassObject ) *;
    using DllRegisterServer_t   = decltype( ::DllRegisterServer ) *;
    using DllUnregisterServer_t = decltype( ::DllUnregisterServer ) *;
    using GetdfDIJoystick_t     = decltype( ::GetdfDIJoystick ) *;

    //
    // global vars
    //

    // original dinput8.dll handle
    extern HMODULE g_orig_dinput8_dll;

    // export funcs
    extern DirectInput8Create_t  g_orig_DirectInput8Create;
    extern DllCanUnloadNow_t     g_orig_DllCanUnloadNow;
    extern DllGetClassObject_t   g_orig_DllGetClassObject;
    extern DllRegisterServer_t   g_orig_DllRegisterServer;
    extern DllUnregisterServer_t g_orig_DllUnregisterServer;
    extern GetdfDIJoystick_t     g_orig_GetdfDIJoystick;

    //
    // funcs
    //

    // set up export data
    extern NOINLINE bool init();

} // namespace Dinput8Wrapper