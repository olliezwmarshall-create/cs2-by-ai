# Breach / Operations

An original offline C++17 tactical shooter for 64-bit Windows. **Three large maps, mouse-driven map voting, 16 weapons, six-round side intervals, first-to-seven matches, team AI, recoverable weapon drops and paid utility.**

[Download Breach.zip](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip). Extract the **entire folder** and run **Breach.exe**. Keep the **assets** directory beside the executable: it contains the downloaded textured models. No installer or network connection is needed to play.

Click Play / T Side or Play / CT Side, vote for a map, purchase your loadout, then click Back to Game. Escape opens a clickable pause menu. The shop, settings, roster and match/round results support mouse interaction.

## Match rules

- Your team stays on the same side for **six rounds**. Sides change before rounds **7 and 13**. Your team's score follows it across side changes.
- **First team to seven round wins wins the MATCH.** A round result is not a match result. The final result screen announces Match Won / Match Lost and offers Vote Next Map instead of continuing the finished match.
- **T immediately wins the round when every CT is eliminated, planted or not.** T can also win by detonating the bomb.
- CT wins by eliminating every T before planting, surviving the pre-plant clock, or defusing. When every T dies after a plant, surviving CTs still have to defuse.
- Hold F or mouse side button 2 for three seconds in a bomb site to plant, or five seconds within 1.8 units of the device to defuse. Movement, switching guns, reloading, utility or releasing the button interrupts the action.
- Each round begins with eight seconds of freeze/buy time. The active round clock is **150 seconds**. Planting freezes that clock and starts a separate **40-second bomb countdown**.
- Each side has five AI players, with you joining your selected side as an additional player. Your death switches to spectating if teammates remain; their round continues.

## Maps and voting

Choose between three distinct layouts, each larger than the previous 64×64 arena:

| Map | Size | Scenery |
| --- | --- | --- |
| Shipyard | 80×80 | Warehouses, cranes, silos, cargo lanes and service vehicles |
| Old Quarter | 88×88 | Stucco buildings, pitched tile roofs, arches and courtyards |
| Radar Station | 96×96 | Radar domes, dishes, utility buildings and distant hills |

Voting opens before a new match and after a finished match. Click a map card to cast or change your vote. You and the ten bots each have one ballot; bots submit theirs over several seconds. After all bots vote, you can start early. Otherwise the winning map starts after ten seconds. A tied human choice wins a tied ballot; other ties use the first listed map. The same map stays active throughout a match.

Separated spawns, central buildings, two site courtyards, longer approaches and flanking connectors prevent immediate spawn-to-spawn firefights. Both sites remain on the CT side of each map. Round duration still depends on combat and player choices.

## Mouse and keyboard

| Input | Action |
| --- | --- |
| Mouse | Look / aim |
| Left click | Fire; hold for automatic guns; throw an equipped grenade |
| Right click (hold) | Aim / use scope |
| Wheel | Switch carried guns; change shop category or utility selection |
| Middle button (hold/release) | Choose and equip a grenade |
| Mouse side button 1 | Reload |
| Mouse side button 2 (hold) | Plant / defuse |
| WASD / Shift / Control | Move / sprint / crouch |
| Space | Jump |
| R | Reload |
| 1 / 2 | Equip carried primary / secondary |
| Q | Previous gun; return from an equipped grenade |
| E | Pick up a nearby dropped gun; a prompt names it |
| G | Drop your active gun |
| H | Quick-throw selected purchased utility |
| F (hold) | Plant / defuse |
| B | Clickable armory |
| Tab | Clickable roster |
| V / Wheel | Next teammate while spectating |
| Escape | Pause / back |
| Enter | Optional next-round shortcut on the result screen |

Sensitivity, master volume, FOV, shadows, invert-Y and borderless fullscreen are available in Settings. Preferences save to `%LOCALAPPDATA%\Breach\settings.cfg` when writable. Focus loss pauses the offline match. Shopping and utility selection leave combat running after freeze time.

## Arsenal

Carry **one primary and one secondary**. Buying or picking up another gun in the same slot drops the previous one with its remaining ammo. The wheel skips guns you do not carry.

