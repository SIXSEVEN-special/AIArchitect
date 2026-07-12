#include "AIPopup.hpp"
#include "GroqClient.hpp"
#include "LevelBuilder.hpp"

using namespace geode::prelude;

AIPopup* AIPopup::create(LevelEditorLayer* editor) {
    auto ret = new AIPopup();
    if (ret->initAnchored(420.f, 280.f, editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool AIPopup::setup(LevelEditorLayer* editor) {
    m_editor = editor;
    this->setTitle("AI Architect");

    auto winSize = m_mainLayer->getContentSize();

    // --- Chat log area (scrollable) ---
    auto scrollBG = CCScale9Sprite::create("square02b_small.png");
    scrollBG->setContentSize({ winSize.width - 40.f, 160.f });
    scrollBG->setColor({ 0, 0, 0 });
    scrollBG->setOpacity(90);
    scrollBG->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 30.f });
    m_mainLayer->addChild(scrollBG);

    m_scroll = ScrollLayer::create({ winSize.width - 56.f, 148.f });
    m_scroll->setPosition({ 28.f, winSize.height / 2.f + 30.f - 74.f });
    m_mainLayer->addChild(m_scroll);

    m_chatLabel = CCLabelBMFont::create("Type a message below to chat, or ask me to build something!",
        "chatFont.fnt");
    m_chatLabel->setAnchorPoint({ 0.f, 1.f });
    m_chatLabel->setPosition({ 4.f, m_scroll->getContentSize().height - 4.f });
    m_chatLabel->setScale(0.45f);
    m_scroll->m_contentLayer->addChild(m_chatLabel);
    m_scroll->m_contentLayer->setContentSize(
        { m_scroll->getContentSize().width, m_scroll->getContentSize().height }
    );

    // --- Status label (busy / error text) ---
    m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
    m_statusLabel->setScale(0.4f);
    m_statusLabel->setPosition({ winSize.width / 2.f, winSize.height / 2.f - 58.f });
    m_statusLabel->setColor({ 255, 220, 120 });
    m_mainLayer->addChild(m_statusLabel);

    // --- Input field ---
    m_input = TextInput::create(winSize.width - 90.f, "Ask a question, or say 'build a spike tunnel'...");
    m_input->setPosition({ winSize.width / 2.f - 20.f, winSize.height / 2.f - 90.f });
    m_input->setCallback([this](std::string const& text) {
        this->sendMessage(text);
    });
    m_mainLayer->addChild(m_input);

    // --- Send button ---
    auto sendSpr = ButtonSprite::create("Send");
    sendSpr->setScale(0.7f);
    m_sendBtn = CCMenuItemSpriteExtra::create(sendSpr, this, menu_selector(AIPopup::onSend));

    auto menu = CCMenu::create();
    menu->addChild(m_sendBtn);
    menu->setPosition({ winSize.width - 32.f, winSize.height / 2.f - 90.f });
    m_mainLayer->addChild(menu);

    return true;
}

void AIPopup::onSend(CCObject*) {
    if (!m_input) return;
    this->sendMessage(m_input->getString());
}

void AIPopup::sendMessage(std::string const& text) {
    auto trimmed = text;
    // strip leading/trailing whitespace
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.front()))) trimmed.erase(trimmed.begin());
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) trimmed.pop_back();
    if (trimmed.empty()) return;

    appendLine("You", trimmed);
    m_input->setString("");

    matjson::Value userMsg = matjson::Value::object();
    userMsg["role"] = "user";
    userMsg["content"] = trimmed;
    m_history.push_back(userMsg);

    // Keep history from growing forever - last 20 turns is plenty of context
    if (m_history.size() > 20) {
        m_history.erase(m_history.begin(), m_history.begin() + (m_history.size() - 20));
    }

    setBusy(true);

    GroqClient::sendChat(m_history, [this](std::string const& reply, bool success) {
        this->handleReply(reply, success);
    });
}

void AIPopup::handleReply(std::string const& reply, bool success) {
    setBusy(false);

    if (!success) {
        m_statusLabel->setString(reply.c_str());
        return;
    }

    matjson::Value assistantMsg = matjson::Value::object();
    assistantMsg["role"] = "assistant";
    assistantMsg["content"] = reply;
    m_history.push_back(assistantMsg);

    auto maybeJSON = LevelBuilder::extractJSONBlock(reply);
    if (maybeJSON) {
        auto maxObjects = Mod::get()->getSettingValue<int64_t>("max-build-objects");

        // Anchor the build at wherever the player object currently sits
        // in the editor - feels the most intuitive as a "build here" point.
        cocos2d::CCPoint origin = { 0.f, 105.f };
        if (m_editor && m_editor->m_player1) {
            origin = m_editor->m_player1->getPosition();
        }

        int placed = LevelBuilder::buildFromJSON(*maybeJSON, m_editor, origin, static_cast<int>(maxObjects));

        if (placed > 0) {
            appendLine("AI", fmt::format("Built {} object(s) in the editor near your player.", placed));
        } else {
            appendLine("AI", "I tried to build something but couldn't place any objects - check the Geode console log for details.");
        }
        return;
    }

    appendLine("AI", reply);
}

void AIPopup::appendLine(std::string const& who, std::string const& text) {
    if (!m_chatLog.empty()) m_chatLog += "\n\n";
    m_chatLog += who + ": " + text;
    refreshChatLabel();
}

void AIPopup::refreshChatLabel() {
    if (!m_chatLabel) return;
    m_chatLabel->setString(m_chatLog.c_str());

    // Re-wrap roughly to the scroll width (BM fonts don't auto word-wrap,
    // so this is intentionally simple - good enough for short chat lines).
    m_chatLabel->setAnchorPoint({ 0.f, 1.f });
    m_chatLabel->setPosition({ 4.f, m_chatLabel->getContentSize().height + 4.f });

    if (m_scroll) {
        m_scroll->m_contentLayer->setContentSize({
            m_scroll->getContentSize().width,
            std::max(m_chatLabel->getContentSize().height + 20.f, m_scroll->getContentSize().height)
        });
        m_scroll->scrollToTop();
    }
}

void AIPopup::setBusy(bool busy) {
    if (m_sendBtn) m_sendBtn->setEnabled(!busy);
    if (m_input) m_input->setEnabled(!busy);
    m_statusLabel->setString(busy ? "Thinking..." : "");
}
