# Breach / Harbor — Match Edition

An original offline C++17 tactical shooter for 64-bit Windows. **16 weapons, a 64×64 map, six-round side intervals, first-to-seven matches, harder team AI, recoverable weapon drops and paid utility.**

[Download Breach.zip](https://github.com/olliezwmarshall-create/cs2-by-ai/raw/refs/heads/main/dist/Breach.zip). Extract the **entire folder** and run **Breach.exe**. Keep the **assets** directory beside the executable: it contains the downloaded textured models. No installer or network connection is needed to play.

Click Play / T Side or Play / CT Side, purchase your loadout, then click Back to Game. Escape opens a clickable pause menu. The shop, settings, roster and match/round results support mouse interaction.

## Match rules

- Your team stays on the same side for **six rounds**. Sides change before rounds **7 and 13**. Your team's score follows it across side changes.
- **First team to seven round wins wins the MATCH.** A round result is not a match result. The final result screen announces Match Won / Match Lost and offers New Match instead of continuing the finished match.
- **T immediately wins the round when every CT is eliminated, planted or not.** T can also win by detonating the bomb.
- CT wins by eliminating every T before planting, surviving the pre-plant clock, or defusing. When every T dies after a plant, surviving CTs still have to defuse.
- Hold F or mouse side button 2 for three seconds in a bomb site to plant, or five seconds within 1.8 units of the device to defuse. Movement, switching guns, reloading, utility or releasing the button interrupts the action.
- Each round begins with eight seconds of freeze/buy time. The active round clock is **150 seconds**. Planting freezes that clock and starts a separate **40-second bomb countdown**.
- Each side has five AI players, with you joining your selected side as an additional player. Your death switches to spectating if teammates remain; their round continues.

Harbor covers **64×64 units**, four times the previous map's area. Separated spawns, a blocking central warehouse, two site courtyards, multiple building clusters, longer approaches and flanking connectors prevent immediate spawn-to-spawn firefights. The two sites are in the northeast and southeast. Match length still depends on combat and player choices; larger routes do not impose an artificial minimum round duration.

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

Body figures precede armor reduction. Headshots deal triple base damage; high-penetration guns retain more damage against armor. Movement, jumping and recoil reduce accuracy; aiming and crouching improve it. Fire cadence runs on a fixed gameplay step. Reloads consume finite reserves. Bolt and pump cycles visibly move and temporarily lock conflicting actions.

Models differ by class: pistol slides, bullpup magazine placement, suppressor, shotgun tube/pump, machine-gun drum and belt, scopes and bolt handles. Magazine changes, equip transitions, recoil, breathing and sprint lowering are animated.

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

Characters use articulated tapered limbs, knees/elbows, boots, armor, pouches, helmets and goggles, rather than a single colored box. Walking stride follows actual movement. Reload hand motion, crouched objective poses, firing flashes and falling death poses are animated. This is procedural animation rather than a motion-captured character rig.

## See-through scopes and scene art

Scopes have hollow 3D housings and a **picture-in-picture magnified lens**. Looking through one keeps the surrounding scene at normal magnification, with a reticle inside the lens. The former opaque full-screen scope mask is gone.

Props include rounded barrels with hoops/caps, braced crates, corrugated cargo boxes and sloped barriers. Surface materials include perspective-correct masonry, paving, wood, metal and woven cloth, with lighting, shadows, contact shading, muzzle illumination, shell casings and bullet marks.

The game also includes **UV-mapped textured Lantern and WaterBottle models downloaded from KhronosGroup/glTF-Sample-Assets**, licensed CC0-1.0. Original authors, pinned source URLs, hashes, license links and conversion details are in [assets/CREDITS.md](assets/CREDITS.md) and [assets/sources.json](assets/sources.json). These props are converted and bundled locally. The software renderer uses their base-color images, not the full PBR material stack. The overall art remains stylized rather than photorealistic.

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

Checks cover all 16 purchases, primary/secondary replacement, six-round halves, first-to-seven match completion and restart, paid utility carryover/consumption, burst/pellet/automatic fire, armor, drops and ammo-preserving pickups, bomb/timer rules, harder AI, full autonomous rounds, map connectivity and spawn separation, menu hit areas, movement, actual downloaded UV/image data, every viewmodel and reload pose, transparent scope composition, clipping and depth.

`tools/convert_gltf.py` reproduces the BRM asset conversion from the pinned GLB sources using Python, NumPy and Pillow; it is not needed to play or compile the game.

The Windows executable is cross-compiled. Windows mouse input, audio, font presentation and fullscreen behavior still require a Windows play test. There is no multiplayer or licensed Counter-Strike code or art.
