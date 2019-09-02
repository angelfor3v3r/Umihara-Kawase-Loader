#include "includes.h"

/*
    Umihara Kawase Loader by melanite ( https://github.com/melanite/Umihara-Kawase-Loader )

    //
    // MIT License
    //
    // Copyright (c) 2019 Melanite
    //

    Credits and thanks to:
                           frost
                           n0x
                           mpu
                           dan
                           keegan
*/

//
// misc defs / vars
//

// info about each user keybind
class UserKeyData {
public:
    uint32_t m_dinput_key;    // user keybind (dinput)
    uint32_t m_umi_key_state; // key used by game
};

// user keybinds
// default m_dinput_key value is changed in INI
static std::vector< UserKeyData > g_user_keybinds = {
    // movement and move UI selection keys
    { DIK_UP,     UMI_KEY_UP    },
    { DIK_DOWN,   UMI_KEY_DOWN  },
    { DIK_LEFT,   UMI_KEY_LEFT  },
    { DIK_RIGHT,  UMI_KEY_RIGHT },

    // menu related
    { DIK_SPACE,  UMI_KEY_START   },
    { DIK_RETURN, UMI_KEY_PAUSE   },
    { DIK_TAB,    UMI_KEY_SELECT  }, // NOTE: this key is only valid on the first game (UmiharaKawase)
    { DIK_S,      UMI_KEY_RESTART },
    { DIK_ESCAPE, UMI_KEY_BACK    },

    // gameplay
    { DIK_Z,     UMI_KEY_JUMP },
    { DIK_A,     UMI_KEY_HOOK },
    { DIK_PRIOR, UMI_KEY_L    },
    { DIK_NEXT,  UMI_KEY_R    },

    // misc
    { DIK_X, UMI_KEY_SKIP }
};

//
// global vars
//

// spdlog logging
std::shared_ptr< spdlog::logger > g_log;

// filepaths, etc
static std_fs::path g_path_root_dir;
static std_fs::path g_path_loader_dir;
static std_fs::path g_path_loader_dll_dir;
static std_fs::path g_path_loader_ini;
static std_fs::path g_path_loader_log;

// game related funcs / vars
static uintptr_t    g_input_hander_func_addr = 0;
static std::wstring g_game_name              = L"";
static int8_t       g_game_id                = UMI_GAME_INVALID;
static uint32_t     *g_key_list              = nullptr;

// hooks
static Detour< input_handler_t > g_input_handler_hook;

//
// ini settings
//

static auto g_ini_use_keybinds = false;

//
// misc funcs
//

static NOINLINE void init_failed( std::wstring_view error ) {
    const auto title_msg = fmt::format( L"[Umihara Kawase Loader] Error: {}", error );

    MessageBoxW(
        nullptr,
        L"Initialization failed.\n\n"
        L"Ensure loader DLL and directories are set up properly.\n\n"
        L"If you believe this is an error then please report this to the developer at:\n"
        L"https://github.com/melanite/Umihara-Kawase-Loader",
        title_msg.c_str(),
        MB_ICONERROR
    );

    ExitProcess( 0 );
}

static NOINLINE void init_paths() {
    // check exe path dir
    g_path_root_dir = std_fs::current_path();
    if( !std_fs::exists( g_path_root_dir ) ) {
        init_failed( L"Invalid game root directory" );

        return;
    }

    // check output dir
    g_path_loader_dir = g_path_root_dir / L"umi_loader";
    if( !std_fs::exists( g_path_loader_dir ) ) {
        init_failed( L"Invalid loader root directory" );

        return;
    }

    // check dll dir
    g_path_loader_dll_dir = g_path_loader_dir / L"dlls";
    if( !std_fs::exists( g_path_loader_dir ) ) {
        init_failed( L"Invalid loader dlls root directory" );

        return;
    }

    // check INI file
    g_path_loader_ini = g_path_loader_dir / L"config.ini";
    if( !std_fs::exists( g_path_loader_ini ) ) {
        init_failed( L"config.ini not found" );

        return;
    }

    // get output log path
    g_path_loader_log = g_path_loader_dir / L"log.txt";
}

