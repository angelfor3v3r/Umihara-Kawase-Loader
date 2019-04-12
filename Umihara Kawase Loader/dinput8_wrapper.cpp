#include "includes.h"

namespace Dinput8Wrapper {
    //
    // global vars
    //

    // original dinput8.dll handle
    HMODULE g_orig_dinput8_dll = nullptr;

    // export funcs
    DirectInput8Create_t  g_orig_DirectInput8Create  = nullptr;
    DllCanUnloadNow_t     g_orig_DllCanUnloadNow     = nullptr;
    DllGetClassObject_t   g_orig_DllGetClassObject   = nullptr;
    DllRegisterServer_t   g_orig_DllRegisterServer   = nullptr;
    DllUnregisterServer_t g_orig_DllUnregisterServer = nullptr;
    GetdfDIJoystick_t     g_orig_GetdfDIJoystick     = nullptr;

    //
    // funcs
    //
    
    // set up export data
    NOINLINE bool init() {
        PWSTR path;

        // get location of original dinput8.dll
        const auto hr = SHGetKnownFolderPath( FOLDERID_System, 0, nullptr, &path );
        if( hr != S_OK )
            return false;

        const auto dinput8_dll_filename = std::wstring( path ) + L"\\dinput8.dll";

        // attempt to load original dll from system32
        g_orig_dinput8_dll = LoadLibraryW( dinput8_dll_filename.c_str() );
        if( !g_orig_dinput8_dll ) {
            CoTaskMemFree( path );

            return false;
        }

        // get all valid exports
        g_orig_DirectInput8Create  = (DirectInput8Create_t)  ( GetProcAddress( g_orig_dinput8_dll, "DirectInput8Create"  ) );
        g_orig_DllCanUnloadNow     = (DllCanUnloadNow_t)     ( GetProcAddress( g_orig_dinput8_dll, "DllCanUnloadNow"     ) );
        g_orig_DllGetClassObject   = (DllGetClassObject_t)   ( GetProcAddress( g_orig_dinput8_dll, "DllGetClassObject"   ) );
        g_orig_DllRegisterServer   = (DllRegisterServer_t)   ( GetProcAddress( g_orig_dinput8_dll, "DllRegisterServer"   ) );
        g_orig_DllUnregisterServer = (DllUnregisterServer_t) ( GetProcAddress( g_orig_dinput8_dll, "DllUnregisterServer" ) );
        g_orig_GetdfDIJoystick     = (GetdfDIJoystick_t)     ( GetProcAddress( g_orig_dinput8_dll, "GetdfDIJoystick"     ) );

        CoTaskMemFree( path );

        return true;
    }

    //
    // export funcs for dinput8.dll
    //

    extern "C" {
        HRESULT __stdcall DirectInput8Create_wrapper( HINSTANCE hinst, DWORD dwVersion, const IID &riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter ) {
            if( !g_orig_DirectInput8Create )
                return E_FAIL;

            return g_orig_DirectInput8Create( hinst, dwVersion, riidltf, ppvOut, punkOuter );
        }

        HRESULT __stdcall DllCanUnloadNow_wrapper() {
            if( !g_orig_DllCanUnloadNow )
                return E_FAIL;

            return g_orig_DllCanUnloadNow();
        }
        
        HRESULT __stdcall DllGetClassObject_wrapper( const IID &rclsid, const IID &riid, LPVOID *ppv ) {
            if( !g_orig_DllGetClassObject )
                return E_FAIL;

            return g_orig_DllGetClassObject( rclsid, riid, ppv );
        }
        
        HRESULT __stdcall DllRegisterServer_wrapper() {
            if( !g_orig_DllRegisterServer )
                return E_FAIL;

            return g_orig_DllRegisterServer();
        }
        
        HRESULT __stdcall DllUnregisterServer_wrapper() {
            if( !g_orig_DllUnregisterServer )
                return E_FAIL;

            return g_orig_DllUnregisterServer();
        }
        
        LPCDIDATAFORMAT __stdcall GetdfDIJoystick_wrapper() {
            if( !g_orig_GetdfDIJoystick )
                return nullptr;

            return g_orig_GetdfDIJoystick();
        }
    }
} // namespace Dinput8Wrapper