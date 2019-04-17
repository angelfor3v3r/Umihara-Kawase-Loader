#pragma once

#include "includes.h"

//
// ini parse options
//

// use unicode parser
// define " INIP_USE_UNICODE " to use unicode parser

//
// ini parse types
//

#ifdef INIP_USE_UNICODE
    // type(s)
    using inip_str_t         = std::wstring;
    using inip_str_view_t    = std::wstring_view;
    using inip_file_stream_t = std::wifstream;

    #define INIP_STR( x ) L ##x
#else
    // type(s)
    using inip_str_t         = std::string;
    using inip_str_view_t    = std::string_view;
    using inip_file_stream_t = std::ifstream;

    #define INIP_STR( x ) x
#endif

class INIParser {
private:
    // valid ini file?
    bool m_valid;

    // string / wstring npos value
    static constexpr auto INIP_STR_NPOS = inip_str_t::npos;

public:
    //
    // info about each section value
    //

    class ValueInfo {
    private:
        // allow INIParser to access
        friend class INIParser;

        // name of value
        inip_str_t m_name;

        // value name hash
        hash32_t m_name_hash;

        // value
        inip_str_t m_value;

        // ctors
        ValueInfo() = default;

        FORCEINLINE ValueInfo( inip_str_view_t name, hash32_t name_hash, inip_str_view_t value ) : m_name{ name }, m_name_hash{ name_hash }, m_value{ value } {

        }

        // try to convert string to an int / float type
        template< typename t > NOINLINE std::optional< t > convert_str( inip_str_view_t str ) const {
            t out;

#ifdef INIP_USE_UNICODE
            wchar_t *end = nullptr;
#else
            char *end = nullptr;
#endif

            //
            // check type
            //

            // double
            if constexpr( std::is_same_v< t, double > ) {
#ifdef INIP_USE_UNICODE
                out = (t)( std::wcstod( m_value.c_str(), &end ) );
#else
                out = (t)( std::strtod( m_value.c_str(), &end ) );
#endif
            }

            // float
            else if( std::is_same_v< t, float > ) {
#ifdef INIP_USE_UNICODE
                out = (t)( std::wcstof( m_value.c_str(), &end ) );
#else
                out = (t)( std::strtof( m_value.c_str(), &end ) );
#endif
            }

            // 32-bit or 64-bit int
            else if( std::is_integral_v< t > ) {
                // 64-bit or 32-bit int?
                constexpr auto is_64bit = ( sizeof( t ) ) == 8;

                // unsigned
                if( std::is_unsigned_v< t > ) {
                    // 64-bit
                    if( is_64bit ) {
#ifdef INIP_USE_UNICODE
                        out = (t)( std::wcstoull( m_value.c_str(), &end, 0 ) );
#else
                        out = (t)( std::strtoull( m_value.c_str(), &end, 0 ) );
#endif
                    }

                    // 32-bit or lower
                    else {
#ifdef INIP_USE_UNICODE
                        out = (t)( std::wcstoul( m_value.c_str(), &end, 0 ) );
#else
                        out = (t)( std::strtoul( m_value.c_str(), &end, 0 ) );
#endif
                    }
                }

                // signed
                else {
                    // 64-bit
                    if( is_64bit ) {
#ifdef INIP_USE_UNICODE
                        out = (t)( std::wcstoll( m_value.c_str(), &end, 0 ) );
#else
                        out = (t)( std::strtoll( m_value.c_str(), &end, 0 ) );
#endif
                    }

                    // 32-bit or lower
                    else {
#ifdef INIP_USE_UNICODE
                        out = (t)( std::wcstol( m_value.c_str(), &end, 0 ) );
#else
                        out = (t)( std::strtol( m_value.c_str(), &end, 0 ) );
#endif
                    }
                }
            }

            // check errors, no trailing numbers in strings allowed
            // only the first valid number is converted (and allowed)
            if( !end || errno != 0 || *end != '\0' )
                return {};

            return out;
        }

    public:
        // return name of value
        FORCEINLINE inip_str_t get_value_name() const {
            return m_name;
        }

        // return 32-bit hash of section name
        FORCEINLINE hash32_t get_value_name_hash() const {
            return m_name_hash;
        }

        // returns set value string
        FORCEINLINE inip_str_t get_str() const {
            return m_value;
        }

        // returns set value double
        FORCEINLINE auto get_double() const {
            return convert_str< double >( m_value );
        }

        // returns set value float
        FORCEINLINE auto get_float() const {
            return convert_str< float >( m_value );
        }

        // returns set value uint64_t
        FORCEINLINE auto get_uint64() const {
            return convert_str< uint64_t >( m_value );
        }

