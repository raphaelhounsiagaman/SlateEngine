# SlateEngine

SlateEngine is a compact C++23 game engine being developed alongside
[Smells Like Rubber](https://github.com/raphaelhounsiagaman/SmellsLikeRubber).
It provides the application loop, window and input events, rendering, layer
lifecycle, and immediate game-facing UI needed by the current driving
prototype.

The public API is intentionally renderer-agnostic. The current Windows backend
uses Direct3D 11 for 3D rendering and Direct2D, DirectWrite, and WIC for UI and
images. Those native dependencies are owned by the backend and do not need to
appear in a game's build configuration.

## Current features

- Application and layer lifecycle
- Deferred layer transitions after update and render work completes
- Keyboard, mouse-button, pointer, wheel, focus, and resize events
- Update-based input queries for down, pressed, and released state
- Resizable Win32 window and D3D11 swap chain
- Mesh, material, camera, and transform-based 3D rendering
- Canvas UI with labels, buttons, images, text styles, and hit testing
- One update and one render pass per frame
- Renderer-owned canvas submission through `Renderer::DrawCanvas`
- Optional VSync with capability-driven DXGI tearing when disabled
- SI world-space convention: one world unit is one metre

## Requirements

- Windows 10 or newer
- Visual Studio with the Desktop development with C++ workload
- A Windows SDK containing Direct3D 11, Direct2D, DirectWrite, and WIC
- Git, including submodule support

Premake is included under `vendor/Premake`.

## Build

From the repository root, run:

```bat
scripts\Setup.bat
```

This generates the Visual Studio workspace. Build `SlateEngine` using the
`Debug`, `Release`, or `Dist` configuration. Generated projects and build
artifacts are intentionally ignored by Git.

## Integration

Add the engine as a submodule and include its Premake project:

```lua
include "vendor/SlateEngine/Slate/Build-SlateEngine.lua"
```

The game target needs only the engine dependency:

```lua
links
{
  "SlateEngine",
}
```

Application layers receive events, update in seconds, and render through the
engine:

```cpp
class GameLayer final : public Slate::ApplicationLayer
{
public:
    void OnEvent(Slate::Event& event) override;
    void OnUpdate(float deltaTimeSeconds) override;
    void OnRender() override;
};
```

Layer replacement is deferred safely:

```cpp
TransitionTo<GameLayer>();
```

Submit a complete UI canvas to the renderer:

```cpp
Slate::Renderer& renderer = Slate::Application::Get().GetRenderer();
renderer.DrawCanvas(canvas);
```

VSync can be enabled explicitly when desired:

```cpp
Slate::Application::Get().GetRenderer().SetVSyncEnabled(true);
```

Each application-loop iteration updates every layer once, renders every layer
once, and presents once. The update callback receives raw wall-clock delta
time, which a game can use to calculate FPS or drive its own timing systems.
SlateEngine does not maintain separate update/render rate limits or performance
counters.

On Windows, SlateEngine queries `DXGI_FEATURE_PRESENT_ALLOW_TEARING` before
creating the swap chain. Immediate presentation uses tearing only when the
system reports support and otherwise falls back without changing the game
configuration.

## Source layout

```text
Slate/
├── include/Slate/
│   ├── Core/       Application and layer lifecycle
│   ├── Graphics/   Renderer-facing types and resources
│   ├── Input/      Events, codes, and input state
│   ├── Math/       Vectors, transforms, matrices, and quaternions
│   └── UI/         Canvas and UI elements
└── src/
    ├── Core/       Windows application and window implementation
    ├── Graphics/   Renderer facade and D3D11 backend
    ├── Input/      Input-state implementation
    ├── Math/       Math implementation
    └── UI/         UI behavior
```

## Project status

SlateEngine is an early-stage engine under active development. Its APIs may
change as additional renderers and operating-system backends are introduced.
There is currently no standalone license file; all rights remain with the
repository owner unless a license is added.
