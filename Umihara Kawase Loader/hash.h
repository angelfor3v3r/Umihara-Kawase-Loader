#pragma once

#pragma warning( disable : 4307 ) // "'*': integral constant overflow"

#include "hash_base.h"

//
// default hash type
//

using hash32_t = FNV1aHash::T::HashBase< uint32_t >;

//
// helper macros for compile-time hashes of a string_view / wstring_view
// note: the lambda is a hacky fix for MSVC...
//

#define CT_HASH_32( str )                                 \
    []() {                                                \
        constexpr auto out = FNV1aHash::ct_get_32( str ); \
                                                          \
        return out;                                       \
    }()

//
// FNV-1a hash implementation
//

namespace FNV1aHash {
    //
    // hash types, etc
    //

    namespace T {
        //
        // FNV constants
        //

        constexpr uint32_t FNV_BASIS_32 = 0x811C9DC5;
        constexpr uint32_t FNV_PRIME_32 = 0x1000193;
    } // namespace T

    //
    // FNV-1a hash helpers and misc funcs
    //

    // hash a single byte
    // 32-bit hash
    FORCEINLINE constexpr hash32_t hash_byte_32( uint8_t value, uint32_t hash = T::FNV_BASIS_32 ) {
        return ( value ^ hash ) * T::FNV_PRIME_32;
    }

    // hash 2-byte value
    // 32-bit hash
    FORCEINLINE constexpr hash32_t hash_short_32( uint16_t value, uint32_t hash = T::FNV_BASIS_32 ) {
        // get low / high bytes then hash all bytes
        const auto lo = (uint8_t)( value & 0xFF );
        const auto hi = (uint8_t)( ( value >> 8 ) & 0xFF );
    
        hash = hash_byte_32( lo, hash );
        hash = hash_byte_32( hi, hash );
    
        return hash;
    }

    //
    // compile-time hashing
    //

    // 32-bit hash
    constexpr hash32_t ct_get_32( uint8_t *data, size_t len ) {
        hash32_t out = T::FNV_BASIS_32;

        for( size_t i = 0; i < len; ++i )
            out = hash_byte_32( data[ i ], out );

        return out;
    }

    constexpr hash32_t ct_get_32( std::string_view str ) {
        hash32_t out = T::FNV_BASIS_32;

        for( const auto &c : str )
            out = hash_byte_32( c, out );

        return out;
    }

    constexpr hash32_t ct_get_32( std::wstring_view wstr ) {
        hash32_t out = T::FNV_BASIS_32;

        for( const auto &wc : wstr )
            out = hash_short_32( wc, out );

        return out;
    }

    //
    // run-time hashing
    //

    // 32-bit hash
    FORCEINLINE hash32_t get_32( uint8_t *data, size_t len ) {
        hash32_t out = T::FNV_BASIS_32;

        for( size_t i = 0; i < len; ++i )
            out = hash_byte_32( data[ i ], out );

        return out;
    }

    FORCEINLINE hash32_t get_32( uint16_t *data, size_t len ) {
        hash32_t out = T::FNV_BASIS_32;

        for( size_t i = 0; i < len; ++i )
            out = hash_short_32( data[ i ], out );

        return out;
    }

    FORCEINLINE hash32_t get_32( std::string_view str ) {
        return get_32( (uint8_t *)str.data(), str.size() );
    }

    FORCEINLINE hash32_t get_32( std::wstring_view wstr ) {
        return get_32( (uint16_t *)wstr.data(), wstr.size() );
    }
} // namespace FNV1aHash