#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "AIPopup.hpp"

using namespace geode::prelude;

class $modify(AIArchitectEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;

        auto spr = ButtonSprite::create("AI", "bigFont.fnt", "GJ_button_04.png", 0.8f);
        spr->setScale(0.7f);

        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(AIArchitectEditorUI::onOpenAI)
        );
        btn->setID("ai-architect-button"_spr);

        // The pause/settings button menu in the editor is a reliable place
        // to dock a custom button. If your Geode SDK version names this
        // menu differently, use Cocos node explorer (bind Geode's built-in
        // node ID debug tool) to find the right getChildByID string.
        if (auto pauseMenu = this->getChildByID("pause-button-menu")) {
            pauseMenu->addChild(btn);
            pauseMenu->updateLayout();
        } else {
            log::warn("AI Architect: couldn't find pause-button-menu to dock the AI button");
        }

        return true;
    }

    void onOpenAI(CCObject*) {
        AIPopup::create(this->m_editorLayer)->show();
    }
};
