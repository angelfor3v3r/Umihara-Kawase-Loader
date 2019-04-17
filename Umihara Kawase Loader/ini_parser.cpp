#include "ini_parser.h"

#ifdef INIP_USE_UNICODE
    #define INIP_STR( x ) L ##x
#else
    #define INIP_STR( x ) x
#endif

NOINLINE INIParser::INIParser( inip_str_view_t filename ) : m_valid{ false }, m_sections{} {
    init( filename );
}

NOINLINE bool INIParser::init( inip_str_view_t filename ) {
    inip_str_t  cur_line;
    SectionInfo *section_info = nullptr;

#ifdef INIP_USE_UNICODE
    const auto isspace_pred = []( int c ) {
        return std::isspace( c );
    };
#else
    const auto isspace_pred = []( uint8_t c ) {
        return std::isspace( c );
    };
#endif

    // bad filename
    if( filename.empty() )
        return false;

    // attempt to open file
    auto file = inip_file_stream_t( filename, std::ios::binary );
    if( !file )
        return false;

    // read by new line
    while( std::getline( file, cur_line ) ) {
        // empty line
        if( cur_line.empty() )
            continue;

        // strip all whitespace
        cur_line.erase( std::remove_if( cur_line.begin(), cur_line.end(), isspace_pred ), cur_line.end() );

        // ignore comments
        // go to next line
        // note: no support for trailing comments
        if( cur_line[ 0 ] == INIP_STR( ';' ) || cur_line[ 0 ] == INIP_STR( '#' ) )
            continue;

        // new section open
        if( cur_line[ 0 ] == INIP_STR( '[' ) ) {
            // try to find end
            const auto end_bracket_delim = cur_line.find_first_of( INIP_STR( ']' ) );
            if( end_bracket_delim == INIP_STR_NPOS )
                return false;

            // get section name
            // start after first bracket
            const auto section_name = cur_line.substr( 1, end_bracket_delim - 1 );
            if( section_name.empty() )
                return false;

            // make hash of section name
            const auto section_name_hash = FNV1aHash::get_32( section_name );

            // check if section exists already
            const auto sec_exists_it = std::find_if( m_sections.begin(), m_sections.end(),
                [ & ]( const SectionInfo &s ) -> bool {
                    return s.get_name_hash() == section_name_hash;
                }
            );

            // ... and use that section instead if possible
            if( sec_exists_it != m_sections.end() ) {
                section_info = &( *sec_exists_it );

                continue;
            }

            // ... otherwise make a new section entry
            m_sections.push_back( SectionInfo( section_name, section_name_hash ) );

            // try to get last section entry
            section_info = &m_sections.back();
            if( !section_info )
                return false;
        }

        // assume it's a value
        else {
            // no section to work off of...
            if( !section_info )
                return false;

            // try to find equal delim
            const auto equal_delim = cur_line.find_first_of( INIP_STR( '=' ) );
            if( equal_delim == INIP_STR_NPOS )
                return false;

            // get name
            // empty value names are not allowed
            const auto value_name = cur_line.substr( 0, equal_delim );
            if( value_name.empty() )
                return false;

            // make hash of value name
            const auto value_name_hash = FNV1aHash::get_32( value_name );

            // todo: check same value name?

            // get value
            // null values are allowed...
            const auto value = cur_line.substr( equal_delim + 1 );

            // add to section
            section_info->m_values.push_back( ValueInfo( value_name, value_name_hash, value ) );
        }
    }

    // if we got here, parsing succeeded
    m_valid = true;

    return true;
}

NOINLINE std::optional< INIParser::ValueInfo * > INIParser::get_value_info_by_hash( hash32_t section_name, hash32_t value_name ) {
    // first, find section info by hash
    const auto section_it = std::find_if( m_sections.begin(), m_sections.end(),
        [ & ]( const SectionInfo &s ) {
            return s.get_name_hash() == section_name;
        }
    );

    if( section_it == m_sections.end() )
        return {};

    // now, find the value info by hash
    const auto value_it = std::find_if( section_it->m_values.begin(), section_it->m_values.end(),
        [ & ]( const ValueInfo &s ) {
            return s.get_value_name_hash() == value_name;
        }
    );

    if( value_it == section_it->m_values.end() )
        return {};

    return &( *value_it );
}

NOINLINE inip_str_t INIParser::get_value_str( inip_str_view_t section_name, inip_str_view_t value_name, inip_str_t default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    return ( value_info ) ? ( *value_info )->get_str() : default_value;
}

NOINLINE double INIParser::get_value_double( inip_str_view_t section_name, inip_str_view_t value_name, double default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_double();

    return ( out ) ? *out : default_value;
}

NOINLINE float INIParser::get_value_float( inip_str_view_t section_name, inip_str_view_t value_name, float default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_float();

    return ( out ) ? *out : default_value;
}

NOINLINE uint64_t INIParser::get_value_uint64( inip_str_view_t section_name, inip_str_view_t value_name, uint64_t default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_uint64();

    return ( out ) ? *out : default_value;
}

NOINLINE int64_t INIParser::get_value_int64( inip_str_view_t section_name, inip_str_view_t value_name, int64_t default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_int64();

    return ( out ) ? *out : default_value;
}

NOINLINE uint32_t INIParser::get_value_uint32( inip_str_view_t section_name, inip_str_view_t value_name, uint32_t default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_uint32();

    return ( out ) ? *out : default_value;
}

NOINLINE int32_t INIParser::get_value_int32( inip_str_view_t section_name, inip_str_view_t value_name, int32_t default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_int32();

    return ( out ) ? *out : default_value;
}

NOINLINE bool INIParser::get_value_bool( inip_str_view_t section_name, inip_str_view_t value_name, bool default_value ) {
    const auto value_info = get_value_info_by_hash(
        FNV1aHash::get_32( section_name ),
        FNV1aHash::get_32( value_name )
    );

    if( !value_info )
        return default_value;

    const auto out = ( *value_info )->get_bool();

    return ( out ) ? *out : default_value;
}

#undef INIP_STR