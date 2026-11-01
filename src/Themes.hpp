#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

enum Feature {
    F_Trim = 0,   // bright strip along tops of platforms
    F_Glow,       // glow strip + edge outlines
    F_Blades,     // grass / spikes-like blades on top
    F_Hangers,    // icicles / stalactites / roots under blocks
    F_Floaters,   // floating squares / particles / bubbles
    F_Sparkles,   // small diamonds / stars
    F_Pillars,    // tall background columns
    F_BgPanels,   // wide translucent background panels
    F_Rain,       // falling streaks
    F_Count
};

struct FeatureWeights { float v[F_Count] = {0}; };

struct Palette {
    cocos2d::ccColor3B bg, ground, accent1, accent2, accent3, glow;
};

struct Theme {
    std::string name;
    Palette pal;
    FeatureWeights w;
    std::vector<std::string> keywords;
};

const std::vector<Theme>& allThemes();
int findThemeIndex(std::string const& name);
