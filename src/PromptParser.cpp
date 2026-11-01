#include "PromptParser.hpp"
#include <algorithm>
#include <cctype>

using namespace geode::prelude;

namespace {
    bool match(std::string const& tok, std::string const& kw) {
        return tok == kw || tok == kw + "s" || tok + "s" == kw;
    }

    std::string lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        return s;
    }

    ccColor3B mix(ccColor3B a, ccColor3B b, float t) {
        auto m = [&](GLubyte x, GLubyte y) { return (GLubyte)std::clamp(x + (y - x) * t, 0.f, 255.f); };
        return { m(a.r, b.r), m(a.g, b.g), m(a.b, b.b) };
    }

    ccColor3B scale(ccColor3B c, float f) {
        auto s = [&](GLubyte x) { return (GLubyte)std::clamp(x * f, 0.f, 255.f); };
        return { s(c.r), s(c.g), s(c.b) };
    }

    struct FeatureWord { const char* word; Feature f; const char* label; };
    const FeatureWord FEATURE_WORDS[] = {
        {"stars", F_Sparkles, "stars"}, {"sparkle", F_Sparkles, "sparkles"}, {"glitter", F_Sparkles, "sparkles"},
        {"twinkle", F_Sparkles, "sparkles"}, {"diamonds", F_Sparkles, "sparkles"},
        {"grass", F_Blades, "grass"}, {"blades", F_Blades, "grass"}, {"plants", F_Blades, "grass"},
        {"vines", F_Blades, "grass"}, {"weeds", F_Blades, "grass"}, {"seaweed", F_Blades, "grass"},
        {"icicles", F_Hangers, "hangers"}, {"stalactites", F_Hangers, "hangers"}, {"drips", F_Hangers, "hangers"},
        {"hanging", F_Hangers, "hangers"}, {"roots", F_Hangers, "hangers"}, {"dripping", F_Hangers, "hangers"},
        {"particles", F_Floaters, "floaters"}, {"floating", F_Floaters, "floaters"}, {"floaters", F_Floaters, "floaters"},
        {"bubbles", F_Floaters, "floaters"}, {"confetti", F_Floaters, "floaters"}, {"dust", F_Floaters, "floaters"},
        {"embers", F_Floaters, "floaters"}, {"squares", F_Floaters, "floaters"},
        {"pillars", F_Pillars, "pillars"}, {"columns", F_Pillars, "pillars"}, {"towers", F_Pillars, "pillars"},
        {"background", F_BgPanels, "background"}, {"bg", F_BgPanels, "background"}, {"panels", F_BgPanels, "background"},
        {"stripes", F_BgPanels, "background"}, {"skyline", F_BgPanels, "background"}, {"buildings", F_BgPanels, "background"},
        {"rain", F_Rain, "rain"}, {"streaks", F_Rain, "rain"}, {"falling", F_Rain, "rain"}, {"lines", F_Rain, "rain"},
        {"glow", F_Glow, "glow"}, {"neon", F_Glow, "glow"}, {"borders", F_Glow, "glow"}, {"outline", F_Glow, "glow"},
        {"trim", F_Trim, "trim"}, {"edge", F_Trim, "trim"}, {"edges", F_Trim, "trim"},
    };

    struct ColorWord { const char* word; ccColor3B c; };
    const ColorWord COLOR_WORDS[] = {
        {"red", {255,50,50}}, {"orange", {255,140,30}}, {"yellow", {255,235,50}}, {"gold", {255,200,40}},
        {"green", {60,230,80}}, {"lime", {160,255,40}}, {"teal", {0,200,170}}, {"cyan", {0,240,255}},
        {"blue", {50,110,255}}, {"purple", {160,60,255}}, {"violet", {170,90,255}}, {"pink", {255,100,200}},
        {"magenta", {255,0,200}}, {"white", {255,255,255}}, {"black", {35,35,40}}, {"gray", {150,150,160}},
        {"grey", {150,150,160}}, {"brown", {140,90,40}},
    };
}

