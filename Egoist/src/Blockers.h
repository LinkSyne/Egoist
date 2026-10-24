#pragma once
#include <vector>
#include "Lang.h"

struct Blocker {
    Str label;
    Str note;
    bool (*isActive)();
    bool (*set)(bool on);
};

const std::vector<Blocker>& Blockers();
void ReapplyActive();
