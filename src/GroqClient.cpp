#include "GroqClient.hpp"

using namespace geode::prelude;

std::string GroqClient::buildSystemPrompt() {
    // This is the whole trick: we tell the model to answer normal questions
    // in plain text, but to answer BUILD requests with ONLY a fenced json
    // block describing objects. LevelBuilder looks for that block and, if
    // found, spawns the objects instead of printing the reply as chat text.
    return
        "You are an AI assistant embedded inside a Geometry Dash level editor mod called AI Architect. "
        "You can have normal conversations. "
        "However, when the user asks you to BUILD, GENERATE, or CREATE a layout, decoration, "
        "structure, or object arrangement (for example 'build a heliopolis style layout' or "
        "'make a symmetrical spike pattern'), you must respond with ONLY a single fenced code "
        "block of JSON, and nothing else - no explanation, no greeting. The format is:\n\n"
        "```json\n"
        "{\"objects\":[{\"id\":1,\"x\":0,\"y\":15,\"rot\":0,\"scale\":1}, ...]}\n"
        "```\n\n"
        "Rules for the JSON:\n"
        "- \"id\" is the Geometry Dash object ID (1 = basic block, 8 = spike, 103 = mini spike, "
        "  103/144/etc for blocks, use common GD object IDs you know).\n"
        "- \"x\" and \"y\" are OFFSETS in game units (30 units = 1 grid square) relative to the "
        "  player's current position in the editor, where x+ is right and y+ is up.\n"
        "- \"rot\" is rotation in degrees (0-360), optional, default 0.\n"
        "- \"scale\" is a float, optional, default 1.\n"
        "- Keep the object count reasonable (under 150) unless asked for something huge.\n"
        "- Do not wrap the JSON in any other text when building.\n\n"
        "For everything that is NOT a build request, just answer normally in plain text, and never "
        "output a json code block unless you are actually building something.";
}

void GroqClient::sendChat(std::vector<matjson::Value> const& messageHistory, ResponseCallback callback) {
    auto apiKey = Mod::get()->getSettingValue<std::string>("groq-api-key");
    auto model = Mod::get()->getSettingValue<std::string>("groq-model");

    if (apiKey.empty()) {
        callback("No Groq API key set. Open this mod's settings and paste your key from console.groq.com.", false);
        return;
    }

    matjson::Value body = matjson::Value::object();
    body["model"] = model.empty() ? "llama-3.3-70b-versatile" : model;
    body["temperature"] = 0.4;

    matjson::Value msgs = matjson::Value::array();

    // System prompt always goes first
    matjson::Value sys = matjson::Value::object();
    sys["role"] = "system";
    sys["content"] = GroqClient::buildSystemPrompt();
    msgs.push(sys);

    for (auto& m : messageHistory) {
        msgs.push(m);
    }
    body["messages"] = msgs;

    auto req = web::WebRequest();
    req.header("Authorization", "Bearer " + apiKey);
    req.header("Content-Type", "application/json");
    req.bodyJSON(body);

    req.post("https://api.groq.com/openai/v1/chat/completions").listen(
        [callback](web::WebResponse* res) {
            if (!res || !res->ok()) {
                callback(
                    fmt::format("Groq request failed (HTTP {}). Check your API key and internet connection.",
                        res ? res->code() : -1),
                    false
                );
                return;
            }

            auto jsonResult = res->json();
            if (!jsonResult) {
                callback("Groq returned a response that wasn't valid JSON.", false);
                return;
            }
            auto json = jsonResult.unwrap();

            auto content = json["choices"][0]["message"]["content"].asString();
            if (!content || content.unwrap().empty()) {
                callback("Groq returned an empty response.", false);
                return;
            }

            callback(content.unwrap(), true);
        }
    );
}
