# Breach: Dockyard 3D

An original, offline C++17 tactical shooter prototype for 64-bit Windows. Dockyard now uses a **triangle-based software 3D renderer** with perspective projection, near-plane clipping, and a per-pixel depth buffer. Buildings, cover, characters, grenades, and the planted bomb have 3D geometry. The first-person weapon and interface use screen-space art. No external assets or installer are needed.

[Download the game folder](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip). Extract the ZIP and run **Breach.exe**. Click the window to capture the mouse. Escape releases it and pauses; Escape again quits.

## Round objectives

You fight five AI opponents on a 32×32 Dockyard map, four times the previous map's ground area. It includes two marked bomb courtyards, flanking routes, a middle connector, buildings, overhead structures, and crates.

- **Attack:** Eliminate all defenders, or hold **F** in the yellow A or B bomb-site area for three seconds to plant. Defend the bomb for its 40-second countdown. Enemies can defuse it if you let them reach it for five seconds.
- **Defend:** Stop the attackers before they plant, or reach the planted device and hold **F** within 1.8 units for five seconds to defuse. Moving, firing, switching guns, throwing a grenade, or releasing F interrupts your own objective interaction. (F blocks shooting while held.)
- The round lasts 115 seconds before a plant. After planting, the bomb has its own timer. Round timeout favors the defender. Death ends your round.
- Press **Enter** after a result to begin the next round. Your role alternates each round. Wins and losses accumulate; there is no fixed match length.

## Controls

| Key | Action |
| --- | --- |
| WASD | Move / strafe |
| Mouse | Turn and look up / down |
| Left click / Space | Fire |
| Right click (hold) | Aim / zoom; marksman has a scope overlay |
| Shift | Sprint |
| R | Reload from your weapon's finite reserve |
| 1 / 2 / 3 / 4 | Pistol / carbine / SMG / marksman, if owned |
| F (hold) | Plant / defuse when in range |
| Q | Cycle frag / smoke / flash |
| G | Throw selected utility |
| B | Open / close buy menu in spawn during first 22 seconds |
| Tab | Show personal kills and match score |
| Left / Right arrows | Alternative turning |
| Escape | Release mouse / pause; again to quit |
| Enter | Start next round after a result |

## Weapons and economy

Start with a pistol, $3200, and one of each grenade. A kill awards $300, a round win $3000, and a loss $1900, capped at $16000. Winning preserves purchased guns and armor; losing removes those guns and armor. Magazines, reserves and one of each utility refresh next round.

While the buy menu is open, its numbered choices replace weapon-selection keys:

| Buy key | Item | Price | Magazine | Body damage |
| --- | --- | --- | --- | --- |
| 1 | Carbine | $2400 | 30 | 35 |
| 2 | Compact SMG | $1200 | 32 | 24 |
| 3 | Marksman | $3500 | 5 | 100 |
| 4 | Armor | $650 | — | Reduces incoming gun damage |
| 5 | Utility refill | $300 | — | Adds one of each, maximum two |

Your free pistol holds 12 rounds and deals 34 body damage. Weapons have different fire rates, ranges, spreads and reload times. Headshots deal triple damage; moving and sustained fire widen spread, and aiming narrows it. Every gun keeps its own ammo. Switching cancels a reload. Buying does not pause combat.

## AI and utilities

Bots use grid pathfinding around walls and crates, share nearby sightings, investigate audible gunfire, strafe in combat, and reload their magazines. Defenders respond to planted bombs; attackers can navigate to either site and plant when they are not engaging you. Smoke interrupts their sight, and flash grenades temporarily blind them. This is a single-player prototype with probabilistic AI accuracy, not competitive multiplayer AI.

- **Frag:** Bouncing projectile; explodes after 1.25 seconds, dealing damage in line of sight. Can hurt you.
- **Smoke:** Creates a 12-second smoke cloud that blocks AI visibility. Bullets still pass through it.
- **Flash:** Blinds nearby bots in line of sight for four seconds. Can white out your screen if you face the blast.

Procedural gunfire, footsteps, reload clicks, grenade blasts, bomb beeps, recoil, muzzle flashes, tracers, damage feedback and weapon sway are retained. No audio device means silent play.

## Build and validation

```sh
# Windows cross-build with MinGW-w64
./build.sh
# For an extracted toolchain, set CXX to its x86_64-w64-mingw32-g++-posix.

# Portable gameplay and software-renderer tests
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests /tmp/breach.ppm

# Optional memory / undefined-behavior checks
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp -o /tmp/breach-sanitized
/tmp/breach-sanitized
```

Tests exercise map connectivity, spawn collision, economy, independent weapons, head/body hits, cover, finite reloads, planting, interruptions, bot planting/defusing, bomb and round timers, result accounting, all three utilities, AI navigation/hearing, triangle rendering, depth occlusion, and camera projection. The optional PPM is a current rendered scene.

Windows mouse capture, input, audio playback, and window presentation still require a Windows play test; these were cross-compiled but not runtime-tested in this Linux environment. The art is deliberately simple low-poly geometry. There is no networking, licensed Counter-Strike content, imported character animation, or photorealistic rendering.
