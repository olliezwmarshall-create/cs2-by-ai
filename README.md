# Breach: Dockyard — Team Assault

An original offline C++17 tactical shooter for 64-bit Windows. Buildings, cover, characters, projectiles, **first-person weapons, magazines, hands and scopes are 3D meshes**, rendered with perspective projection, near clipping and depth testing. All geometry and sound are generated in code; no installer or extra assets are required.

[Download Breach.zip](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip), extract the folder and run **Breach.exe**. Click to capture the mouse. Escape releases it and pauses; Escape again quits.

## Teams and round rules

**T and CT each have five AI players. You join your current side as an additional player.** This means six versus five while you are alive, rather than replacing one of the five bots. Your side alternates between rounds.

- **T:** Eliminate every CT for an immediate win, **whether or not the bomb is planted**, or plant and defend the bomb until it detonates.
- **CT:** Eliminate every T before a plant, survive the pre-plant timer, or defuse the planted bomb. If every T dies after a plant, CT must still defuse or survive no further round timer: only the bomb countdown matters.
- **Plant:** Hold F for three seconds inside a yellow A or B site. **Defuse:** Hold F for five seconds within 1.8 units of the planted device. Moving, changing weapons, reloading, throwing utility or releasing F interrupts the interaction.
- **Timers:** The round clock starts at 115 seconds and **freezes at planting**. The bomb then has its own 40-second countdown. The HUD marks the round clock `FROZEN` and displays the bomb countdown separately.
- **After death:** Your AI teammates keep fighting. You spectate a living teammate; V cycles teammates. The round resolves using team elimination and bomb rules rather than your death alone.
- Enter starts the next round after a result. Wins and losses accumulate without a fixed match length.

The 32×32 map has two bomb courtyards, middle connectors, flanking routes, buildings, overhead structures and crates. Friendly bots have green overhead markers and blue minimap dots. Friendly bullet and grenade damage is disabled; teammates still block player bullets.

## Controls

| Input | Action |
| --- | --- |
| WASD | Move / strafe |
| Mouse | Turn / look up and down |
| Left click / Space | Fire; hold for automatic weapons |
| Right click (hold) | Aim; scoped weapons zoom further |
| Shift | Sprint |
| R | Reload from finite reserve ammunition |
| 1 / 2 / 3 / 4 / 5 | Pistol / carbine / SMG / bolt sniper / scout DMR, if owned |
| F (hold) | Plant / defuse in range; blocks firing while held |
| Q / G | Cycle utility / throw utility |
| B | Buy menu in spawn, first 22 seconds |
| Tab | Team roster, AI state, kills and your status |
| V | Cycle living teammates while spectating |
| Arrow Left / Right | Alternative turning |
| Escape | Release mouse / pause; again to quit |
| Enter | Next round after a result |

## Weapons and buying

The carbine and SMG fire continuously while held, using a fixed 240 Hz gameplay step. Cadence does not depend on rendering speed. The pistol, bolt sniper and DMR require a fresh trigger press per shot.

| Weapon | Buy key* | Price | Magazine | Body damage | Nominal firing interval |
| --- | --- | --- | --- | --- | --- |
| Service pistol | Free | $0 | 12 | 34 | 0.22 s, semi-auto |
| Auto carbine | 1 | $2400 | 30 | 35 | 0.065 s, automatic (~923 RPM) |
| Rapid SMG | 2 | $1200 | 32 | 24 | 0.05 s, automatic (1200 RPM) |
| Bolt sniper | 3 | $3500 | 5 | 110 | 1.2 s, bolt-action |
| Scout DMR | 6 | $2800 | 10 | 72 | 0.28 s, semi-auto |

*Buy keys apply only with the B menu open; they differ from weapon-selection keys. In the menu, 4 buys armor ($650), and 5 refills utility ($300, maximum two of each).

Headshots deal triple damage. Moving and sustained fire widen spread; aiming narrows it. Each weapon keeps separate magazine and reserve counts. The sniper has a visible 3D bolt cycle and bolt sound; switching and reloading are blocked during its bolt cycle. Reloads move the 3D magazine and tilt the entire weapon. Switching normally cancels a reload.

You start with a pistol, $3200, and one of each grenade. Your kills award $300, round wins $3000, and losses $1900, capped at $16000. Wins preserve bought guns and armor; losses remove them. Magazines, reserves and one of each grenade refresh next round. Buying does not pause the match.

## Team AI

Both sides use the same combat and perception system. Bots have entry, support, flanking and sniper roles. They:

- Pick visible opposing bots or the human, react after a short delay, share sightings with teammates, and investigate heard gunfire.
- Navigate around geometry, spread apart in lanes, strafe, fire controlled bursts, reload, and seek cover when injured or reloading.
- Wait for clear fire lanes instead of shooting through allies; incoming damage suppresses accuracy.
- Have entry bots throw a flash and supports throw smoke during engagements.
- Assign a T planter with escorts and flank support. After planting, Ts guard nearby positions.
- Assign the nearest living CT to defuse while others cover. If that CT dies, another takes over.

Sight checks are staggered rather than repeated at physics frequency; shots recheck line of sight. Hidden positions are not shared as live target data. AI accuracy is probabilistic, and teamwork is simplified for this prototype.

## Utilities and feedback

- **Frag:** Bouncing projectile, 1.25-second fuse, blast damage through clear physical lines. Smoke does not block damage. Your own frag can hurt you.
- **Smoke:** A 12-second cloud blocks AI sight. Bullets still pass through it.
- **Flash:** Blinds bots in line of sight for four seconds and whites out your screen if you face a nearby blast. Can affect teammates.

The kill feed, live T/CT counts, team roster, friendly markers, bomb beeps, mixed gunfire, footsteps, reload sounds, recoil, tracers, muzzle flashes, walking sway and damage feedback help track combat. If no audio device exists, play continues silently.

## Build and validate

```sh
# Windows cross-build with MinGW-w64
./build.sh
# For a locally extracted compiler, set CXX to x86_64-w64-mingw32-g++-posix.

# Portable gameplay and renderer tests
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests /tmp/breach.ppm

# Memory and undefined-behavior validation
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp -o /tmp/breach-sanitized
/tmp/breach-sanitized
```

Checks cover map connectivity and collision, economy and guns, automatic cadence, semi-auto trigger behavior, bolt-cycle locking, head/body hits, finite reloads, team counts, teammate combat, friendly bullet blocking, instant elimination results, independent bomb/round clocks, planting and defusing, death/spectating, utilities, AI navigation, full autonomous rounds on both sides, 3D viewmodels/reload poses, depth preservation, world occlusion and camera changes. Optional PPM output captures a current rendered scene.

The Windows executable is cross-compiled here; Windows mouse input, audio playback and window presentation still require a Windows play test. The visuals remain simple low-poly art. This is an original single-player prototype without networking or licensed Counter-Strike content.