| Weapon | Slot | Cost | Magazine | Damage per body hit / pellet | Behavior |
| --- | --- | --- | --- | --- | --- |
| Service pistol | Secondary | Free | 12 | 34 | Semi-auto |
| Heavy pistol | Secondary | $700 | 7 | 65 | High-damage semi-auto |
| Compact pistol | Secondary | $400 | 20 | 24 | Fast semi-auto |
| Burst pistol | Secondary | $650 | 18 | 28 | Three-shot committed burst |
| Auto carbine | Primary | $2400 | 30 | 35 | Automatic, ~923 RPM |
| Suppressed rifle | Primary | $2900 | 25 | 36 | Lower spread and shorter gunshot hearing radius |
| Battle rifle | Primary | $3000 | 20 | 48 | Heavy automatic rifle |
| Bullpup rifle | Primary | $2700 | 30 | 33 | Compact automatic rifle |
| Rapid SMG | Primary | $1200 | 32 | 24 | Automatic, 1200 RPM |
| Compact PDW | Primary | $1900 | 50 | 22 | Large-magazine automatic |
| Pump shotgun | Primary | $1600 | 8 | 13 × 8 pellets | Pump cycle, spread pattern |
| Auto shotgun | Primary | $2400 | 7 | 10 × 8 pellets | Automatic shotgun |
| Belt machine gun | Primary | $4500 | 80 | 30 | Sustained automatic fire, long reload |
| Bolt sniper | Primary | $3500 | 5 | 110 | Bolt-action, long-range scope |
| Light sniper | Primary | $2000 | 10 | 80 | Faster light bolt-action |
| Scout DMR | Primary | $2800 | 10 | 72 | Scoped semi-auto |

Body figures precede armor reduction. Headshots deal triple base damage; high-penetration guns retain more damage against armor. Movement and jumping reduce accuracy; aiming and crouching improve it. Fire cadence runs on a fixed gameplay step. Short clicks are queued so pressing and releasing between rendered frames still fires. The Windows loop requests a one-millisecond timer period while running so its short yield does not routinely stretch to a coarse system timer tick. Reloads consume finite reserves. Bolt and pump cycles visibly move and temporarily lock conflicting actions.

**Recoil follows a repeatable spray path**, with weapon-dependent strength, an initial upward climb and a horizontal weave during sustained automatic fire. The camera and bullet direction share the same aim kick: pull down and counter-steer to compensate. There is still a small random accuracy spread. Recoil eases back and the spray sequence resets after a firing pause; reloading or switching also resets the sequence. The first shot follows your current aim before applying its kick.

Scoped weapons have **no normal screen crosshair**, including while unscoped. Their reticle appears only inside the magnified scope lens.

Models differ by class: pistol slides, bullpup magazine placement, suppressor, shotgun tube/pump, machine-gun drum and belt, scopes and bolt handles. Reloads use eased reach, magazine extraction/insertion and return stages, with the support hand following the magazine. Spring-based weapon kick, breathing, distance-driven gait and eased sprint/equip transitions animate independently of abrupt input changes. Movement accelerates and brakes as a vector; crouching and aiming ease between poses.

## Economy, drops and utility

Start with a free service pistol and **$3200**, but **no free grenades**. Buy weapons through the Rifle/Heavy, SMG/Shotgun, Sniper and Sidearm tabs. Equipment sells:

- Armor: **$650**.
- Frag: **$300**, maximum one.
- Smoke: **$300**, maximum one.
- Flash: **$200**, maximum two.

Purchases require spawn proximity during freeze time or the first 22 seconds of active round time. A kill awards $300, a round win $3000 and a loss $1900, capped at $16000.

**Unused purchased utility stays with a survivor; used utility is not replenished for free next round.** Death removes utility and armor. Survivors retain their equipment even if their team loses. Gun magazines/reserves refresh for the next round; a player without a secondary receives the free service pistol.

Killed bots drop their gun, including magazine and reserve counts. The human drops both carried guns on death. Drops settle onto the ground, render as the corresponding 3D weapon and can be recovered with E. Manual drops and same-slot swaps use the same system. Drops clear at the start of the next round.

Frag grenades bounce, explode after 1.25 seconds and respect physical cover. Smoke creates a 12-second visibility screen while bullets pass through. Flash blinds bots for four seconds and can affect teammates or your own view. Friendly weapon/grenade damage is disabled, but allies can block shots and your own frag can hurt you.

