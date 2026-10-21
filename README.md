# Breach: Industrial Sector

An original single-player C++17 tactical shooter prototype for 64-bit Windows. Uses a software raycaster, Windows GDI, raw mouse input and real-time synthesized PCM audio. All graphics and sounds are generated in code; no extra assets or installer are needed. This is a small prototype, not Counter-Strike 2 or a photorealistic game.

## Download and play

[Download Breach.zip](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip), extract the folder, and run `Breach.exe` on Windows.

Click the window to capture the mouse. The game pauses when the mouse is released or the window loses focus.

| Control | Action |
| --- | --- |
| W / A / S / D | Move / strafe |
| Mouse | Turn and look up / down |
| Left click or Space | Fire |
| Right click (hold) | Aim with zoom and slower movement |
| R | Reload; animated 1.8-second magazine change |
| Shift (hold) | Sprint |
| Left / Right arrow | Alternative turning |
| Escape | Release mouse; press again to quit |
| Enter | Restart after victory or defeat |

Eliminate five opponents before your health reaches zero. Enemies take two hits, pursue you when visible, and return fire. Walls block vision and bullets. You have 24 rounds per magazine with unlimited reserve ammunition.

## New immersion features

- Raw mouse look, aim zoom, normalized diagonal movement and sprinting.
- Textured masonry and paving, distance shading, edge vignette, detailed enemy silhouettes and fallen enemies.
- Rifle recoil, walking sway, muzzle flashes, player and enemy tracers, impact sparks, hit markers and damage feedback.
- Visible magazine removal / insertion and timed reload sounds; shooting is blocked until the reload finishes.
- Mixed procedural gunfire, enemy gunfire, footsteps, empty-magazine clicks, hit feedback and reload / bolt effects. If no audio device is available the game continues silently.

## Build

On Linux with MinGW-w64 installed:

```sh
./build.sh
```

For an extracted toolchain, set `CXX` to its `x86_64-w64-mingw32-g++-posix` executable. The build statically links compiler runtimes and uses standard Windows system DLLs including `winmm.dll`.

## Validate

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests /tmp/breach.ppm
```

Checks cover collision, hits, tracers, victory, damage, reload timing and fire blocking, wall occlusion, empty-fire behavior, angle wrapping, rendering and camera changes. The optional PPM output captures the software renderer. Windows mouse input, audio playback and window presentation require a Windows play test; they have not been runtime-tested in the Linux build environment.