static NOINLINE bool init_ini() {
    INIParser ini;

    // get keybind from file, keep track of index
    static auto get_ini_keybind = [ & ]( std::wstring_view section_name, std::wstring_view value_name, uint32_t default_value ) {
        static size_t cur_idx = 0;

        // get entry @ index, go to next
        const auto entry = &g_user_keybinds[ cur_idx++ ];

        // set dinput key in vector
        entry->m_dinput_key = ini.get_value_uint32( section_name, value_name, default_value );
    };

    ini = INIParser( g_path_loader_ini.wstring() );
    if( !ini.is_valid() ) {
        init_failed( L"INI parse error" );

        return false;
    }

    // get misc INI stuff
    g_ini_use_keybinds = ini.get_value_bool( L"settings", L"rebind_keys", false );

    // get keybinds from INI
    if( g_ini_use_keybinds ) {
        // this sucks, but it must be done in order...

        // movement and move UI selection keybinds
        get_ini_keybind( L"settings", L"KEY_UP",    DIK_UP    );
        get_ini_keybind( L"settings", L"KEY_DOWN",  DIK_DOWN  );
        get_ini_keybind( L"settings", L"KEY_LEFT",  DIK_LEFT  );
        get_ini_keybind( L"settings", L"KEY_RIGHT", DIK_RIGHT );

        // menu related keybinds
        get_ini_keybind( L"settings", L"KEY_START",   DIK_SPACE  );
        get_ini_keybind( L"settings", L"KEY_PAUSE",   DIK_RETURN );
        get_ini_keybind( L"settings", L"KEY_SELECT",  DIK_TAB    );
        get_ini_keybind( L"settings", L"KEY_RESTART", DIK_S      );
        get_ini_keybind( L"settings", L"KEY_BACK",    DIK_ESCAPE );

        // gameplay keybinds
        get_ini_keybind( L"settings", L"KEY_JUMP", DIK_Z     );
        get_ini_keybind( L"settings", L"KEY_HOOK", DIK_A     );
        get_ini_keybind( L"settings", L"KEY_L",    DIK_PRIOR );
        get_ini_keybind( L"settings", L"KEY_R",    DIK_NEXT  );

        // misc keybinds
        get_ini_keybind( L"settings", L"KEY_SKIP", DIK_X );

        g_log->info( L"Extracted keybinds from INI" );
    }

    g_log->info( L"INI parse done: \"{}\"", g_path_loader_ini.wstring() );

    return true;
}

static NOINLINE bool check_valid_dll( std::wstring_view filename ) {
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;

    // open up file at end
    auto file = std::ifstream( filename, ( std::ios::ate | std::ios::binary ) );
    if( !file )
        return false;

    // get filesize
    const auto file_size = file.tellg();
    if( file_size == -1 )
        return false;

    // go back to start, since we opened at the end
    file.seekg( 0 );

    // allocate buffer for file
    auto file_buffer = std::vector< char >( (size_t)file_size );

    // read file contents to buffer
    if( !file.read( file_buffer.data(), file_size ) )
        return false;

    // get file headers
    // make sure this is a DLL
    if( !Utils::get_pe_file_headers( (uintptr_t)( file_buffer.data() ), dos, nt ) )
        return false;

    if( !( nt->FileHeader.Characteristics & IMAGE_FILE_DLL ) )
        return false;

    return true;
}

static NOINLINE bool load_dlls() {
    uint32_t loaded_amt = 0;

    g_log->info( L"Trying to load extra DLLs (if any)" );

    // iterate files
    for( const auto &f : std::filesystem::recursive_directory_iterator( g_path_loader_dll_dir ) ) {
        // skip bad files
        if( !f.exists() ) {
            g_log->warn( L"Invalid DLL filepath (1)" );

            continue;
        }

        // "recursive_directory_iterator" will always go into directories but we must skip them
        // since they will be treated as "files"
        if( f.is_directory() )
            continue;

        // convert to path object
        const auto cur_file = f.path();
        if( cur_file.empty() ) {
            g_log->warn( L"Invalid DLL filepath (2)" );

            continue;
        }

        // get path string
        const auto filename = cur_file.wstring();

        // skip bad extensions
        if( cur_file.extension() != L".dll" ) {
            g_log->warn( L"Invalid DLL extension: \"{}\"", filename );

            continue;
        }

        // skip bad PE files
        if( !check_valid_dll( filename ) ) {
            g_log->warn( L"Invalid DLL file: \"{}\"", filename );

            continue;
        }

        // try to load it
        if( !LoadLibraryW( filename.c_str() ) ) {
            g_log->warn( L"Failed to map DLL: \"{}\"", filename );

            continue;
        }

        g_log->info( L"Loaded DLL: \"{}\"", filename );

        // keep track of amount
        ++loaded_amt;
    }

    g_log->info( L"Extra DLL loading done, loaded {} {}", loaded_amt, ( loaded_amt == 1 ) ? L"DLL" : L"DLLs" );

    return true;
}

