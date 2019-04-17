#pragma once

#include "build_pattern.h"

namespace PatternScan {

    // search for an IDA-style pattern in range
    extern NOINLINE uintptr_t find( uintptr_t start, size_t size, std::string_view pattern_str );

    //
    // templated funcs
    //

    // search for an IDA-style pattern in range
    template< typename t = uintptr_t > FORCEINLINE t find( uintptr_t start, size_t size, std::string_view pattern_str ) {
        return (t)( find( start, size, pattern_str ) );
    }

    // search for pattern in module with size
    template< typename t = uintptr_t > NOINLINE t find( std::string_view module_name, size_t size, std::string_view pattern_str ) {
        IMAGE_DOS_HEADER *dos;
        IMAGE_NT_HEADERS *nt;

        // get needed module info
        // we only need the start of the code section
        const auto base = (uintptr_t)( GetModuleHandleA( ( !module_name.empty() ) ? module_name.data() : 0 ) );
        if( !Utils::get_pe_file_headers( base, dos, nt ) )
            return {};

        const auto scan_start = Utils::RVA_to_ptr( base, nt->OptionalHeader.BaseOfCode );

        // find pattern and cast
        return find< t >( scan_start, size, pattern_str );
    }

    // search for pattern in entire module
    template< typename t = uintptr_t > FORCEINLINE t find( std::string_view module_name, std::string_view pattern_str ) {
        IMAGE_DOS_HEADER *dos;
        IMAGE_NT_HEADERS *nt;

        // get needed module info
        // we need the code section start and size
        const auto base = (uintptr_t)( GetModuleHandleA( ( !module_name.empty() ) ? module_name.data() : 0 ) );
        if( !Utils::get_pe_file_headers( base, dos, nt ) )
            return {};

        const auto scan_start = Utils::RVA_to_ptr( base, nt->OptionalHeader.BaseOfCode );
        const auto scan_size  = nt->OptionalHeader.SizeOfCode;

        // find pattern and cast
        return find< t >( scan_start, scan_size, pattern_str );
    }

} // namespace PatternScan