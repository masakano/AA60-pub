//
//
//
#pragma once
#include <spu++/spu_gesture.h>

namespace spu {
namespace {

const char* s_symbolmap[] = {

        "NUL",    // 00
        "SOH",    // 01
        "STX",    // 02
        "ETX",    // 03
        "EOT",    // 04
        "ENQ",    // 05
        "ACK",    // 06
        "BEL",    // 07
        "BS ",    // 08
        "HT ",    // 09
        "LF ",    // 0A
        "VT ",    // 0B
        "FF ",    // 0C
        "CR ",    // 0D
        "SO ",    // 0E
        "SI ",    // 0F
        "DLE",    // 10
        "DC1",    // 11
        "DC2",    // 12
        "DC3",    // 13
        "DC4",    // 14
        "NAK",    // 15
        "SYN",    // 16
        "ETB",    // 17
        "CAN",    // 18
        "EM ",    // 19
        "SUB",    // 1A
        "ESC",    // 1B
        "FS ",    // 1C
        "GS ",    // 1D
        "RS ",    // 1E
        "US ",    // 1F
        "SPAC",   // 20
        "!",      // 21
        "\"",     // 22
        "#",      // 23
        "$",      // 24
        "%",      // 25
        "&",      // 26
        "'",      // 27
        "(",      // 28
        ")",      // 29
        "*",      // 2A
        "+",      // 2B
        ",",      // 2C
        "-",      // 2D
        ".",      // 2E
        "/",      // 2F
        "0",      // 30
        "1",      // 31
        "2",      // 32
        "3",      // 33
        "4",      // 34
        "5",      // 35
        "6",      // 36
        "7",      // 37
        "8",      // 38
        "9",      // 39
        ":",      // 3A
        ";",      // 3B
        "<",      // 3C
        "=",      // 3D
        ">",      // 3E
        "?",      // 3F
        "@",      // 40
        "A",      // 41
        "B",      // 42
        "C",      // 43
        "D",      // 44
        "E",      // 45
        "F",      // 46
        "G",      // 47
        "H",      // 48
        "I",      // 49
        "J",      // 4A
        "K",      // 4B
        "L",      // 4C
        "M",      // 4D
        "N",      // 4E
        "O",      // 4F
        "P",      // 50
        "Q",      // 51
        "R",      // 52
        "S",      // 53
        "T",      // 54
        "U",      // 55
        "V",      // 56
        "W",      // 57
        "X",      // 58
        "Y",      // 59
        "Z",      // 5A
        "[",      // 5B
        "\\",     // 5C
        "]",      // 5D
        "^",      // 5E
        "SPACE",  // 5F
        "`",      // 60
        "a",      // 61
        "b",      // 62
        "c",      // 63
        "d",      // 64
        "e",      // 65
        "f",      // 66
        "g",      // 67
        "h",      // 68
        "i",      // 69
        "j",      // 6A
        "k",      // 6B
        "l",      // 6C
        "m",      // 6D
        "n",      // 6E
        "o",      // 6F
        "p",      // 70
        "q",      // 71
        "r",      // 72
        "s",      // 73
        "t",      // 74
        "u",      // 75
        "v",      // 76
        "w",      // 77
        "x",      // 78
        "y",      // 79
        "z",      // 7A
        "{",      // 7B
        "|",      // 7C
        "}",      // 7D
        "~",      // 7E
        "DEL",    // 7F

        "f1",        // SpuPad::e_f1
        "f2",        // SpuPad::e_f2
        "f3",        // SpuPad::e_f3
        "f4",        // SpuPad::e_f4
        "f5",        // SpuPad::e_f5
        "f6",        // SpuPad::e_f6
        "f7",        // SpuPad::e_f7
        "f8",        // SpuPad::e_f8
        "f9",        // SpuPad::e_f9
        "f10",       // SpuPad::e_f10
        "f11",       // SpuPad::e_f11
        "f12",       // SpuPad::e_f12
        "up",        // SpuPad::e_up
        "down",      // SpuPad::e_down
        "left",      // SpuPad::e_left
        "right",     // SpuPad::e_right
        "lshift",    // SpuPad::e_lshift
        "rshift",    // SpuPad::e_rshift
        "lctrl",     // SpuPad::e_lctrl
        "rctrl",     // SpuPad::e_rctrl
        "lalt",      // SpuPad::e_lalt
        "ralt",      // SpuPad::e_ralt
        "home",      // SpuPad::e_home
        "pageup",    // SpuPad::e_pageup
        "pagedown",  // SpuPad::e_pagedown
        "end",       // SpuPad::e_end
        nullptr,     // sentinel
};
}  // namespace
}  // namespace spu