static NOINLINE std::optional< uint32_t > umi_key_to_dinput_key( uint32_t key ) {
    // array in .rdata size, etc
    constexpr auto KEY_LIST_SIZE_BYTES = uint32_t{ 0x90 };
    constexpr auto KEY_LIST_SIZE       = KEY_LIST_SIZE_BYTES / sizeof( uint32_t );

    // iterate list
    // skip an index, game keys are seperated like this...
    // don't scan real dinput keys, only game keys
    for( uint32_t i = 0; i < KEY_LIST_SIZE; i += 2 ) {
        // real key code is back an index
        if( g_key_list[ i ] == key )
            return g_key_list[ i - 1 ];
    }

    return {};
}

static NOINLINE void modify_input_data( InputData *data ) {
    uint8_t m_key_states[ 256 ];

    // check for dinput keypress flag
    static auto is_key_pressed = [ & ]( uint8_t sc ) -> bool {
        return ( m_key_states[ sc ] & 0x80 );
    };

    // make sure data is valid first
    if( !data || !data->m_dinput_device )
        return;

    // only do this if we're in focus
    if( data->m_focus_flag != UMI_FOCUSED )
        return;

    // capture keyboard state
    const auto dihr = data->m_dinput_device->GetDeviceState( sizeof( m_key_states ), &m_key_states );
    if( dihr != DI_OK )
        return;

    // swallow all keys
    data->m_key_state = 0;

    // check for key(s) being pressed
    for( const auto &b : g_user_keybinds ) {
        // // this key is only valid on the first game
        // // skip on other games
        // if( b.m_umi_key_state == UMI_KEY_SELECT && g_game_ver_id != UMI_GAME_KAWASE )
        //     continue;

        // key is pressed, add to keybind flag
        if( is_key_pressed( b.m_dinput_key ) )
            data->m_key_state |= b.m_umi_key_state;
    }
}

//
// hooked funcs
//

static NOINLINE int __fastcall input_handler_hook( InputData *data, uintptr_t edx ) {
    // store original function
    static const auto orig = g_input_handler_hook.get_orig_func();

    // let original run first
    // the game will fill out the input data structure
    const auto ret = orig( data );

    // send inputs as needed
    modify_input_data( data );

    return ret;
}

//
// init funcs
//

