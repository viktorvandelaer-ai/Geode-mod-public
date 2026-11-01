#include "DecoEngine.hpp"
#include <algorithm>
#include <cmath>
#include <random>
#include <unordered_set>

using namespace geode::prelude;

namespace {
    constexpr float C = 30.f;     // grid cell size
    constexpr int CH_A1 = 201, CH_A2 = 202, CH_A3 = 203, CH_GLOW = 204, CH_PANEL = 205;

    std::vector<Ref<GameObject>> g_last;

    struct Ctx {
        LevelEditorLayer* lel;
        std::mt19937 rng;
        int cap = 3000;
        std::vector<Ref<GameObject>> made;
    };

    float rf(Ctx& c, float a, float b) { return std::uniform_real_distribution<float>(a, b)(c.rng); }
    int ri(Ctx& c, int a, int b) { return std::uniform_int_distribution<int>(a, b)(c.rng); }
    bool chance(Ctx& c, float p) { return p > 0.f && rf(c, 0.f, 1.f) < p; }
    long long K(int x, int y) { return (long long)x * 1000003LL + y; }

    bool nearAny(std::unordered_set<long long> const& s, int x, int y, int r) {
        for (int dx = -r; dx <= r; dx++)
            for (int dy = -r; dy <= r; dy++)
                if (s.count(K(x + dx, y + dy))) return true;
        return false;
    }

    struct Run { int major, start, len; };
    std::vector<Run> makeRuns(std::vector<std::pair<int, int>> v) {
        std::sort(v.begin(), v.end());
        std::vector<Run> out;
        for (auto& [m, n] : v) {
            if (!out.empty() && out.back().major == m && out.back().start + out.back().len == n) out.back().len++;
            else out.push_back({ m, n, 1 });
        }
        return out;
    }

    // Places one non-solid decoration block (scaled, rotated, recolored).
    GameObject* place(Ctx& c, int id, float x, float y, float sx, float sy, float rot,
                      int channel, ZLayer z, int zOrder) {
        if ((int)c.made.size() >= c.cap) return nullptr;
        auto obj = c.lel->createObject(id, { x, y }, false);
        if (!obj) return nullptr;
        obj->updateCustomScaleX(sx);
        obj->updateCustomScaleY(sy);
        if (rot != 0.f) obj->setRotation(rot);
        obj->m_isNoTouch = true;
        obj->m_zLayer = z;
        obj->m_zOrder = zOrder;
        if (obj->m_baseColor) obj->m_baseColor->m_colorID = channel;
        c.made.push_back(obj);
        return obj;
    }

    void colorTrigger(Ctx& c, float x, float y, int channel, ccColor3B col, float opacity) {
        if ((int)c.made.size() >= c.cap + 32) return;
        auto obj = c.lel->createObject(899, { x, y }, false);
        if (!obj) return;
        auto t = typeinfo_cast<EffectGameObject*>(obj);
        if (!t) return;
        t->m_targetColorID = channel;
        t->m_triggerTargetColor = col;
        t->m_duration = 0.f;
        t->m_opacity = opacity;
        c.made.push_back(obj);
    }
}

