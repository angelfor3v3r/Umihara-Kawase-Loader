#include "utils.h"

namespace Utils {

    NOINLINE bool get_pe_file_headers( uintptr_t base, PIMAGE_DOS_HEADER &out_dos, PIMAGE_NT_HEADERS &out_nt ) {
        if( !base )
            return false;

        // get DOS MZ header
        const auto dos = (IMAGE_DOS_HEADER *)base;
        if( dos->e_magic != IMAGE_DOS_SIGNATURE )
            return false;

        // get PE header
        const auto nt = (IMAGE_NT_HEADERS *)( (uintptr_t)dos + dos->e_lfanew );
        if( !nt || nt->Signature != IMAGE_NT_SIGNATURE )
            return false;

        // executable image files are required to have an optional header
        if( !nt->FileHeader.SizeOfOptionalHeader )
            return false;

        // check OptionalHeader magic num
        if( nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC )
            return false;

        out_dos = dos;
        out_nt  = nt;

        return true;
    }

} // namespace Utils