static NOINLINE ulong_t __stdcall init_thread( void *arg ) {
    // 100ms
    constexpr ulong_t INIT_WAIT_TIME = 100;

    // 10 seconds
    constexpr ulong_t MAX_INIT_WAIT_TIME = INIT_WAIT_TIME * 100;

    ulong_t   total_wait_time = 0;
    uintptr_t found_name_str  = 0;

    // this is pretty silly but my guess is the steam DRM unpacking routine takes a bit to finish (???)
    // find reference to game path wstring
    do {
        found_name_str = PatternScan::find( "", "0F B7 8A ? ? ? ? 66 85 C9 75 EA 33 C9 66 89 0C 46 EB 77" );

        // keep track of total sleep time
        total_wait_time += INIT_WAIT_TIME;
        if( total_wait_time > MAX_INIT_WAIT_TIME ) {
            init_failed( L"Invalid game or outdated signatures" );

            return 0;
        }

        // give CPU some time
        Sleep( INIT_WAIT_TIME );
    }
    while( !found_name_str );

    // get name location in .rdata
    found_name_str = *(uintptr_t *)( found_name_str + 3 );

    // initialize string from location in .rdata
    g_game_name = std::wstring( (wchar_t *)found_name_str );
    if( g_game_name.empty() )
        return 0;

    // remove backslashes from the game name string, we don't need them
    // the games use them for filepaths
    g_game_name.erase( std::remove( g_game_name.begin(), g_game_name.end(), L'\\' ), g_game_name.end() );

    // check game name hash
    // set game version ID
    switch( FNV1aHash::get_32( g_game_name ) ) {
        case CT_HASH_32( L"UmiharaKawase" ): {
            g_game_id = UMI_GAME_KAWASE;

            break;
        }

        case CT_HASH_32( L"UmiharaKawase Shun SE" ): {
            g_game_id = UMI_GAME_KAWASE_SHUN;

            break;
        }

        case CT_HASH_32( L"Sayonara Umihara Kawase" ): {
            g_game_id = UMI_GAME_SAYONARA_KAWASE;

            break;
        }

        default: {
            break;
        }
    }

    //
    // initialize
    //

    // set up paths
    init_paths();

    // set up log sinks
    const auto file_sink    = std::make_shared< spdlog::sinks::basic_file_sink_mt >( g_path_loader_log.u8string(), true );
    const auto console_sink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();

    // create multi log sink
    g_log = std::make_shared< spdlog::logger >( "multi_sink", spdlog::sinks_init_list{ file_sink, console_sink } );

    // set spdlog behaviour
    g_log->flush_on( spdlog::level::info );
    g_log->set_pattern( "[Umihara Kawase Loader] [%D %T.%e] [%^%l%$] - %v" );

    // check game version
    if( g_game_id == UMI_GAME_INVALID ) {
        g_log->error( L"Failed to identify game version (1)" );

        return 0;
    }

    // print game version info
    g_log->info( L"Game: \"{}\" (ID: {})", g_game_name, g_game_id );

    // set up ini
    if( !init_ini() ) {
        g_log->error( L"Failed to set up ini" );

        return 0;
    }

    // load other DLLs
    if( !load_dlls() ) {
        g_log->error( L"Failed to load extra DLLs" );

        return 0;
    }

    //
    // sigscan, etc
    //

    uintptr_t key_list_tmp;

    // find sigs on each game
    switch( g_game_id ) {
        case UMI_GAME_KAWASE: {
            // follow relative jmp
            g_input_hander_func_addr = Utils::follow_rel_instruction( PatternScan::find( "", "E8 ? ? ? ? B8 ? ? ? ? 8B FF" ) );

            key_list_tmp = PatternScan::find( "", "B8 ? ? ? ? 8D 9B ? ? ? ?" );

            break;
        }

        case UMI_GAME_KAWASE_SHUN: {
            // follow relative jmp
            g_input_hander_func_addr = Utils::follow_rel_instruction( PatternScan::find( "", "E8 ? ? ? ? FF 35 ? ? ? ? 8B 35 ? ? ? ?" ) );

            key_list_tmp = PatternScan::find( "", "B8 ? ? ? ? EB 08" );

            break;
        }

        case UMI_GAME_SAYONARA_KAWASE: {
            g_input_hander_func_addr = Utils::follow_rel_instruction( PatternScan::find( "", "E8 ? ? ? ? FF 35 ? ? ? ? 8B 35 ? ? ? ?" ) );

            key_list_tmp = PatternScan::find( "", "89 86 ? ? ? ? B8 ? ? ? ? EB 08" );
            if( !key_list_tmp )
                break;

            // skip over (mov [reg+disp32], reg)
            // (modrm = 2, 0, 6)
            key_list_tmp += 7;

            break;
        }

        default: {
            g_log->error( L"Failed to identify game version (2)" );

            return 0;
        }
    }

    // valid sigs?
    if( !g_input_hander_func_addr ) {
        g_log->error( L"Failed to find input handler func" );

        return 0;
    }

    if( !key_list_tmp ) {
        g_log->error( L"Failed to find key list array" );

        return 0;
    }

    // get key list now
    // actual key list starts 4 bytes back
    g_key_list  = *(uint32_t **)key_list_tmp;
    g_key_list -= 1;

    // log...
    g_log->info( L"Input handler func: 0x{:X}", g_input_hander_func_addr );
    g_log->info( L"Key list array: 0x{:X}", (uintptr_t)g_key_list );

    //
    // set up hooks
    //

    // check if user wants to rebind keys
    if( g_ini_use_keybinds ) {
        // hook input handler
        if( !g_input_handler_hook.init( g_input_hander_func_addr, &input_handler_hook ) ) {
            g_log->error( L"Failed to initialize input handler hook" );

            return 0;
        }

        // ... and enable it
        if( !g_input_handler_hook.enable() ) {
            g_log->error( L"Failed to enable input handler hook" );

            return 0;
        }
    }

    g_log->info( L"Initialized!" );

    return 1;
}

int __stdcall DllMain( HINSTANCE instance, ulong_t reason_for_call, void *reserved ) {
    if( reason_for_call == DLL_PROCESS_ATTACH ) {
        // set up dinput8.dll wrapper
        // this must be done first
        // our init thread routine must wait
        if( !Dinput8Wrapper::init() ) {
            init_failed( L"Failed to load original dinput8.dll" );

            return 0;
        }

        // ... now create our init thread
        const auto thread = SHandle( CreateThread( nullptr, 0, init_thread, nullptr, 0, nullptr ) );
        if( !thread )
            return 0;

        return 1;
    }

    return 0;
}