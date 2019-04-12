#include "includes.h"

/*
    Umihara Kawase Loader by melanite ( https://github.com/melanite/Umihara-Kawase-Loader )
   
    //
        MIT License

        Copyright (c) 2019 Melanite
    //

    Credits and thanks to:
                           frost
                           n0x
                           mpu
                           dan
*/

//
// fwd declare funcs
//

// hooked funcs
static NOINLINE int __fastcall input_handler( InputData *input_data, uintptr_t edx );

//
// global vars
//

// spdlog logging
std::shared_ptr< spdlog::logger > g_log;

// filepaths, etc
static std_fs::path g_path_root_dir;
static std_fs::path g_path_loader_dir;
static std_fs::path g_path_loader_dll_dir;
static std_fs::path g_path_loader_log;
static std_fs::path g_path_loader_ini;

// game related funcs / vars
static uintptr_t g_input_hander_func_addr = 0;
static uint32_t  *g_key_list              = nullptr;

// hooks
static Detour< input_handler_t > g_input_handler_hook;

//
// ini settings
//

static auto g_ini_use_keybinds = false;

//
// misc defs / vars
//

class UserKeyData {
public:
    uint32_t m_dinput_key;
    uint32_t m_umi_key_state;
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
    { DIK_TAB,    UMI_KEY_SELECT  },
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
// misc funcs
//

static NOINLINE bool init_paths() {
    // check exe path dir
    g_path_root_dir = std_fs::current_path();
    if( !std_fs::exists( g_path_root_dir ) ) {
        // g_log->error( L"bad root directory" );
    
        return false;
    }
    
    // check output dir
    g_path_loader_dir = g_path_root_dir / L"umi_loader";
    if( !std_fs::exists( g_path_loader_dir ) ) {
        // g_log->error( L"\"umi_loader\" directory doesn't exist -- you need this from the release ZIP and it must be in the game install directory" );

        return false;
    }

    // check dll dir
    g_path_loader_dll_dir = g_path_loader_dir / L"dlls";
    if( !std_fs::exists( g_path_loader_dir ) ) {
        // g_log->error( L"\"umi_loader/dlls"\ directory doesn't exist -- you need this from the release ZIP and it must be in the game install directory" );

        return false;
    }

    // get output log path
    g_path_loader_log = g_path_loader_dir / L"log.txt";

    return true;
}

static NOINLINE bool init_ini() {
    // check ini file
    g_path_loader_ini = g_path_loader_dir / L"config.ini";
    if( !std_fs::exists( g_path_loader_ini ) ) {
        g_log->error( L"\"umi_loader/config.ini\" file doesn't exist -- you need this from the release ZIP and it must be in the umi_loader directory; loader will NOT run" );

        return false;
    }

    INIReader ini( g_path_loader_ini.u8string().c_str() );
    if( ini.ParseError() != 0 ) {
        g_log->error( L"Failed to parse INI" );
    
        return false;
    }
    
    // get misc ini stuff
    g_ini_use_keybinds = ini.GetBoolean( "settings", "rebind_keys", false );

    // get keybinds from ini
    // this sucks, but it must be done in order...
    if( g_ini_use_keybinds ) {
        // movement and move UI selection keys
        g_user_keybinds[ 0 ].m_dinput_key = ini.GetInteger( "settings", "KEY_UP",    DIK_UP    );
        g_user_keybinds[ 1 ].m_dinput_key = ini.GetInteger( "settings", "KEY_DOWN",  DIK_DOWN  );
        g_user_keybinds[ 2 ].m_dinput_key = ini.GetInteger( "settings", "KEY_LEFT",  DIK_LEFT  );
        g_user_keybinds[ 3 ].m_dinput_key = ini.GetInteger( "settings", "KEY_RIGHT", DIK_RIGHT );
        
        // menu related
        g_user_keybinds[ 4 ].m_dinput_key = ini.GetInteger( "settings", "KEY_START",   DIK_SPACE  );
        g_user_keybinds[ 5 ].m_dinput_key = ini.GetInteger( "settings", "KEY_PAUSE",   DIK_RETURN );
        g_user_keybinds[ 6 ].m_dinput_key = ini.GetInteger( "settings", "KEY_SELECT",  DIK_TAB    );
        g_user_keybinds[ 7 ].m_dinput_key = ini.GetInteger( "settings", "KEY_RESTART", DIK_S      );
        g_user_keybinds[ 8 ].m_dinput_key = ini.GetInteger( "settings", "KEY_BACK",    DIK_ESCAPE );
        
        // gameplay
        g_user_keybinds[ 9 ].m_dinput_key  = ini.GetInteger( "settings", "KEY_JUMP", DIK_Z     );
        g_user_keybinds[ 10 ].m_dinput_key = ini.GetInteger( "settings", "KEY_HOOK", DIK_A     );
        g_user_keybinds[ 11 ].m_dinput_key = ini.GetInteger( "settings", "KEY_L",    DIK_PRIOR );
        g_user_keybinds[ 12 ].m_dinput_key = ini.GetInteger( "settings", "KEY_R",    DIK_NEXT  );
        
        // misc
        g_user_keybinds[ 13 ].m_dinput_key = ini.GetInteger( "settings", "KEY_SKIP", DIK_X );

        g_log->info( L"Extracted keybinds from INI" );
    }

    g_log->info( L"INI parse done: \"{}\"", g_path_loader_ini.wstring() );

    return true;
}

static NOINLINE bool check_valid_dll( std::wstring_view filename ) {
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS *nt;

    // open up file
    // map file to memory
    // get base address
    const auto file = CreateFileW( filename.data(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
    if( file == INVALID_HANDLE_VALUE )
        return false;

    const auto file_mapping = CreateFileMappingW( file, nullptr, PAGE_READONLY, 0, 0, 0 );
    if( !file_mapping ) {
        CloseHandle( file );
    
        return false;
    }

    const auto file_base = MapViewOfFile( file_mapping, FILE_MAP_READ, 0, 0, 0 );
    if( !file_base ) {
        CloseHandle( file );
        CloseHandle( file_mapping );
    
        return false;
    }

    // get file headers, return if this isn't a valid image file
    if( !Utils::get_pe_file_headers( (uintptr_t)file_base, dos, nt ) )
        return false;

    // not a dll...
    if( !( nt->FileHeader.Characteristics & IMAGE_FILE_DLL ) )
        return false;

    return true;
}

static NOINLINE bool load_dlls() {
    g_log->info( L"Trying to load extra DLLs (if any)" );

    auto loaded_amt = uint32_t{ 0 };

    // iterate files
    for( const auto &f : std::filesystem::directory_iterator( g_path_loader_dll_dir ) ) {
        // make sure this file actually exists
        if( !f.exists() )
            continue;

        // convert to path object
        const auto cur_file = f.path();
        if( cur_file.empty() ) {
            // g_log->error( L"Bad DLL filepath" );

            continue;
        }

        // get path string
        const auto filename = cur_file.wstring();

        // skip bad extensions
        if( cur_file.extension() != L".dll" ) {
            g_log->error( L"Bad DLL extension: \"{}\"", filename );

            continue;
        }

        // skip bad PE file
        if( !check_valid_dll( filename ) ) {
            g_log->error( L"Bad DLL file header: \"{}\"", filename );
        
            continue;
        }

        // try to load it
        if( !LoadLibraryW( filename.c_str() ) ) {
            g_log->error( L"Failed to map DLL: \"{}\"", filename );

            continue;
        }

        // keep track of amount
        ++loaded_amt;

        g_log->info( L"Loaded DLL: \"{}\"", filename );
    }

    g_log->info( L"Extra DLL loading done, loaded {} {}", loaded_amt, ( loaded_amt > 1 ) ? L"DLLs" : L"DLL" );

    return true;
}

static NOINLINE std::optional< uint32_t > umi_key_to_dinput_key( uint32_t key ) {
    // array in rdata size, etc
    constexpr auto KEY_LIST_SIZE_BYTES = uint32_t{ 0x90 };
    constexpr auto KEY_LIST_SIZE       = KEY_LIST_SIZE_BYTES / sizeof( uint32_t );

    // iterate list
    // skip an index, game keys are seperated like this...
    // don't scan real dinput keys, only game keys
    for( uint32_t i = 0; i < KEY_LIST_SIZE; i += 2 ) {
        // find key in list
        // real key code is back an index
        if( g_key_list[ i ] == key )
            return g_key_list[ i - 1 ];
    }

    return {};
}

static NOINLINE void modify_input_data( InputData *input_data ) {
    uint8_t m_key_states[ 256 ];

    // check for dinput keypress flag
    static auto is_key_pressed = [ & ]( uint8_t sc ) {
        return ( m_key_states[ sc ] & 0x80 );
    };

    // make sure data is valid first
    if( !input_data || !input_data->m_dinput_device )
        return;

    // only do this if we're in focus
    if( input_data->m_focus_flag != UMI_FOCUSED )
        return;

    // we could read the device state keys off stack (EBP - 0x104)
    // but let's just grab a new device state
    const auto dihr = input_data->m_dinput_device->GetDeviceState( sizeof( m_key_states ), &m_key_states );
    if( dihr != DI_OK )
        return;

    // check if user wants to rebind keys
    if( g_ini_use_keybinds ) {
        // swallow all keys
        input_data->m_key_state = 0;

        // init key flag
        auto key_flag = uint32_t{ 0 };

        // check for key(s) being pressed
        for( const auto &b : g_user_keybinds ) {
            // key is pressed, add to keybind flag
            if( is_key_pressed( b.m_dinput_key ) )
                key_flag |= b.m_umi_key_state;
        }

        // set final key
        input_data->m_key_state = key_flag;
    }
}

//
// hooked funcs
//

static NOINLINE int __fastcall input_handler( InputData *input_data, uintptr_t edx ) {
    // the ret address we care about...
    // check return address
    // we only care about keyboard input handler
    // seems like this is called in 3 different places with a different structure ptr
    // static const auto ret_1 = g_exe_base + 0x3FE9;

    // store original function
    static const auto orig = g_input_handler_hook.get_orig_func();

    // let original run first
    // the game will fill out the input data structure
    const auto ret = orig( input_data );

    if( true /* (uintptr_t)( _ReturnAddress() ) == ret_1 */ )
        modify_input_data( input_data );

    return ret;
}

//
// init funcs
//

static NOINLINE ulong_t __stdcall init_thread( void *arg ) {
    // this is pretty silly but my guess is the steam DRM unpacking routine takes a bit to finish (???)
    // wait until the input handler is found...
    do {
        // find jmp to input handler func
        g_input_hander_func_addr = PatternScan::find( "", "E8 ? ? ? ? B8 ? ? ? ? 8B FF" );
        
        // give CPU some time
        Sleep( 100 );
    }
    while( g_input_hander_func_addr == 0 );

    //
    // initialize
    //

    // set up paths
    if( !init_paths() )
        return 0;
    
    // // spawn a debug console
    // if( !AllocConsole() )
    //     return 0;
    // 
    // SetConsoleTitleA( "Umihara Kawase Loader" );
    
    // set up log sinks
    const auto file_sink    = std::make_shared< spdlog::sinks::basic_file_sink_mt >( g_path_loader_log.u8string(), true );
    const auto console_sink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();
    
    // create multi log sink
    g_log = std::make_shared< spdlog::logger >( "multi_sink", spdlog::sinks_init_list{ file_sink, console_sink } );

    g_log->flush_on( spdlog::level::info );
    g_log->set_pattern( "[Umihara Kawase Loader] [%D %T.%e] [%^%l%$] - %v" );
    
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
    // sig scan, etc
    //

    // find input handler func
    // follow relative jmp
    g_input_hander_func_addr = Utils::follow_rel_instruction( g_input_hander_func_addr );
    if( !g_input_hander_func_addr ) {
        g_log->error( L"Failed to find input handler func" );

        return 0;
    }

    g_log->info( L"Input handler func: 0x{:X}", g_input_hander_func_addr );

    // find key list
    auto tmp = PatternScan::find( "", "B8 ? ? ? ? 8D 9B ? ? ? ?" );
    if( !tmp ) {
        g_log->error( L"Failed to find key list array" );

        return 0;
    }

    // actual key list starts 4 bytes back
    g_key_list  = *(uint32_t **)( tmp );
    g_key_list -= 1;

    g_log->info( L"Key list array: 0x{:X}", (uintptr_t)g_key_list );

    //
    // set up hooks
    //

    // set up hooks
    if( !g_input_handler_hook.init( g_input_hander_func_addr, &input_handler ) ) {
        g_log->error( L"Failed to initialize input handler hook" );

        return 0;
    }
    
    // enable hooks
    if( !g_input_handler_hook.enable() ) {
        g_log->error( L"Failed to enable input handler hook" );

        return 0;
    }

    // finished...
    g_log->info( L"Initialization done..." );

    return 1;
}

int __stdcall DllMain( HINSTANCE instance, ulong_t reason_for_call, void *reserved ) {
    if( reason_for_call == DLL_PROCESS_ATTACH ) {
        // set up dinput8.dll wrapper
        // this must be done first
        // our init thread routine must wait
        if( !Dinput8Wrapper::init() )
            return 0;

        // ... now create our init thread
        const auto thread = CreateThread( nullptr, 0, init_thread, nullptr, 0, nullptr );
        if( !thread ) {
            g_log->error( L"Failed to create init thread" );

            return 0;
        }

        CloseHandle( thread );
    
        return 1;
    }

    return 0;
}