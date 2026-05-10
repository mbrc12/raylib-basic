#pragma once

#include "raylib.h"
#include <string_view>

namespace colors {

constexpr unsigned char hex_digit(char c) {
    return (c >= '0' && c <= '9') ? c - '0'
         : (c >= 'a' && c <= 'f') ? c - 'a' + 10
         : (c >= 'A' && c <= 'F') ? c - 'A' + 10
         : 0;
}

constexpr Color hex(std::string_view sv) {
    if (sv.size() >= 1 && sv[0] == '#') sv.remove_prefix(1);
    return {
        static_cast<unsigned char>(hex_digit(sv[0]) * 16 + hex_digit(sv[1])),
        static_cast<unsigned char>(hex_digit(sv[2]) * 16 + hex_digit(sv[3])),
        static_cast<unsigned char>(hex_digit(sv[4]) * 16 + hex_digit(sv[5])),
        255,
    };
}

constexpr Color Transparent{0, 0, 0, 0};
constexpr Color PureWhite   = hex("#ffffff");
constexpr Color PureBlack   = hex("#000000");

constexpr Color Fairydust_Sand   = hex("#f0dab1");
constexpr Color Fairydust_Blush  = hex("#e39aac");
constexpr Color Fairydust_Orchid = hex("#c45d9f");
constexpr Color Fairydust_Mauve  = hex("#634b7d");
constexpr Color Fairydust_Indigo = hex("#6461c2");
constexpr Color Fairydust_Aqua   = hex("#2ba9b4");
constexpr Color Fairydust_Mint   = hex("#93d4b5");
constexpr Color Fairydust_Pearl  = hex("#f0f6e8");
constexpr Color Fairydust_Pitch  = hex("#0a0f14");

constexpr Color NA_Lavender  = hex("#8c8fae");
constexpr Color NA_Purple    = hex("#584563");
constexpr Color NA_Plum      = hex("#3e2137");
constexpr Color NA_Sienna    = hex("#9a6348");
constexpr Color NA_Peach     = hex("#d79b7d");
constexpr Color NA_Cream     = hex("#f5edba");
constexpr Color NA_Lime      = hex("#c0c741");
constexpr Color NA_Olive     = hex("#647d34");
constexpr Color NA_Amber     = hex("#e4943a");
constexpr Color NA_Red       = hex("#9d303b");
constexpr Color NA_Rose      = hex("#d26471");
constexpr Color NA_Grape     = hex("#70377f");
constexpr Color NA_Teal      = hex("#7ec4c1");
constexpr Color NA_Turquoise = hex("#34859d");
constexpr Color NA_Midnight  = hex("#17434b");
constexpr Color NA_Obsidian  = hex("#1f0e1c");

constexpr Color NeonPulse8_DeepPurple   = hex("#1a0f2e");
constexpr Color NeonPulse8_Indigo       = hex("#241649");
constexpr Color NeonPulse8_RoyalPurple  = hex("#391f59");
constexpr Color NeonPulse8_MediumPurple = hex("#6e348c");
constexpr Color NeonPulse8_HotPink      = hex("#df35c1");
constexpr Color NeonPulse8_Lime         = hex("#c9ff00");
constexpr Color NeonPulse8_Cyan         = hex("#00e5e5");
constexpr Color NeonPulse8_SoftPink     = hex("#f4d0d0");

constexpr Color CC29_Eggshell     = hex("#f2f0e5");
constexpr Color CC29_Silver       = hex("#b8b5b9");
constexpr Color CC29_StormGray    = hex("#868188");
constexpr Color CC29_Ash          = hex("#646365");
constexpr Color CC29_Charcoal     = hex("#45444f");
constexpr Color CC29_SlateIndigo  = hex("#3a3858");
constexpr Color CC29_OnyxBlack    = hex("#212123");
constexpr Color CC29_Plum         = hex("#352b42");
constexpr Color CC29_Periwinkle   = hex("#43436a");
constexpr Color CC29_Azure        = hex("#4b80ca");
constexpr Color CC29_Aqua         = hex("#68c2d3");
constexpr Color CC29_Seafoam      = hex("#a2dcc7");
constexpr Color CC29_PaleGold     = hex("#ede19e");
constexpr Color CC29_BurntSienna  = hex("#d3a068");
constexpr Color CC29_Brick        = hex("#b45252");
constexpr Color CC29_DustyMauve   = hex("#6a536e");
constexpr Color CC29_Eggplant     = hex("#4b4158");
constexpr Color CC29_RusticBrown  = hex("#80493a");
constexpr Color CC29_Mocha        = hex("#a77b5b");
constexpr Color CC29_Almond       = hex("#e5ceb4");
constexpr Color CC29_Chartreuse   = hex("#c2d368");
constexpr Color CC29_PeaGreen     = hex("#8ab060");
constexpr Color CC29_Teal         = hex("#567b79");
constexpr Color CC29_Olive        = hex("#4e584a");
constexpr Color CC29_Khaki        = hex("#7b7243");
constexpr Color CC29_Sage         = hex("#b2b47e");
constexpr Color CC29_Blush        = hex("#edc8c4");
constexpr Color CC29_Orchid       = hex("#cf8acb");
constexpr Color CC29_Grape        = hex("#5f556a");

constexpr Color SteamLords_ForestGreen    = hex("#213b25");
constexpr Color SteamLords_DeepFern       = hex("#3a604a");
constexpr Color SteamLords_MossGreen      = hex("#4f7754");
constexpr Color SteamLords_MutedOlive     = hex("#a19f7c");
constexpr Color SteamLords_TaupeGreen     = hex("#77744f");
constexpr Color SteamLords_SiennaBrown    = hex("#775c4f");
constexpr Color SteamLords_Mahogany       = hex("#603b3a");
constexpr Color SteamLords_Aubergine      = hex("#3b2137");
constexpr Color SteamLords_MidnightBlack  = hex("#170e19");
constexpr Color SteamLords_IndigoBerry    = hex("#2f213b");
constexpr Color SteamLords_EggplantPurple = hex("#433a60");
constexpr Color SteamLords_SlateViolet    = hex("#4f5277");
constexpr Color SteamLords_SteelBlue      = hex("#65738c");
constexpr Color SteamLords_StormBlue      = hex("#7c94a1");
constexpr Color SteamLords_SeaGlass       = hex("#a0b9ba");
constexpr Color SteamLords_PaleTeal       = hex("#c0d1cc");

} // namespace colors
