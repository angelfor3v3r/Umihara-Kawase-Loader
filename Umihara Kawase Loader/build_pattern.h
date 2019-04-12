#pragma once

#include "includes.h"

namespace PatternScan {
    // code for building pattern data for scanning funcs
    namespace Build {
        //
        // wraps a uint8_t and tells us if we should skip it due to a wildcard
        //

        class PatternByte {
        private:
            uint8_t m_byte;
            bool    m_is_wildcard;
        
        public:
            FORCEINLINE PatternByte() = default;
        
            FORCEINLINE PatternByte( uint8_t byte, bool is_wildcard ) : m_byte{ byte }, m_is_wildcard{ is_wildcard } {
        
            }
        
            // get byte / wildcard
            FORCEINLINE uint8_t get_byte() const {
                return m_byte;
            }
        
            FORCEINLINE bool is_wildcard() const {
                return m_is_wildcard;
            }
        
            // match a byte to stored byte
            FORCEINLINE bool compare( uint8_t other ) const {
                return m_byte == other || m_is_wildcard;
            }
        };

        //
        // converts a string to a pattern vector (only supports IDA-style patterns)
        //

        class Pattern {
        private:
            // types
            using container_t       = std::vector< PatternByte >;
            using container_iter_t  = container_t::iterator;
            using container_citer_t = container_t::const_iterator;

            // vector for our pattern bytes
            container_t m_pattern;

        public:
            Pattern() = default;
            NOINLINE Pattern( std::string_view str );

            // returns pattern vector
            FORCEINLINE container_t get_vector() const {
                return m_pattern;
            }

            // returns begin / end of vector
            // for ranged-based loops, etc
            FORCEINLINE container_citer_t cbegin() const {
                return m_pattern.cbegin();
            }

            FORCEINLINE container_citer_t cend() const {
                return m_pattern.cend();
            }

            // returns a PatternByte object from the vector
            FORCEINLINE const PatternByte &operator []( size_t idx ) const {
                return m_pattern[ idx ];
            }

            // is the pattern empty?
            FORCEINLINE bool empty() const {
                return m_pattern.empty();
            }

            // valid checks
            FORCEINLINE operator bool() const {
                return empty() != true;
            }

            FORCEINLINE bool operator !() const {
                return empty() == true;
            }
        };

    } // namespace Build
} // namespace PatternScan