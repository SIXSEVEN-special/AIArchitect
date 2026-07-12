#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/ScrollLayer.hpp>
#include <matjson.hpp>
#include <vector>
#include <string>

class AIPopup : public geode::Popup<LevelEditorLayer*> {
protected:
    LevelEditorLayer* m_editor = nullptr;
    std::vector<matjson::Value> m_history;   // conversation sent to Groq
    std::string m_chatLog;                    // what's shown on screen

    cocos2d::CCLabelBMFont* m_chatLabel = nullptr;
    geode::ScrollLayer* m_scroll = nullptr;
    geode::TextInput* m_input = nullptr;
    cocos2d::CCMenuItemSpriteExtra* m_sendBtn = nullptr;
    cocos2d::CCLabelBMFont* m_statusLabel = nullptr;

    bool setup(LevelEditorLayer* editor) override;

    void onSend(cocos2d::CCObject*);
    void sendMessage(std::string const& text);
    void handleReply(std::string const& reply, bool success);

    void appendLine(std::string const& who, std::string const& text);
    void refreshChatLabel();
    void setBusy(bool busy);

public:
    static AIPopup* create(LevelEditorLayer* editor);
};
