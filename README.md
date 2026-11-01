# Auto Decorator (Geode mod)

Automatically decorates a Geometry Dash layout. Choose a theme, type a prompt, or both.

## Use
1. Open the editor, select the part of the layout to decorate (or nothing = whole level).
2. Pause -> **Auto Deco** button (left side of the pause menu).
3. Pick a theme with the arrows, optionally write a prompt, set the density, press **Generate**.
4. **Remove Last** deletes the last batch.

Prompt examples: `dense purple neon with stars` / `forest with grass and pillars` / `minimal white` / `lava, no floaters, glow only`

## Themes
Neon, Space, Forest, Lava, Ice, Candy, Tech, Ocean, Minimal, Sunset

## How it works
- `DecoEngine` finds exposed block edges (tops, bottoms, sides) and adds trims, glow strips, grass blades, hangers, pillars, floating particles, sparkles, rain streaks, and background panels. All of it is no-touch, so gameplay is not affected.
- Deco uses color channels 201-205; color triggers set them (plus BG/Ground/Line) to the theme palette.
- `PromptParser` is an offline keyword parser (theme words, feature words, negation like "no X", "only X", density words, color words, dark/bright).

## Build
1. Install the Geode CLI and SDK: https://docs.geode-sdk.org/getting-started/
2. Set up the project:
   ```
   geode sdk install
   geode build
   ```
   (or push to GitHub: the included workflow builds Windows/macOS/Android and gives you a `.geode` file)
3. Edit `mod.json` -> change `id` (`yourname.auto-decorator`) and `developer`.

Targets Geode 4.x / GD 2.2074. On Geode 5+ the `Popup` class API changed (non-template, `init(...)` instead of `setup(...)`), so `AutoDecoPopup` would need a small port.