DecoResult decorateLayout(LevelEditorLayer* lel, EditorUI* ui, DecoOptions const& o) {
    DecoResult res;
    if (!lel) { res.error = "No editor layer."; return res; }

    // ---- 1. collect source objects ----
    CCArray* src = nullptr;
    if (o.selectionOnly && ui) {
        auto sel = ui->getSelectedObjects();
        if (sel && sel->count() > 0) src = sel;
    }
    if (!src) src = lel->m_objects;
    if (!src || src->count() == 0) { res.error = "The level has no objects to decorate."; return res; }

    std::unordered_set<long long> solids, occupied;
    std::vector<std::pair<int, int>> solidList;
    float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;

    for (auto obj : CCArrayExt<GameObject*>(src)) {
        if (!obj) continue;
        auto type = obj->m_objectType;
        bool isSolid = type == GameObjectType::Solid;
        bool isHazard = type == GameObjectType::Hazard;
        bool isSlope = type == GameObjectType::Slope;
        if (!isSolid && !isHazard && !isSlope) continue;

        auto p = obj->getPosition();
        int cx = (int)std::floor(p.x / C);
        int cy = (int)std::floor(p.y / C);
        occupied.insert(K(cx, cy));
        if (isSolid && solids.insert(K(cx, cy)).second) solidList.push_back({ cx, cy });
        minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
    }
    if (solids.empty()) { res.error = "No solid blocks found in the layout."; return res; }
    res.solids = (int)solids.size();

    // ---- 2. exposure analysis ----
    std::vector<std::pair<int, int>> topH, botH, leftV, rightV;   // topH: (cy, cx) etc.
    std::vector<std::pair<int, int>> topCells, botCells;          // (cx, cy)
    for (auto const& [cx, cy] : solidList) {
        if (!solids.count(K(cx, cy + 1))) { topH.push_back({ cy, cx }); topCells.push_back({ cx, cy }); }
        if (!solids.count(K(cx, cy - 1))) { botH.push_back({ cy, cx }); botCells.push_back({ cx, cy }); }
        if (!solids.count(K(cx - 1, cy))) leftV.push_back({ cx, cy });
        if (!solids.count(K(cx + 1, cy))) rightV.push_back({ cx, cy });
    }

    // ---- 3. setup ----
    Ctx c;
    c.lel = lel;
    c.cap = o.maxObjects;
    c.rng.seed(o.seed ? o.seed : std::random_device{}());

    auto const& pal = o.theme.pal;
    auto const& W = o.theme.w.v;
    float D = o.density;
    auto P = [&](int f, float mul = 1.f) { return std::min(1.f, W[f] * D * mul); };
    const int randChannels[3] = { CH_A1, CH_A2, CH_A3 };

    // ---- 4. trim + glow along platform tops ----
    for (auto const& r : makeRuns(topH)) {
        float x = (r.start + r.len / 2.f) * C;
        float top = (r.major + 1) * C;
        if (chance(c, P(F_Trim, 1.6f)))
            place(c, 1, x, top + 1.5f, (float)r.len, 0.1f, 0.f, CH_A1, ZLayer::T1, 1);
        if (chance(c, P(F_Glow, 1.3f)))
            place(c, 1, x, top + 3.75f, (float)r.len, 0.25f, 0.f, CH_GLOW, ZLayer::B2, -2);
    }
    // bottoms
    for (auto const& r : makeRuns(botH)) {
        if (!chance(c, P(F_Glow, 1.0f))) continue;
        float x = (r.start + r.len / 2.f) * C;
        place(c, 1, x, r.major * C - 1.5f, (float)r.len, 0.1f, 0.f, CH_A2, ZLayer::T1, 1);
    }
    // sides
    for (auto const& r : makeRuns(leftV)) {
        if (!chance(c, P(F_Glow, 1.0f))) continue;
        place(c, 1, r.major * C - 1.5f, (r.start + r.len / 2.f) * C, 0.1f, (float)r.len, 0.f, CH_A2, ZLayer::T1, 1);
    }
    for (auto const& r : makeRuns(rightV)) {
        if (!chance(c, P(F_Glow, 1.0f))) continue;
        place(c, 1, (r.major + 1) * C + 1.5f, (r.start + r.len / 2.f) * C, 0.1f, (float)r.len, 0.f, CH_A2, ZLayer::T1, 1);
    }

    // ---- 5. blades / pillars on tops ----
    for (auto const& [cx, cy] : topCells) {
        float top = (cy + 1) * C;
        if (chance(c, P(F_Blades, 0.8f))) {
            int n = ri(c, 1, 3);
            for (int i = 0; i < n; i++) {
                float h = rf(c, 8.f, 28.f);
                place(c, 1, (cx + 0.5f) * C + rf(c, -12.f, 12.f), top + h / 2.f, rf(c, 0.06f, 0.12f), h / C,
                      rf(c, -18.f, 18.f), CH_A2, ZLayer::B2, -1);
            }
        }
        if (chance(c, P(F_Pillars, 0.08f))) {
            int tall = ri(c, 3, 8);
            place(c, 1, (cx + 0.5f) * C + rf(c, -8.f, 8.f), top + tall * C / 2.f, 0.2f, (float)tall, 0.f,
                  CH_PANEL, ZLayer::B4, -3);
        }
    }

    // ---- 6. hangers under bottoms ----
    for (auto const& [cx, cy] : botCells) {
        if (!chance(c, P(F_Hangers, 0.6f))) continue;
        float h = rf(c, 10.f, 45.f);
        place(c, 1, (cx + 0.5f) * C + rf(c, -10.f, 10.f), cy * C - h / 2.f, rf(c, 0.08f, 0.18f), h / C,
              rf(c, -8.f, 8.f), CH_A3, ZLayer::B2, -1);
    }

    // ---- 7. ambient: floaters, sparkles, rain ----
    float wCells = (maxX - minX) / C + 10.f, hCells = (maxY - minY) / C + 10.f;
    float area = std::min(wCells * hCells, 20000.f);
    float x0 = minX - 150.f, x1 = maxX + 150.f, y0 = minY - 150.f, y1 = maxY + 150.f;

    auto freeSpot = [&](float& x, float& y) {
        for (int tries = 0; tries < 6; tries++) {
            x = rf(c, x0, x1); y = rf(c, y0, y1);
            if (!nearAny(occupied, (int)std::floor(x / C), (int)std::floor(y / C), 1)) return true;
        }
        return false;
    };

    int nFloat = (int)std::min(500.f, area * 0.04f * D * W[F_Floaters]);
    for (int i = 0; i < nFloat; i++) {
        float x, y; if (!freeSpot(x, y)) continue;
        place(c, 1, x, y, rf(c, 0.08f, 0.3f), 0.f + rf(c, 0.08f, 0.3f), rf(c, 0.f, 90.f),
              randChannels[ri(c, 0, 2)], ZLayer::B4, -2);
    }
    int nSpark = (int)std::min(500.f, area * 0.02f * D * W[F_Sparkles]);
    for (int i = 0; i < nSpark; i++) {
        float x, y; if (!freeSpot(x, y)) continue;
        float s = rf(c, 0.05f, 0.16f);
        place(c, 1, x, y, s, s, 45.f, CH_GLOW, ZLayer::T1, 0);
    }
    int nRain = (int)std::min(400.f, area * 0.015f * D * W[F_Rain]);
    for (int i = 0; i < nRain; i++) {
        float x, y; if (!freeSpot(x, y)) continue;
        place(c, 1, x, y, 0.03f, rf(c, 0.5f, 1.5f), 12.f, CH_GLOW, ZLayer::B4, -2);
    }

    // ---- 8. background panels ----
    if (o.bgPanels && W[F_BgPanels] > 0.f) {
        float H = (maxY - minY) + 600.f;
        float midY = (minY + maxY) / 2.f;
        float step = 160.f / (0.4f + D);
        for (float x = minX - 60.f; x < maxX + 60.f; x += step * rf(c, 0.6f, 1.4f)) {
            if (!chance(c, std::min(1.f, 0.4f + W[F_BgPanels] * 0.6f))) continue;
            place(c, 1, x, midY, rf(c, 0.4f, 2.0f), H / C, 0.f, CH_PANEL, ZLayer::B4, -4);
        }
    }

    // ---- 9. color triggers (theme palette) ----
    if (o.addTriggers) {
        float tx = minX - 60.f, ty = maxY + 150.f;
        struct T { int ch; ccColor3B col; float op; };
        T list[] = {
            { 1000, pal.bg, 1.f },       // BG
            { 1001, pal.ground, 1.f },   // Ground
            { 1009, pal.ground, 1.f },   // Ground 2
            { 1002, pal.accent1, 1.f },  // Line
            { CH_A1, pal.accent1, 1.f },
            { CH_A2, pal.accent2, 1.f },
            { CH_A3, pal.accent3, 1.f },
            { CH_GLOW, pal.glow, 0.55f },
            { CH_PANEL, pal.accent3, 0.22f },
        };
        int i = 0;
        for (auto const& t : list) colorTrigger(c, tx, ty + 30.f * i++, t.ch, t.col, t.op);
    }

    g_last = c.made;
    res.created = (int)c.made.size();
    return res;
}

int removeLastDeco(LevelEditorLayer* lel) {
    if (!lel || !lel->m_objects) return 0;
    int n = 0;
    for (auto& obj : g_last) {
        if (obj && lel->m_objects->containsObject(obj)) {
            lel->removeObject(obj, true);
            n++;
        }
    }
    g_last.clear();
    return n;
}
