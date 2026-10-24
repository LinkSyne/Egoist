#include "Lang.h"
#include "Config.h"

static const wchar_t* RU[] = {
    L"Egoist Beta",
    L"Egoist",
    L"Программа которая избавит тебя от дискомфорта! Ты можешь отключить уведомления и многое другое",
    L"Выключить уведомления",
    L"О программе",
    L"Настройки",
    L"Отключение Push - уведомлений",
    L"Отключение идет прямо на все браузеры",
    L"Далее",
    L"Назад",
    L"Браузеры (Push-уведомления сайтов)",
    L"Системные уведомления Windows",
    L"Игры (Xbox Game Bar лаунчеры)",
    L"Steam",
    L"Telegram",
    L"Success Перезапусти браузеры чтобы изменения вступили в силу",
    L"Success Если уведомления ещё приходят перезапусти ПК",
    L"Success Если уведомления ещё приходят перезапусти программу",
    L"Блокировка снята",
    L"Не удалось применить изменения. Запусти программу от имени администратора.",
    L"О программе",
    L"Версия: 1.0.0",
    L"Автор",
    L"Сайт автора",
    L"Настройки",
    L"Язык программы",
    L"Тема программы",
    L"Светлая",
    L"Темная",
};

static const wchar_t* EN[] = {
    L"Egoist Beta",
    L"Egoist",
    L"A program that will free you from discomfort! You can turn off notifications and much more",
    L"Turn off notifications",
    L"About",
    L"Settings",
    L"Turning off Push notifications",
    L"Applies directly to all browsers",
    L"Next",
    L"Back",
    L"Browsers (website Push notifications)",
    L"Windows system notifications",
    L"Games (Xbox Game Bar, launchers)",
    L"Steam",
    L"Telegram",
    L"Success. Restart your browsers for the changes to take effect",
    L"Success. If notifications still appear, restart your PC",
    L"Success. If notifications still appear, restart the program",
    L"Block removed",
    L"Could not apply the changes. Run the program as administrator.",
    L"About",
    L"Version: 1.0.0",
    L"Author",
    L"Author's website",
    L"Settings",
    L"Program language",
    L"Program theme",
    L"Light",
    L"Dark",
};

static_assert(sizeof(RU) / sizeof(RU[0]) == (size_t)Str::Count, "RU table size mismatch");
static_assert(sizeof(EN) / sizeof(EN[0]) == (size_t)Str::Count, "EN table size mismatch");

const wchar_t* T(Str s) {
    return (Cfg().lang == Lang::RU ? RU : EN)[(int)s];
}
