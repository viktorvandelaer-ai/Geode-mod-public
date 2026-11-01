#pragma once
#include <Geode/Geode.hpp>
#include "Themes.hpp"
#include <string>

struct DecoOptions {
    Theme theme;
    float density = 0.6f;       // multiplier
    unsigned seed = 0;          // 0 = random
    bool selectionOnly = true;  // use selected objects if any
    bool addTriggers = true;
    bool bgPanels = true;
    int maxObjects = 3000;
};

struct DecoResult {
    int created = 0;
    int solids = 0;
    std::string error;
};

// Decorates the layout. Everything created is no-touch decoration.
DecoResult decorateLayout(LevelEditorLayer* lel, EditorUI* ui, DecoOptions const& opts);

// Removes the objects created by the most recent decorateLayout call.
int removeLastDeco(LevelEditorLayer* lel);
