#pragma once
#include <windows.h>

// "Terminal Dracula" color scheme
namespace Colors
{
    constexpr COLORREF BackgroundPrimary   = RGB(40, 42, 54);    // #282a36
    constexpr COLORREF BackgroundSecondary = RGB(68, 71, 90);    // #44475a
    constexpr COLORREF BackgroundTertiary  = RGB(55, 57, 73);    // #373949
    constexpr COLORREF Foreground          = RGB(248, 248, 242); // #f8f8f2
    constexpr COLORREF Comment             = RGB(98, 114, 164);  // #6272a4
    constexpr COLORREF Cyan                = RGB(139, 233, 253); // #8be9fd
    constexpr COLORREF Green               = RGB(80, 250, 123);  // #50fa7b
    constexpr COLORREF Orange              = RGB(255, 184, 108); // #ffb86c
    constexpr COLORREF Pink                = RGB(255, 121, 198); // #ff79c6
    constexpr COLORREF Purple              = RGB(189, 147, 249); // #bd93f9
    constexpr COLORREF Red                 = RGB(255, 85, 85);   // #ff5555
    constexpr COLORREF Yellow              = RGB(241, 250, 140); // #f1fa8c
    constexpr COLORREF Olive               = RGB(128, 128, 0);   // #808000 - military/contractor
}
