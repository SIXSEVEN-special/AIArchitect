# AI Architect (Geode mod)

Chat with a Groq-hosted AI from inside the Geometry Dash level editor, and let it
place objects for you when you ask it to build something.

## How "building" actually works

There's no special GD API for "generate a layout from English." Instead:

1. Every request to Groq includes a system prompt (see `GroqClient::buildSystemPrompt`)
   that tells the model: normal questions get normal text answers, but build
   requests ("build a heliopolis layout", "make a spike tunnel") must be answered
   with **only** a JSON block like:
   ```json
   { "objects": [ { "id": 1, "x": 0, "y": 15, "rot": 0, "scale": 1 }, ... ] }
   ```
2. `LevelBuilder::extractJSONBlock` looks for that JSON in the reply.
3. If found, `LevelBuilder::buildFromJSON` spawns each object via
   `EditorUI::createObject`, offset from your player's current position.
4. If not found, the reply is just shown as a normal chat message.

Conversation history (last ~20 turns) is kept in memory per popup session, so you
can say things like "make it wider" and it has context on what it just built.

## Setup

1. Get a free API key from https://console.groq.com
2. Build/install the mod (see below), then in-game go to
   **Geode Settings -> AI Architect** and paste your key into `Groq API Key`.
3. Open the level editor, click the new **AI** button (docked near the pause
   button menu), and start chatting.

## Known rough edges (read before you build)

- `EditorUI::createObject` and the editor button docking (`pause-button-menu`)
  are written against commonly-seen Geode binding names. If your installed
  Geode SDK version has renamed these, the build will fail with a clear
  "no member named ..." error - just swap in the correct name from your SDK's
  headers (or use Geode's node ID debug overlay to find the right menu to dock
  the button to).
- The AI's object IDs/coordinates are only as good as the model - expect to
  iterate on the system prompt in `GroqClient.cpp` (e.g. give it a short list
  of known-good object IDs) once you see what it tends to generate.
- `max-build-objects` (mod setting, default 150) is a hard safety cap so a
  wild response can't dump thousands of objects into your level.

## Building locally

Requires the Geode CLI and SDK installed (see https://docs.geode-sdk.org).

```powershell
geode build
```

## Pushing to GitHub so Actions builds it for you

From inside the `AIArchitect` folder:

```powershell
cd C:\path\to\AIArchitect
git init
git remote add origin https://github.com/yourname/AIArchitect.git
git add .
git add .github
git commit -m "Initial commit: AI Architect mod scaffold"
git branch -M main
git push -u origin main
```

Note the extra `git add .github` - on Windows, `.github` is a hidden folder,
and if you ever copy this project via zip/extract instead of git, some zip
tools silently drop dotfolders. `git add .` alone can also occasionally skip
it depending on your `.gitignore`/global config, so adding it explicitly is
the safe habit (same issue you hit on the Warmup mod's CI).

Once pushed, go to your repo's **Actions** tab - the `Build` workflow runs
automatically and uploads the compiled `.geode` file as a downloadable
artifact.
