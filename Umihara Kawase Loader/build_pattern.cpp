#include "build_pattern.h"

namespace PatternScan {
    // code for building pattern data for scanning funcs
    namespace Build {
        Pattern::Pattern( std::string_view str ) : m_pattern{} {
            std::string token;

            if( str.empty() )
                return;

            // construct str stream
            auto sstream = std::istringstream( str.data() );

            // iterate each token
            // split strings by space
            while( std::getline( sstream, token, ' ' ) ) {
                // skip if byte / wildcard is too long
                const auto size = token.size();
                if( !size || size > 2 ) {
                    m_pattern.clear();
                
                    break;
                }

                // is it a wildcard? insert as wildcard byte
                // "??" is also valid here
                if( token[ 0 ] == '?' )
                    m_pattern.emplace_back( 0, true );

                // check for byte
                else {
                    // not a valid byte yet
                    auto is_valid_byte = false;

                    // 1 char
                    if( size == 1 && std::isxdigit( (uint8_t)token[ 0 ] ) )
                        is_valid_byte = true;

                    // 2 chars
                    else if( size == 2 && std::isxdigit( (uint8_t)token[ 0 ] ) && std::isxdigit( (uint8_t)token[ 1 ] ) )
                        is_valid_byte = true;

                    // not a valid byte
                    if( !is_valid_byte ) {
                        m_pattern.clear();
                    
                        break;
                    }

                    // now put into the vector
                    m_pattern.emplace_back(
                        (uint8_t)( std::strtoul( token.c_str(), nullptr, 16 ) ),
                        false
                    );
                }
            }
        }
    } // namespace Build
} // namespace PatternScan