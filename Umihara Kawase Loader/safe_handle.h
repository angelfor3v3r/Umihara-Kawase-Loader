#pragma once

// handle type ID
enum class HandleID {
    SYSH_INVALID = 0,
    SYSH_NULL,
    SYSH_FILE_MAPPING,
    SYSH_GDI
};

// fwd declare
template< HandleID _id > class SafeHandleBase;

// handle type(s)
using SHandle     = SafeHandleBase< HandleID::SYSH_NULL >;         // null handle return
using SHandleI    = SafeHandleBase< HandleID::SYSH_INVALID >;      // "INVALID_HANDLE_VALUE" return
using SHandleFile = SafeHandleBase< HandleID::SYSH_FILE_MAPPING >; // for file mapping
using SHandleGDI  = SafeHandleBase< HandleID::SYSH_GDI >;          // GDI handles, etc

//
// RAII for windows handles
//

template< HandleID _id > class SafeHandleBase {
private:
    HANDLE m_handle;

    // init implementation
    FORCEINLINE void init( HANDLE handle ) {
        // check handle type
        if constexpr( _id == HandleID::SYSH_NULL ) {

        }

        else if( _id == HandleID::SYSH_INVALID ) {
            if( handle == INVALID_HANDLE_VALUE )
                return;
        }

        else if( _id == HandleID::SYSH_FILE_MAPPING ) {

        }

        else if( _id == HandleID::SYSH_GDI ) {

        }

        // set
        m_handle = handle;
    }

    // close implementation
    FORCEINLINE void close() const {
        // not valid
        if( !m_handle )
            return;

        // check handle type
        if constexpr( _id == HandleID::SYSH_NULL ) {
            CloseHandle( m_handle );
        }

        else if( _id == HandleID::SYSH_INVALID ) {
            CloseHandle( m_handle );
        }

        else if( _id == HandleID::SYSH_FILE_MAPPING ) {
            UnmapViewOfFile( m_handle );
        }

        else if( _id == HandleID::SYSH_GDI ) {
            DeleteObject( m_handle );
        }
    }

public:
    // ctors
    SafeHandleBase() = default;

    FORCEINLINE SafeHandleBase( HANDLE handle ) : m_handle{ nullptr } {
        init( handle );
    }

    // dtor
    FORCEINLINE ~SafeHandleBase() {
        close();
    }

    // return handle as t
    // null if invalid
    template< typename t = HANDLE > FORCEINLINE t get() const {
        return (t)m_handle;
    }

    // return handle
    // null if invalid
    FORCEINLINE operator HANDLE() const {
        return m_handle;
    }
};