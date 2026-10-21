# Breach

An original, small single-player tactical shooter written in C++17 using Windows GDI. This is a prototype, not Counter-Strike 2: it has no Valve assets, networking, bomb mode, or commercial-game feature parity.

## Play

Run `dist/Breach.exe` on 64-bit Windows. No installer or separate game assets are required.

- W / S: forward / backward
- A / D: strafe
- Left / Right arrows: aim / turn
- Left click or Space: fire
- R: reload (unlimited reserve magazines)
- Enter: restart after winning or losing
- Escape: quit

Eliminate all five opponents before your health reaches zero. Each opponent takes two hits. Walls block bullets and vision. The minimap shows your position in green.

## Build

On Linux with MinGW-w64 installed:

```sh
./build.sh
```

With a locally extracted toolchain, set `CXX` to its `x86_64-w64-mingw32-g++-posix` binary. On Windows, use MinGW-w64 with the flags in build.sh.

## Logic checks

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests
```

These exercise movement collision, hits, victory, reset, damage, reload, and wall occlusion. They do not validate Windows rendering or input. The executable must be played on Windows for those checks.
