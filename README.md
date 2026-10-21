# Breach / Dockyard

An original offline tactical shooter for 64-bit Windows, written in C++17. Start, choose your team, buy equipment and adjust settings through **clickable mouse menus**. Both sides have five AI players, and you join your chosen side as an additional player.

[Download Breach.zip](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip), extract it and run **Breach.exe**. Click **Play / T Side** or **Play / CT Side**, buy a weapon, then click **Back to Game**. The game captures the mouse for aiming. Escape opens the pause menu and releases your cursor. Use the menu's Quit button to exit.

## New presentation and controls

- Main menu, pause menu, weapon/equipment shop, settings, utility selector, team roster and round-result screen have mouse hit areas and hover feedback. Number keys remain optional shortcuts for equipping owned guns; purchases use clickable cards.
- Scroll the wheel to cycle owned guns. In the utility selector it cycles grenades; in the shop it switches categories. While spectating, it changes teammates.
- Hold the middle mouse button, point at a grenade card and release to select it. You can also scroll or click a card. G throws the selected grenade.
- Settings have draggable sliders for sensitivity, volume and field of view, plus shadows, invert-Y and borderless fullscreen toggles. Save & Back or Escape saves them to `%LOCALAPPDATA%\Breach\settings.cfg` when that location is writable.
- Rendering includes perspective-correct procedural materials, textured masonry, paving and wood, directional lighting, baked floor shadows and ambient contact shading, facade windows, muzzle illumination, bullet marks and ejected casings.
- The 3D weapon models use smoother aim transitions, idle breathing, sprint lowering, recoil, reload tilting and moving magazines/bolts. Movement now has acceleration, stopping friction, crouching and a grounded jump.

All geometry and audio are generated in code. There are no downloaded game assets or runtime installers.

## Mouse and keyboard controls

| Input | Action |
| --- | --- |
| Mouse movement | Look / aim |
| Left click | Fire; hold for carbine or SMG |
| Right click (hold) | Aim / scope |
| Wheel | Cycle owned guns; menus use it for categories/selection |
| Middle button (hold/release) | Open / choose utility |
| Mouse side button 1 | Reload |
| Mouse side button 2 (hold) | Plant / defuse in range |
| WASD | Move / strafe |
| Shift | Sprint |
| Control | Crouch |
| Space | Jump |
| R | Reload |
| F (hold) | Plant / defuse in range |
| G | Throw selected grenade |
| Q | Quick-switch to previous weapon |
| 1–5 | Pistol / carbine / SMG / bolt sniper / DMR, if owned |
| B | Open the clickable armory |
| Tab | Open clickable roster screen |
| V | Next teammate while spectating |
| Escape | Pause / go back; does not immediately quit |
| Enter | Optional next-round shortcut on the result screen |

Movement uses WASD as in a standard desktop shooter. Team selection, shopping, settings, utility selection, resuming, starting the next round and quitting are available with the mouse.

## Round rules

The 32×32 Dockyard has two yellow bomb-site courtyards, connected middle and flanking routes, buildings and crate cover.

- **T wins immediately when every CT is eliminated, planted or not.** T can also plant and defend the bomb until detonation.
- **CT wins** by eliminating every T before a plant, surviving the pre-plant clock, or defusing the bomb. If every T dies after planting, surviving CTs must still defuse.
- Hold F or mouse side button 2 for **three seconds** in a site to plant, or **five seconds** within 1.8 units of a planted bomb to defuse. Movement, reloading, switching weapons, throwing utility or releasing the button interrupts the action.
- Each round starts with an **eight-second freeze/buy phase**. Movement, firing and bots wait while you shop. The 115-second round clock starts after that phase.
- Planting freezes the round clock; the bomb starts a separate **40-second countdown**. The HUD displays that bomb countdown in place of the round clock.
- Death does not end the round while teammates remain. You spectate a living teammate while the match continues.
- Your role alternates after each round. Click Next Round on the result screen. There is no fixed match length.

Pause, settings and the roster pause this offline match. Shopping and utility selection leave the match running, so use cover. Losing window focus opens the pause menu.

## Weapons, ammo and money

| Weapon | Cost | Magazine | Body damage | Fire behavior |
| --- | --- | --- | --- | --- |
| Service pistol | Free | 12 | 34 | Semi-auto, 0.22 s interval |
| Auto carbine | $2400 | 30 | 35 | Automatic, 0.065 s (~923 RPM) |
| Rapid SMG | $1200 | 32 | 24 | Automatic, 0.05 s (1200 RPM) |
| Bolt sniper | $3500 | 5 | 110 | Bolt-action, 1.2 s interval |
| Scout DMR | $2800 | 10 | 72 | Scoped semi-auto, 0.28 s interval |

The armory has weapon cards with prices, magazine sizes, damage bars and silhouettes. Owned guns can be equipped without repurchasing. Equipment offers armor for $650 and a utility refill for $300. Purchases require spawn proximity and the first 22 seconds of active round time; freeze time also permits purchases.

You start with $3200, a pistol and one of each grenade. Your kills award $300, round wins $3000 and losses $1900, capped at $16000. Wins preserve bought weapons and armor; losses remove them. Each round refreshes magazines, finite ammo reserves and one of each utility.

Headshots deal triple damage. Moving and jumping reduce accuracy; aiming and crouching improve it. Each gun retains its own ammo. Switching cancels a reload; the bolt sniper blocks switching and reloading during its bolt cycle. Friendly damage is disabled, but teammates can block player bullets. Your own frag can hurt you.

## Bots and utilities

Bots on both sides engage opposing bots and the human. Entry, support, flanker and sniper roles share sightings, investigate gunfire, navigate around cover, separate in narrow lanes, reload, strafe, fire controlled bursts and seek cover when injured or reloading. They wait for clear friendly fire lanes, react after a short delay and lose accuracy under damage suppression.

T assigns a planter and escort/flanking roles, then guards the planted bomb. CT assigns the nearest living defuser with others covering; another takes over if that bot dies. Entry bots can flash and supports can smoke during engagements. Perception is staggered for performance and shots recheck visibility.

- **Frag:** Bounces and explodes after 1.25 seconds. Physical cover blocks damage; smoke does not.
- **Smoke:** Creates a 12-second cloud that blocks AI sight. Bullets still pass through.
- **Flash:** Blinds bots in line of sight for four seconds; can affect teammates and white out your screen when facing a nearby blast.

The interface shows live team counts, money, objectives, selected equipment, kill feed, friendly markers, roster and spectating status. Gunfire, footsteps, reloads, explosions and bomb beeps use mixed procedural audio. A missing audio device results in silent play.

## Build and validation

```sh
# Cross-compile the Windows executable using MinGW-w64
./build.sh
# For an extracted compiler, set CXX to its x86_64-w64-mingw32-g++-posix executable.

# Portable gameplay, renderer and mouse-menu logic checks
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests /tmp/breach.ppm

# Memory and undefined-behavior checks
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp -o /tmp/breach-sanitized
/tmp/breach-sanitized
```

Checks cover mouse menu transitions, clickable purchases and disabled cards, scroll selection, persistent settings, freeze time, movement/crouch/jump, team combat and spectating, weapons and fire cadence, bomb rules and timers, autonomous rounds from both roles, utility, map navigation, 3D viewmodel poses, renderer depth and camera behavior. Optional PPM output captures a current software-rendered scene.

The executable is cross-compiled here. Windows input, font rendering, fullscreen behavior and audio playback still need a Windows play test. This remains a stylized low-poly single-player prototype with an original software renderer; it does not include networking or licensed Counter-Strike content.
