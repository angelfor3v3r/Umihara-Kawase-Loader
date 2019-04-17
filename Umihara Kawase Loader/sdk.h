#pragma once

#include "includes.h"

//
// struct padding magic
//

#define TOKENIZE_1( a, b ) a##b
#define TOKENIZE_2( a, b ) TOKENIZE_1( a, b )

#define PAD( amount )                                       \
    private:                                                \
        uint8_t TOKENIZE_2( _pad_, __COUNTER__ )[ amount ]; \
                                                            \
    public:

//
// fwd declare
//

class InputData;

//
// game func prototypes
//

using input_handler_t = int (__thiscall *)( InputData *data );

//
// game types, etc
//

enum GameVersion : int8_t {
    UMI_GAME_INVALID = -1,
    UMI_GAME_KAWASE,
    UMI_GAME_KAWASE_SHUN,
    UMI_GAME_SAYONARA_KAWASE
};

enum FocusStatus : uint32_t {
    UMI_NOT_FOCUSED = 2,
    UMI_FOCUSED     = 3
};

class alignas( 4 ) InputData {
public:
    IDirectInputDevice8W *m_dinput_device; // 0x0000

    PAD( 1144 );                           // 0x0004

    uint32_t m_focus_flag;                 // 0x047C

    PAD( 32 );                             // 0x0480

    uint32_t m_key_state;                  // 0x04A0
};

enum KeyStates : uint32_t {
    // movement and move UI selection keybinds
    UMI_KEY_UP     = 0x1, // DIK_UP
    UMI_KEY_DOWN   = 0x2, // DIK_DOWN
    UMI_KEY_LEFT   = 0x4, // DIK_LEFT
    UMI_KEY_RIGHT  = 0x8, // DIK_RIGHT

    // menu related keybinds
    UMI_KEY_START   = 0x10000, // DIK_SPACE
    UMI_KEY_PAUSE   = 0x4000,  // DIK_RETURN, DIK_NUMPADENTER
    UMI_KEY_RESTART = 0x10,    // DIK_S, DIK_Y
    UMI_KEY_BACK    = 0x20000, // DIK_ESCAPE, DIK_LCONTROL, DIK_RCONTROL

    // NOTE: this key is only valid on the first game (UmiharaKawase)
    UMI_KEY_SELECT  = 0x8000, // DIK_TAB

    // gameplay keybinds
    UMI_KEY_JUMP = 0x20,  // DIK_Z
    UMI_KEY_HOOK = 0x40,  // DIK_A
    UMI_KEY_L    = 0x100, // DIK_PRIOR
    UMI_KEY_R    = 0x200, // DIK_NEXT

    // misc keybinds
    UMI_KEY_SKIP = 0x80, // DIK_X
};