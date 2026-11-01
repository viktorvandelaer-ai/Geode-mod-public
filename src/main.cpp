#include <Geode/Geode.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>
#include "AutoDecoPopup.hpp"

using namespace geode::prelude;

class $modify(AutoDecoPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* lel) {
        if (!EditorPauseLayer::init(lel)) return false;

        auto winSize = CCDirector::get()->getWinSize();
        auto spr = ButtonSprite::create("Auto Deco", 100, true, "bigFont.fnt", "GJ_button_01.png", 30.f, 0.6f);
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(AutoDecoPauseLayer::onAutoDeco));
        btn->setID("auto-deco-button"_spr);

        auto menu = CCMenu::create();
        menu->setID("auto-deco-menu"_spr);
        menu->setPosition({ 0.f, 0.f });
        btn->setPosition({ 70.f, winSize.height / 2.f });
        menu->addChild(btn);
        this->addChild(menu);
        return true;
    }

    void onAutoDeco(CCObject*) {
        auto ui = m_editorLayer ? m_editorLayer->m_editorUI : nullptr;
        if (!ui) return;
        AutoDecoPopup::create(ui)->show();
    }
};
