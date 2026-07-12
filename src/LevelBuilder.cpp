#include "LevelBuilder.hpp"

using namespace geode::prelude;

std::optional<std::string> LevelBuilder::extractJSONBlock(std::string const& text) {
    size_t searchFrom = 0;

    auto fence = text.find("```json");
    if (fence != std::string::npos) {
        searchFrom = fence;
    } else {
        auto plainFence = text.find("```");
        if (plainFence != std::string::npos) {
            searchFrom = plainFence;
        }
    }

    auto braceStart = text.find('{', searchFrom);
    if (braceStart == std::string::npos) {
        return std::nullopt;
    }

    auto braceEnd = text.rfind('}');
    if (braceEnd == std::string::npos || braceEnd < braceStart) {
        return std::nullopt;
    }

    auto candidate = text.substr(braceStart, braceEnd - braceStart + 1);

    // Cheap sanity check - a real build response should mention "objects"
    if (candidate.find("\"objects\"") == std::string::npos) {
        return std::nullopt;
    }

    return candidate;
}

int LevelBuilder::buildFromJSON(
    std::string const& jsonStr,
    LevelEditorLayer* editor,
    cocos2d::CCPoint originPos,
    int maxObjects
) {
    if (!editor) return 0;

    auto parsed = matjson::parse(jsonStr);
    if (!parsed) {
        log::warn("AI Architect: failed to parse build JSON: {}", parsed.unwrapErr());
        return 0;
    }
    auto root = parsed.unwrap();

    if (!root.contains("objects") || !root["objects"].isArray()) {
        return 0;
    }

    auto editorUI = editor->m_editorUI;
    if (!editorUI) return 0;

    int count = 0;
    for (auto& obj : root["objects"].asArray().unwrapOr(matjson::Array{})) {
        if (count >= maxObjects) {
            log::info("AI Architect: hit max-build-objects cap ({}), stopping", maxObjects);
            break;
        }

        int objID = static_cast<int>(obj["id"].asInt().unwrapOr(1));
        double x = obj["x"].asDouble().unwrapOr(0.0);
        double y = obj["y"].asDouble().unwrapOr(0.0);
        double rot = obj["rot"].asDouble().unwrapOr(0.0);
        double scale = obj["scale"].asDouble().unwrapOr(1.0);

        auto pos = originPos + cocos2d::CCPoint(static_cast<float>(x), static_cast<float>(y));

        // NOTE: EditorUI::createObject's exact signature can vary slightly
        // between Geode SDK / GD versions. This matches the commonly used
        // (objectID, position, snapToGrid) form. If this doesn't compile
        // against your installed SDK, check Geode's EditorUI bindings header
        // for the exact signature and adjust this one call.
        auto* gameObj = editorUI->createObject(objID, pos, false);
        if (!gameObj) continue;

        gameObj->setRotation(static_cast<float>(rot));
        gameObj->setScale(static_cast<float>(scale));

        count++;
    }

    return count;
}
