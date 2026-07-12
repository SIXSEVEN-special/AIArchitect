#pragma once
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <matjson.hpp>
#include <functional>
#include <vector>
#include <string>

// Thin wrapper around the Groq chat completions endpoint
// (OpenAI-compatible API: https://api.groq.com/openai/v1/chat/completions)
class GroqClient {
public:
    using ResponseCallback = std::function<void(std::string const& reply, bool success)>;

    // messageHistory is a list of {"role": "...", "content": "..."} matjson objects,
    // oldest first. The caller is responsible for keeping this trimmed to a
    // reasonable length (context window) - see AIPopup for the trimming logic.
    static void sendChat(std::vector<matjson::Value> const& messageHistory, ResponseCallback callback);

    // The system prompt that instructs the model on the build-JSON protocol.
    static std::string buildSystemPrompt();
};
