#pragma once

enum class Lang  : int { RU = 0, EN = 1 };
enum class Theme : int { Light = 0, Dark = 1 };

struct Config {
    Lang  lang  = Lang::RU;
    Theme theme = Theme::Light;
};

Config& Cfg();
void LoadConfig();
void SaveConfig();
