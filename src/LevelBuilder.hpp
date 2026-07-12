#pragma once
#include <Geode/Geode.hpp>
#include <matjson.hpp>
#include <optional>
#include <string>

// Turns an AI reply into real objects placed in the currently open editor.
class LevelBuilder {
public:
    // Pulls the ```json {...} ``` (or bare {...}) block out of a chat reply.
    // Returns std::nullopt if the reply doesn't look like a build response,
    // in which case AIPopup should just print it as normal chat text.
    static std::optional<std::string> extractJSONBlock(std::string const& text);

    // Parses the JSON and spawns each object into `editor`, offset from
    // `originPos` (typically the player's current position when the request
    // was sent). Returns how many objects were actually placed.
    static int buildFromJSON(
        std::string const& jsonStr,
        LevelEditorLayer* editor,
        cocos2d::CCPoint originPos,
        int maxObjects
    );
};
