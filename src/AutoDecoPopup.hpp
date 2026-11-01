#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>

class AutoDecoPopup : public geode::Popup<EditorUI*> {
protected:
    EditorUI* m_ui = nullptr;
    int m_themeIdx = 0;
    float m_slider01 = 0.5f;
    bool m_selOnly = true;
    cocos2d::CCLabelBMFont* m_themeLabel = nullptr;
    cocos2d::CCLabelBMFont* m_densityLabel = nullptr;
    cocos2d::CCNode* m_swatches = nullptr;
    Slider* m_slider = nullptr;
    geode::TextInput* m_promptInput = nullptr;

    bool setup(EditorUI* ui) override;
    void refreshTheme();
    void refreshDensity();
    float densityMultiplier() const;

    void onPrev(cocos2d::CCObject*);
    void onNext(cocos2d::CCObject*);
    void onDensity(cocos2d::CCObject*);
    void onToggleSel(cocos2d::CCObject*);
    void onGenerate(cocos2d::CCObject*);
    void onRemoveLast(cocos2d::CCObject*);

public:
    static AutoDecoPopup* create(EditorUI* ui);
};