## Hard AI and animation

Both teams use faster reactions and perception, higher accuracy, armor, weapon-dependent damage, faster controlled bursts, longer firing ranges and quicker movement. They still require visibility, react to suppression, avoid firing through allies and share nearby sightings rather than hidden live coordinates.

Bots navigate around buildings, flank, strafe, seek cover while hurt or reloading, and use smoke/flash on entry. A T planter has escorts; CT assigns the nearest surviving defuser, with others covering. Bot loadouts are pre-equipped rather than paid from a simulated bot economy.

Characters use articulated tapered limbs, knees/elbows, boots, armor, pouches, helmets and goggles, rather than a single colored box. Walking stride follows actual movement, with foot lift, torso motion and eased turn/stance poses. Reload hand motion, crouched objective poses, firing flashes and falling death poses are animated. This is procedural animation rather than a motion-captured character rig.

## See-through scopes and scene art

Scopes have hollow 3D housings and a **picture-in-picture magnified lens**. Looking through one keeps the surrounding scene at normal magnification, with a reticle inside the lens. The former opaque full-screen scope mask is gone.

Scenery includes continuous pitched roofs and gables, arches, drainpipes, roof vents, cranes, radar dishes, rounded tanks and domes, plus barrels, cargo and sloped barriers. Characters have rounded heads and helmets; weapon receivers, stocks and grips have bevelled edges. Surface materials include perspective-correct brick, stucco, paving, asphalt, wood, metal and woven cloth. Theme-specific sky and fog, cast shadows, contact shading, metal highlights and edge smoothing support the new shapes. Mipmapped, bilinearly filtered surface materials reduce distant texture shimmer; imported model textures are also filtered. Camera-relative billboards keep thin parts stable while turning. Weapon lighting uses its actual rendering camera. Scanline triangle coverage and incremental perspective interpolation reduce raster work while keeping full-resolution silhouettes. Scope passes clear only their lens render region. Detail levels and visibility culling limit distant geometry cost.

The game includes **UV-mapped textured Lantern, WaterBottle and CesiumMilkTruck models downloaded from KhronosGroup/glTF-Sample-Assets**. Lantern and WaterBottle are CC0-1.0; CesiumMilkTruck is copyright 2017 Cesium and licensed CC-BY-4.0. Its license, attribution and trademark notice are included with the assets. Original authors, pinned source URLs, hashes, license links and conversion details are in [assets/CREDITS.md](assets/CREDITS.md) and [assets/sources.json](assets/sources.json). These props are converted and bundled locally. The software renderer uses their base-color images, not the full PBR material stack. The overall art remains stylized rather than photorealistic.

## Build and validation

```sh
# Cross-build Windows executable and copy its runtime assets
./build.sh
# For an extracted compiler, set CXX to its x86_64-w64-mingw32-g++-posix executable.

# Gameplay, match, inventory, AI, menus and renderer checks
g++ -std=c++17 -O2 -Wall -Wextra -Werror -pedantic main.cpp -o /tmp/breach-tests
/tmp/breach-tests /tmp/breach.ppm

# Address / undefined-behavior checks
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer main.cpp -o /tmp/breach-sanitized
/tmp/breach-sanitized
```

Checks cover all three map layouts, voting revisions/ties/timeouts, map activation, circular cover, all 16 purchases, primary/secondary replacement, six-round halves, first-to-seven match completion and restart, paid utility carryover/consumption, burst/pellet/automatic fire, armor, drops and ammo-preserving pickups, bomb/timer rules, harder AI, full autonomous rounds, map connectivity and spawn separation, menu hit areas, movement, actual downloaded UV/image data, every viewmodel and reload pose, transparent scope composition, sniper crosshair suppression, recoil compensation/recovery, movement at different update rates, spring integration, camera-relative billboards, filtered texture sampling, scanline coverage, clipping and depth.

`tools/convert_gltf.py` reproduces the BRM asset conversion from the pinned GLB sources using Python, NumPy and Pillow; it is not needed to play or compile the game.

The Windows executable is cross-compiled. Windows mouse input, audio, font presentation and fullscreen behavior still require a Windows play test. There is no multiplayer or licensed Counter-Strike code or art.
