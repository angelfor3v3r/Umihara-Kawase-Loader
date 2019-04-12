#pragma once

#include "includes.h"

static auto g_is_minhook_initialized = false;

//
// wrapper class for MinHook
//
template< typename t > class Detour {
private:
    uintptr_t m_target;
    void      *m_dest;
    t         m_orig;

public:
    // init minhook / vars
    NOINLINE Detour() : m_target{ 0 }, m_dest{ nullptr }, m_orig{ nullptr } {
        // check minhook status
        // init lib if we have to
        if( !g_is_minhook_initialized ) {
            const auto mhs = MH_Initialize(); 
            if( mhs == MH_OK )
                g_is_minhook_initialized = true;
        }
    }

    // get hook ready
    NOINLINE bool init( uintptr_t target, void *dest ) {
        t orig;

        // minhook not set up yet
        if( !g_is_minhook_initialized )
            return false;

        if( !target || !dest )
            return false;
        
        const auto mhs = MH_CreateHook( (void *)target, dest, (void **)&orig );
        if( mhs != MH_OK )
            return false;
        
        m_target = target;
        m_dest   = dest;
        m_orig   = orig;
        
        return true;
    }

    // enable hook
    NOINLINE bool enable() {
        // minhook not set up yet
        if( !g_is_minhook_initialized )
            return false;

        // not a valid hook
        if( !m_target || !m_dest || !m_orig ) {
            m_target = 0;
            m_dest   = nullptr;
            m_orig   = nullptr;

            return false;
        }

        // try to enable it
        if( MH_EnableHook( (void *)m_target ) != MH_OK ) {
            m_target = 0;
            m_dest   = nullptr;
            m_orig   = nullptr;

            return false;
        }

        g_log->info( L"Hooked function: 0x{:X} -> 0x{:X}", m_target, (uintptr_t)m_dest );
    
        return true;
    }

    // clear and remove hook
    NOINLINE bool remove() const {
        // no target, nothing to remove
        if( !m_target )
            return false;

        // try to disable the hook first
        if( MH_DisableHook( (void *)m_target ) != MH_OK )
            return false;

        // ... now remove it
        if( MH_RemoveHook( (void *)m_target ) != MH_OK )
            return false;

        // clear member vars
        m_target = 0;
        m_dest   = nullptr;
        m_orig   = nullptr;

        return true;
    }

    // returns original function
    FORCEINLINE t get_orig_func() const {
        return m_orig;
    }
};