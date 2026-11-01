#include "Themes.hpp"

using namespace geode::prelude;

static FeatureWeights W(float trim, float glow, float blades, float hang, float flo,
                        float spark, float pill, float bg, float rain) {
    FeatureWeights w;
    w.v[F_Trim] = trim;     w.v[F_Glow] = glow;       w.v[F_Blades] = blades;
    w.v[F_Hangers] = hang;  w.v[F_Floaters] = flo;    w.v[F_Sparkles] = spark;
    w.v[F_Pillars] = pill;  w.v[F_BgPanels] = bg;     w.v[F_Rain] = rain;
    return w;
}

const std::vector<Theme>& allThemes() {
    static const std::vector<Theme> themes = {
        { "Neon",
          { {10,5,30}, {20,10,50}, {0,255,255}, {255,0,200}, {140,60,255}, {255,255,255} },
          W(1.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.7f, 0.3f, 0.6f, 0.0f),
          { "neon", "cyber", "synth", "synthwave", "retro", "electric", "vaporwave", "80s", "disco" } },
        { "Space",
          { {3,3,15}, {25,25,60}, {120,160,255}, {200,120,255}, {255,255,255}, {255,255,200} },
          W(0.6f, 0.4f, 0.0f, 0.0f, 0.8f, 1.0f, 0.2f, 0.4f, 0.0f),
          { "space", "galaxy", "stars", "cosmic", "planet", "night", "universe", "nebula", "moon" } },
        { "Forest",
          { {15,40,20}, {40,25,10}, {80,200,70}, {40,140,50}, {110,70,30}, {200,255,150} },
          W(0.8f, 0.1f, 1.0f, 0.5f, 0.5f, 0.2f, 0.4f, 0.4f, 0.0f),
          { "forest", "grass", "jungle", "nature", "tree", "plants", "leaves", "garden", "swamp", "vines" } },
        { "Lava",
          { {40,5,0}, {60,15,5}, {255,80,0}, {255,200,0}, {120,20,10}, {255,230,120} },
          W(0.8f, 0.8f, 0.3f, 0.9f, 0.7f, 0.5f, 0.3f, 0.5f, 0.0f),
          { "lava", "fire", "hell", "volcano", "magma", "demon", "inferno", "ember", "hot", "flame" } },
        { "Ice",
          { {10,30,60}, {60,110,160}, {180,240,255}, {110,190,255}, {255,255,255}, {255,255,255} },
          W(0.8f, 0.5f, 0.0f, 1.0f, 0.6f, 0.6f, 0.2f, 0.4f, 0.0f),
          { "ice", "snow", "frozen", "winter", "cold", "icicle", "arctic", "frost", "christmas" } },
        { "Candy",
          { {255,170,210}, {255,120,180}, {255,255,255}, {120,220,255}, {255,235,100}, {255,255,255} },
          W(1.0f, 0.4f, 0.3f, 0.3f, 0.9f, 0.6f, 0.1f, 0.6f, 0.0f),
          { "candy", "sweet", "pastel", "cute", "sugar", "cake", "rainbow", "kawaii" } },
        { "Tech",
          { {5,15,10}, {10,40,25}, {0,255,120}, {0,180,255}, {255,255,0}, {200,255,220} },
          W(1.0f, 0.9f, 0.0f, 0.0f, 0.4f, 0.3f, 0.6f, 0.8f, 0.6f),
          { "tech", "matrix", "digital", "circuit", "pixel", "hacker", "code", "robot", "machine", "factory" } },
        { "Ocean",
          { {0,40,90}, {0,70,110}, {0,200,220}, {100,255,200}, {0,100,180}, {200,255,255} },
          W(0.5f, 0.3f, 1.0f, 0.7f, 1.0f, 0.4f, 0.2f, 0.5f, 0.2f),
          { "ocean", "sea", "water", "underwater", "aqua", "coral", "fish", "bubble", "deep" } },
        { "Minimal",
          { {25,25,30}, {45,45,55}, {255,255,255}, {170,170,190}, {90,90,110}, {255,255,255} },
          W(0.9f, 0.3f, 0.0f, 0.0f, 0.15f, 0.1f, 0.0f, 0.2f, 0.0f),
          { "minimal", "clean", "simple", "plain", "basic", "monochrome", "subtle" } },
        { "Sunset",
          { {60,20,60}, {90,40,40}, {255,140,60}, {255,90,120}, {255,210,100}, {255,240,200} },
          W(0.8f, 0.6f, 0.4f, 0.1f, 0.5f, 0.4f, 0.5f, 0.7f, 0.0f),
          { "sunset", "desert", "sun", "dusk", "warm", "sand", "dawn", "sunrise" } },
    };
    return themes;
}

int findThemeIndex(std::string const& name) {
    auto const& t = allThemes();
    for (size_t i = 0; i < t.size(); i++) if (t[i].name == name) return (int)i;
    return 0;
}