        // returns set value int64_t
        FORCEINLINE auto get_int64() const {
            return convert_str< int64_t >( m_value );
        }

        // returns set value uint32_t
        FORCEINLINE auto get_uint32() const {
            return convert_str< uint32_t >( m_value );
        }

        // returns set value int32_t
        FORCEINLINE auto get_int32() const {
            return convert_str< int32_t >( m_value );
        }

        // returns set value bool
        NOINLINE std::optional< bool > get_bool() const {
#ifdef INIP_USE_UNICODE
            const auto tolower_pred = []( int c ) {
                return std::tolower( c );
            };
#else
            const auto tolower_pred = []( uint8_t c ) {
                return std::tolower( c );
            };
#endif

            // make copy of value string
            auto value_str = m_value;

            // convert to lowercase
            std::transform( value_str.begin(), value_str.end(), value_str.begin(), tolower_pred );

            // check hash
            // return true / false
            // note: int strings besides these are not supported and treated as invalid
            //       (for example, an int or floating point value above 1)
            switch( FNV1aHash::get_32( value_str ) ) {
                case CT_HASH_32( INIP_STR( "1"    ) ):
                case CT_HASH_32( INIP_STR( "true" ) ):
                case CT_HASH_32( INIP_STR( "on"   ) ):
                case CT_HASH_32( INIP_STR( "yes"  ) ): {
                    return true;
                }

                case CT_HASH_32( INIP_STR( "0"      ) ):
                case CT_HASH_32( INIP_STR( "false" ) ):
                case CT_HASH_32( INIP_STR( "off"   ) ):
                case CT_HASH_32( INIP_STR( "no"    ) ): {
                    return false;
                }

                default: {
                    return {};
                }
            }

            return {};
        }
    };

    // section value container
    using section_vars_t = std::vector< ValueInfo >;

    //
    // info about each section
    //

    class SectionInfo {
    private:
        // allow INIParser to construct
        friend class INIParser;

        // section name
        inip_str_t m_name;

        // section name hash
        hash32_t m_name_hash;

        // section vars and values
        section_vars_t m_values;

        // ctors
        SectionInfo() = default;

        FORCEINLINE SectionInfo( inip_str_view_t name, hash32_t name_hash ) : m_name{ name }, m_name_hash{ name_hash }, m_values{} {

        }

    public:
        // name of section
        FORCEINLINE inip_str_t get_name() const {
            return m_name;
        }

        // return 32-bit hash of section name
        FORCEINLINE hash32_t get_name_hash() const {
            return m_name_hash;
        }

        // return vector of ValueInfo
        FORCEINLINE const section_vars_t &get_all_values() const {
            return m_values;
        }
    };

    // section info container
    using sections_t = std::vector< SectionInfo >;

private:
    // holds all sections and section values
    sections_t m_sections;

    // find value by hash
    NOINLINE std::optional< ValueInfo * > get_value_info_by_hash( hash32_t section_name, hash32_t value_name );

public:
    INIParser() = default;

    // open and parse INI file
    NOINLINE INIParser( inip_str_view_t filename );

    // open and parse INI file
    NOINLINE bool init( inip_str_view_t filename );

    // returns set value string
    NOINLINE inip_str_t get_value_str( inip_str_view_t section_name, inip_str_view_t value_name, inip_str_t default_value );

    // returns set value double
    NOINLINE double get_value_double( inip_str_view_t section_name, inip_str_view_t value_name, double default_value );

    // returns set value float
    NOINLINE float get_value_float( inip_str_view_t section_name, inip_str_view_t value_name, float default_value );

    // returns set value uint64_t
    NOINLINE uint64_t get_value_uint64( inip_str_view_t section_name, inip_str_view_t value_name, uint64_t default_value );

    // returns set value int64_t
    NOINLINE int64_t get_value_int64( inip_str_view_t section_name, inip_str_view_t value_name, int64_t default_value );

    // returns set value uint32_t
    NOINLINE uint32_t get_value_uint32( inip_str_view_t section_name, inip_str_view_t value_name, uint32_t default_value );

    // returns set value int32_t
    NOINLINE int32_t get_value_int32( inip_str_view_t section_name, inip_str_view_t value_name, int32_t default_value );

    // returns set value bool
    NOINLINE bool get_value_bool( inip_str_view_t section_name, inip_str_view_t value_name, bool default_value );

    // return vector of SectionInfo
    FORCEINLINE const sections_t &get_all_sections() const {
        return m_sections;
    }

    // ini parse error?
    FORCEINLINE bool is_valid() const {
        return m_valid;
    }
};

#undef INIP_STR