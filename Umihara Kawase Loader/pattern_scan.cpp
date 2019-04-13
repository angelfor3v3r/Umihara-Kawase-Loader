#include "pattern_scan.h"

namespace PatternScan {

    NOINLINE uintptr_t find( uintptr_t start, size_t size, std::string_view pattern_str ) {
        if( !start || !size || pattern_str.empty() )
            return 0;

        // convert pattern string to pattern object
        const auto pattern = Build::Pattern( pattern_str );
        if( !pattern )
            return 0;

        // get scan start and end
        const auto scan_start = (const uint8_t *)start;
        const auto scan_end   = scan_start + size;

        // search for pattern, return it if found
        const auto it = std::search( scan_start, scan_end, pattern.cbegin(), pattern.cend(),
            []( const uint8_t &a, const Build::PatternByte &b ) {
                return b.compare( a );
            }
        );

        return ( it != scan_end ) ? (uintptr_t)it : 0;
    }

} // namespace PatternScan