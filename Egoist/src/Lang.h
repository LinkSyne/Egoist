#pragma once

enum class Str {
    WindowTitle, AppName, Description,
    BtnDisable, BtnAbout, BtnSettings,
    PushTitle, PushInfo, BtnNext, BtnBack,
    BlockBrowser, BlockWindows, BlockGames, BlockSteam, BlockTelegram,
    NoteBrowser, NoteWindows, NoteApps, Reverted, NeedAdmin,
    AboutTitle, Version, BtnAuthor, BtnSite,
    SettingsTitle, LangLabel, ThemeLabel, Light, Dark,
    Count
};

const wchar_t* T(Str s);
