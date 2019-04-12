#pragma once

#include "includes.h"

namespace Utils {
    // templated, relative virtual address (RVA) to pointer
    template< typename t = uintptr_t > static FORCEINLINE t RVA_to_ptr( uintptr_t base, uintptr_t rva ) {
        if( !base || !rva )
            return {};

        return (t)( base + rva );
    }

    // follow relative instruction displacement
    template< typename t = uintptr_t > static FORCEINLINE t follow_rel_instruction( uintptr_t op ) {
        if( !op )
            return {};

        const auto rel = *(int *)( op + 1 );
        if( !rel )
            return {};
        
        // skip 5-byte instruction to next
        // ... and add relative disp
        return (t)( ( op + 5 ) + rel );
    }

    //
    // funcs in source file
    //

    extern NOINLINE bool get_pe_file_headers( uintptr_t base, PIMAGE_DOS_HEADER &out_dos, PIMAGE_NT_HEADERS &out_nt );

} // namespace Utils