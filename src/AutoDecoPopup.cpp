#include "AutoDecoPopup.hpp"
#include "DecoEngine.hpp"
#include "PromptParser.hpp"
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

AutoDecoPopup* AutoDecoPopup::create(EditorUI* ui) {
    auto ret = new AutoDecoPopup();
    if (ret->initAnchored(380.f, 270.f, ui)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

float AutoDecoPopup::densityMultiplier() const {
    return 0.15f + 1.35f * m_slider01;   // slider 0..1  ->  0.15x .. 1.5x
}

bool AutoDecoPopup::setup(EditorUI* ui) {
    m_ui = ui;
    this->setTitle("Auto Decorator");

    auto mod = Mod::get();
    auto const& themes = allThemes();
    std::string lastTheme = mod->getSavedValue<std::string>("last-theme", mod->getSettingValue<std::string>("default-theme"));
    m_themeIdx = findThemeIndex(lastTheme);
    m_slider01 = (float)std::clamp(mod->getSettingValue<double>("density"), 0.0, 1.0);

    auto sel = ui ? ui->getSelectedObjects() : nullptr;
    m_selOnly = sel && sel->count() > 0;

    // --- theme row ---
    auto themeTitle = CCLabelBMFont::create("Theme", "bigFont.fnt");
    themeTitle->setScale(0.45f);
    m_mainLayer->addChildAtPosition(themeTitle, Anchor::Top, { 0.f, -45.f });

    auto lSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    lSpr->setScale(0.7f);
    auto lBtn = CCMenuItemSpriteExtra::create(lSpr, this, menu_selector(AutoDecoPopup::onPrev));
    m_buttonMenu->addChildAtPosition(lBtn, Anchor::Top, { -110.f, -72.f });

    auto rSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
    rSpr->setScale(0.7f);
    rSpr->setFlipX(true);
    auto rBtn = CCMenuItemSpriteExtra::create(rSpr, this, menu_selector(AutoDecoPopup::onNext));
    m_buttonMenu->addChildAtPosition(rBtn, Anchor::Top, { 110.f, -72.f });

    m_themeLabel = CCLabelBMFont::create(themes[m_themeIdx].name.c_str(), "goldFont.fnt");
    m_themeLabel->setScale(0.8f);
    m_mainLayer->addChildAtPosition(m_themeLabel, Anchor::Top, { 0.f, -72.f });

    m_swatches = CCNode::create();
    m_mainLayer->addChildAtPosition(m_swatches, Anchor::Top, { 0.f, -98.f });

    // --- prompt ---
    auto promptTitle = CCLabelBMFont::create("Prompt (optional)", "bigFont.fnt");
    promptTitle->setScale(0.45f);
    m_mainLayer->addChildAtPosition(promptTitle, Anchor::Center, { 0.f, 22.f });

    m_promptInput = TextInput::create(320.f, "e.g. dense purple neon with stars", "chatFont.fnt");
    m_promptInput->setMaxCharCount(140);
    m_promptInput->setString(mod->getSavedValue<std::string>("last-prompt", ""));
    m_mainLayer->addChildAtPosition(m_promptInput, Anchor::Center, { 0.f, -4.f });

    // --- density ---
    m_densityLabel = CCLabelBMFont::create("Density", "bigFont.fnt");
    m_densityLabel->setScale(0.45f);
    m_mainLayer->addChildAtPosition(m_densityLabel, Anchor::Center, { 0.f, -38.f });

    m_slider = Slider::create(this, menu_selector(AutoDecoPopup::onDensity), 0.8f);
    m_slider->setValue(m_slider01);
    m_mainLayer->addChildAtPosition(m_slider, Anchor::Center, { 0.f, -58.f });

    // --- selection toggle ---
    auto toggler = CCMenuItemToggler::createWithStandardSprites(this, menu_selector(AutoDecoPopup::onToggleSel), 0.6f);
    toggler->toggle(m_selOnly);
    m_buttonMenu->addChildAtPosition(toggler, Anchor::BottomLeft, { 30.f, 70.f });
    auto selLabel = CCLabelBMFont::create("Selection only", "bigFont.fnt");
    selLabel->setScale(0.4f);
    selLabel->setAnchorPoint({ 0.f, 0.5f });
    m_mainLayer->addChildAtPosition(selLabel, Anchor::BottomLeft, { 48.f, 70.f });

    // --- buttons ---
    auto genBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Generate"), this, menu_selector(AutoDecoPopup::onGenerate));
    m_buttonMenu->addChildAtPosition(genBtn, Anchor::Bottom, { 55.f, 28.f });
    auto remBtn = CCMenuItemSpriteExtra::create(ButtonSprite::create("Remove Last", 90, true, "bigFont.fnt", "GJ_button_06.png", 30.f, 0.6f), this, menu_selector(AutoDecoPopup::onRemoveLast));
    m_buttonMenu->addChildAtPosition(remBtn, Anchor::Bottom, { -75.f, 28.f });

    refreshTheme();
    refreshDensity();
    return true;
}

void AutoDecoPopup::refreshTheme() {
    auto const& t = allThemes()[m_themeIdx];
    m_themeLabel->setString(t.name.c_str());
    m_swatches->removeAllChildren();
    ccColor3B cols[] = { t.pal.bg, t.pal.accent1, t.pal.accent2, t.pal.accent3, t.pal.glow };
    float x = -2 * 24.f;
    for (auto col : cols) {
        auto sw = CCLayerColor::create(ccc4(col.r, col.g, col.b, 255), 20.f, 14.f);
        sw->setPosition({ x - 10.f, -7.f });
        m_swatches->addChild(sw);
        x += 24.f;
    }
}

void AutoDecoPopup::refreshDensity() {
    m_densityLabel->setString(fmt::format("Density  x{:.2f}", densityMultiplier()).c_str());
}

void AutoDecoPopup::onPrev(CCObject*) {
    int n = (int)allThemes().size();
    m_themeIdx = (m_themeIdx + n - 1) % n;
    refreshTheme();
}

void AutoDecoPopup::onNext(CCObject*) {
    m_themeIdx = (m_themeIdx + 1) % (int)allThemes().size();
    refreshTheme();
}

void AutoDecoPopup::onDensity(CCObject*) {
    m_slider01 = m_slider->getThumb()->getValue();
    refreshDensity();
}

void AutoDecoPopup::onToggleSel(CCObject* sender) {
    m_selOnly = !static_cast<CCMenuItemToggler*>(sender)->isToggled();
}

void AutoDecoPopup::onGenerate(CCObject*) {
    auto mod = Mod::get();
    DecoOptions opts;
    opts.theme = allThemes()[m_themeIdx];
    opts.density = densityMultiplier();
    opts.selectionOnly = m_selOnly;
    opts.seed = (unsigned)mod->getSettingValue<int64_t>("seed");
    opts.maxObjects = (int)mod->getSettingValue<int64_t>("max-objects");
    opts.addTriggers = mod->getSettingValue<bool>("color-triggers");
    opts.bgPanels = mod->getSettingValue<bool>("background-panels");

    std::string prompt = m_promptInput->getString();
    std::string understood = applyPrompt(prompt, opts.theme, opts.density);

    mod->setSavedValue<std::string>("last-theme", allThemes()[m_themeIdx].name);
    mod->setSavedValue<std::string>("last-prompt", prompt);

    auto res = decorateLayout(m_ui ? m_ui->m_editorLayer : nullptr, m_ui, opts);
    if (!res.error.empty()) {
        Notification::create(res.error, NotificationIcon::Error)->show();
        return;
    }

    std::string msg = fmt::format("Added {} deco objects ({})", res.created,
                                  opts.theme.name);
    if (!understood.empty()) msg += " - " + understood;
    Notification::create(msg, NotificationIcon::Success, 4.f)->show();
    this->onClose(nullptr);
}

void AutoDecoPopup::onRemoveLast(CCObject*) {
    int n = removeLastDeco(m_ui ? m_ui->m_editorLayer : nullptr);
    Notification::create(n > 0 ? fmt::format("Removed {} objects", n) : std::string("Nothing to remove"),
                         n > 0 ? NotificationIcon::Success : NotificationIcon::Warning)->show();
}