std::string applyPrompt(std::string const& prompt, Theme& theme, float& density) {
    std::vector<std::string> toks;
    std::string cur;
    for (char ch : prompt) {
        if (std::isalnum((unsigned char)ch)) cur += (char)std::tolower((unsigned char)ch);
        else if (!cur.empty()) { toks.push_back(cur); cur.clear(); }
    }
    if (!cur.empty()) toks.push_back(cur);
    if (toks.empty()) return "";

    std::vector<std::string> notes;

    // 1) theme
    int best = 0;
    const Theme* bestTheme = nullptr;
    for (auto const& t : allThemes()) {
        int score = 0;
        for (auto const& tok : toks) {
            if (tok == lower(t.name)) score += 2;
            for (auto const& kw : t.keywords) if (match(tok, kw)) score += 1;
        }
        if (score > best) { best = score; bestTheme = &t; }
    }
    if (bestTheme) {
        theme = *bestTheme;
        notes.push_back("theme " + bestTheme->name);
    }

    // 2) features (with negation + "only")
    bool only = std::find(toks.begin(), toks.end(), "only") != toks.end();
    bool mentioned[F_Count] = {false};
    std::vector<std::pair<Feature, bool>> ops;
    for (size_t i = 0; i < toks.size(); i++) {
        bool neg = i > 0 && (toks[i-1] == "no" || toks[i-1] == "without" || toks[i-1] == "not" || toks[i-1] == "remove");
        for (auto const& fw : FEATURE_WORDS) {
            if (match(toks[i], fw.word)) {
                ops.push_back({fw.f, !neg});
                if (!neg) mentioned[fw.f] = true;
                notes.push_back(std::string(neg ? "no " : "") + fw.label);
                break;
            }
        }
    }
    if (only) {
        bool any = false;
        for (int i = 0; i < F_Count; i++) any = any || mentioned[i];
        if (any) for (int i = 0; i < F_Count; i++) if (!mentioned[i]) theme.w.v[i] = 0.f;
    }
    for (auto const& [f, on] : ops) {
        if (on) theme.w.v[f] = std::max(theme.w.v[f], 1.0f);
        else theme.w.v[f] = 0.f;
    }

    // 3) density words
    for (auto const& tok : toks) {
        if (match(tok, "dense") || match(tok, "lots") || match(tok, "many") || match(tok, "busy") ||
            match(tok, "heavy") || match(tok, "packed") || match(tok, "full") || match(tok, "detailed")) {
            density *= 1.6f; notes.push_back("denser");
        } else if (match(tok, "sparse") || match(tok, "minimal") || match(tok, "subtle") ||
                   match(tok, "simple") || match(tok, "few") || match(tok, "clean")) {
            density *= 0.5f; notes.push_back("sparser");
        }
    }

    // 4) brightness
    for (auto const& tok : toks) {
        if (tok == "dark" || tok == "darker") {
            theme.pal.bg = scale(theme.pal.bg, 0.45f); theme.pal.ground = scale(theme.pal.ground, 0.5f);
            notes.push_back("dark");
        } else if (tok == "bright" || tok == "brighter") {
            theme.pal.bg = scale(theme.pal.bg, 1.6f); theme.pal.ground = scale(theme.pal.ground, 1.4f);
            notes.push_back("bright");
        }
    }

    // 5) color tint (first color word wins; second one becomes accent 2)
    std::vector<ccColor3B> tints;
    std::vector<std::string> tintNames;
    for (auto const& tok : toks)
        for (auto const& cw : COLOR_WORDS)
            if (tok == cw.word) { tints.push_back(cw.c); tintNames.push_back(cw.word); }
    if (!tints.empty()) {
        auto c = tints[0];
        theme.pal.accent1 = c;
        theme.pal.accent2 = tints.size() > 1 ? tints[1] : mix(c, {255,255,255}, 0.4f);
        theme.pal.accent3 = mix(c, theme.pal.bg, 0.45f);
        theme.pal.glow = mix(c, {255,255,255}, 0.75f);
        if (tintNames[0] != "black" && tintNames[0] != "white" && tintNames[0] != "gray" && tintNames[0] != "grey")
            theme.pal.bg = mix(theme.pal.bg, scale(c, 0.25f), 0.5f);
        notes.push_back("color " + tintNames[0] + (tints.size() > 1 ? "/" + tintNames[1] : ""));
    }

    density = std::clamp(density, 0.05f, 2.0f);

    std::string out;
    for (size_t i = 0; i < notes.size(); i++) out += (i ? ", " : "") + notes[i];
    return out;
}
