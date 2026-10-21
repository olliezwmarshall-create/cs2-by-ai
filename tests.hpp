#include <cassert>
#include <unordered_set>
void simulate(Game &g, double seconds, double step = 1. / 120) {
  for (int i = 0; i < int(std::ceil(seconds / step)); i++)
    g.tick(step);
}
Bot target(double x, double y, bool terrorist = false) {
  Bot b{x, y};
  b.terrorist = terrorist;
  b.blind = 1000;
  b.armor = 0;
  return b;
}
Game lane() {
  Game g;
  g.x = 3;
  g.y = 4;
  g.a = g.pitch = 0;
  g.bots = {target(60, 60)};
  return g;
}
void imageFile(const std::string &path) {
  std::ofstream f(path, std::ios::binary);
  f << "P6\n" << W << " " << H << "\n255\n";
  for (auto c : pixels) {
    char b[] = {char(c >> 16), char(c >> 8), char(c)};
    f.write(b, 3);
  }
}
int main(int argc, char **argv) {
  assert(MAP == 64 && WEAPON_COUNT == 16);
  Game g;
  int ts = 0, cts = 0;
  for (auto b : g.bots) {
    assert(!arena.solid(b.x, b.y, .23));
    assert(!g.route(HOME_T.x, HOME_T.y, b.x, b.y).empty());
    (b.terrorist ? ts : cts)++;
  }
  assert(ts == 5 && cts == 5 && g.alive(true) == 6 && g.alive(false) == 5);
  assert(!arena.solid(g.x, g.y, .23));
  assert(g.route(HOME_T.x, HOME_T.y, SITE_A.x, SITE_A.y).size() > 60);
  assert(g.route(HOME_T.x, HOME_T.y, SITE_B.x, SITE_B.y).size() > 60);
  for (auto &b : g.bots)
    if (!b.terrorist)
      assert(!g.visible(g.x, g.y, b.x, b.y));
  // Six-round halves, score tied to the player's team, and first to seven.
  bool side = g.attacker;
  for (int r = 1; r <= 6; r++) {
    assert(g.round == r && g.attacker == side);
    g.finish(r % 2 == 1, "TEST");
    g.reset();
  }
  assert(g.round == 7 && g.attacker != side && g.wins == 3 && g.losses == 3);
  for (int i = 0; i < 4; i++) {
    g.finish(true, "TEST");
    if (i < 3)
      g.reset();
  }
  assert(g.matchOver && g.matchWon && g.wins == 7 && g.losses == 3);
  int round = g.round;
  g.reset();
  assert(g.round == round);
  g.finish(true, "DUPLICATE");
  assert(g.wins == 7);
  g = Game();
  for (int i = 0; i < 7; i++) {
    g.finish(false, "TEST");
    if (i < 6)
      g.reset();
  }
  assert(g.matchOver && !g.matchWon && g.losses == 7);
  // Every purchasable gun has ammo, distinct stats, and a correct inventory
  // slot.
  for (int gun = 1; gun < WEAPON_COUNT; gun++) {
    g = Game();
    g.money = 16000;
    assert(g.buyGun(gun));
    assert(g.weapon == gun && g.owned[gun] &&
           g.ammo() == weapons[gun].capacity);
    assert((secondaryGun(gun) ? g.secondary : g.primary) == gun);
    assert(g.money == 16000 - weapons[gun].price);
  }
  g = Game();
  g.money = 16000;
  assert(g.buyGun(1));
  g.magazines[1] = 9;
  g.reserve[1] = 42;
  assert(g.buyGun(8));
  assert(!g.owned[1] && g.primary == 8 && g.drops.size() == 1 &&
         g.drops[0].ammo == 9 && g.drops[0].reserve == 42);
  assert(g.buyGun(5) && g.secondary == 5 && !g.owned[0]);
  g.time = 23;
  assert(!g.buyGun(6));
  // Utility must be bought independently; none is granted by round reset.
  g = Game();
  assert((g.utilities == std::array<int, 3>{0, 0, 0}));
  int balance = g.money;
  assert(g.buyUtility(0) && g.money == balance - 300);
  assert(!g.buyUtility(0));
  assert(g.buyUtility(1) && g.buyUtility(2));
  g.reset();
  assert(g.utilities[0] == 1);
  g.utility = 0;
  g.throwUtility();
  assert(g.utilities[0] == 0);
  g.reset();
  assert(g.utilities[0] == 0);
  // Body/head hits, armor, wall cover and weapon spread modes.
  g = lane();
  g.bots = {target(5, 4), target(60, 4)};
  g.pitch = -.12;
  g.fire();
  assert(g.bots[0].hp == 66 && g.ammo() == 11);
  g = lane();
  g.bots = {target(5, 4), target(60, 4)};
  g.fire();
  assert(g.bots[0].hp <= 0 && g.kills == 1 && g.drops.size() == 1);
  g = lane();
  g.bots = {target(5, 4), target(60, 4)};
  g.bots[0].armor = 70;
  g.pitch = -.12;
  g.fire();
  assert(g.bots[0].hp == 74 && g.bots[0].armor < 70);
  g = lane();
  g.x = 5;
  g.y = 32;
  g.bots = {target(12, 32)};
  g.fire();
  assert(g.bots[0].hp == 100);
  for (int gun : {1, 2}) {
    g = lane();
    g.weapon = gun;
    g.primary = gun;
    g.owned[gun] = true;
    for (int i = 0; i < 240; i++) {
      g.tick(1. / 240);
      g.trigger(true, i == 0);
    }
    assert(weapons[gun].capacity - g.ammo() == (gun == 1 ? 16 : 20));
  }
  g = lane();
  for (int i = 0; i < 240; i++) {
    g.tick(1. / 240);
    g.trigger(true, i == 0);
  }
  assert(g.ammo() == 11);
  g = lane();
  g.weapon = 7;
  g.secondary = 7;
  g.owned[7] = true;
  for (int i = 0; i < 120; i++) {
    g.tick(1. / 240);
    g.trigger(true, i == 0);
  }
  assert(g.ammo() == 15 && g.burstRemaining == 0);
  g = lane();
  g.weapon = 11;
  g.primary = 11;
  g.owned[11] = true;
  g.fire();
  assert(g.ammo() == 7 && g.tracers.size() == 8 && g.boltCycle > 0);
  g = lane();
  g.weapon = 3;
  g.primary = 3;
  g.owned[3] = true;
  g.fire();
  g.startReload();
  g.switchWeapon(0);
  assert(g.ammo() == 4 && g.weapon == 3 && g.reload == 0);
  simulate(g, 1.3);
  g.trigger(true, false);
  assert(g.ammo() == 4);
  g.trigger(true, true);
  assert(g.ammo() == 3);
  g = lane();
  g.magazines[0] = 0;
  g.reserve[0] = 4;
  g.startReload();
  simulate(g, 1.4);
  assert(g.ammo() == 4 && g.reserve[0] == 0);
  // Killed bots and the human drop recoverable, ammo-preserving weapons once.
  g = lane();
  g.bots = {target(5, 4), target(60, 4)};
  g.bots[0].gun = 9;
  g.bots[0].ammo = 6;
  g.damageBot(0, 200, -1);
  assert(g.drops.size() == 1 && g.drops[0].gun == 9 && g.drops[0].ammo == 6);
  g.damageBot(0, 200, -1);
  assert(g.drops.size() == 1);
  g.x = 4.5;
  assert(g.pickup() && g.primary == 9 && g.ammo() == 6);
  g.dropGun(9);
  assert(!g.owned[9] && g.drops.back().gun == 9);
  g = Game();
  g.money = 16000;
  g.buyGun(1);
  g.hp = 0;
  g.playerDeath();
  size_t n = g.drops.size();
  assert(n == 2 && !g.owned[0] && !g.owned[1]);
  g.playerDeath();
  assert(g.drops.size() == n);
  // Player plant interruption, independent countdown, and both defuse paths.
  g = lane();
  g.x = SITE_A.x;
  g.y = SITE_A.y;
  g.usingObjective = true;
  simulate(g, 1);
  assert(g.plantProgress > .9 && !g.planted);
  g.move(.2, 0);
  assert(g.plantProgress == 0);
  g.walking = false;
  simulate(g, 3.1);
  assert(g.planted);
  double frozen = g.clock, bomb = g.bombClock;
  simulate(g, 1);
  assert(g.clock == frozen && g.bombClock < bomb);
  simulate(g, 40);
  assert(g.ended && g.won && g.result == "BOMB DETONATED");
  g = lane();
  g.planted = true;
  g.bombX = SITE_A.x;
  g.bombY = SITE_A.y;
  g.bots = {target(SITE_A.x, SITE_A.y)};
  g.bots[0].blind = 0;
  simulate(g, 5.2);
  assert(g.ended && !g.won && g.result == "CT WIN - BOMB DEFUSED");
  g = lane();
  g.attacker = false;
  g.x = SITE_A.x;
  g.y = SITE_A.y;
  g.bots = {target(3, 4, true)};
  g.planted = true;
  g.bombX = g.x;
  g.bombY = g.y;
  g.usingObjective = true;
  simulate(g, 5.1);
  assert(g.ended && g.won && g.result == "BOMB DEFUSED");
  for (bool planted : {false, true}) {
    g = lane();
    g.bots = {target(5, 4)};
    g.planted = planted;
    g.fire();
    assert(g.ended && g.won && g.result == "T WIN - ALL CT ELIMINATED");
  }
  g = lane();
  g.hp = 0;
  g.planted = true;
  g.bombX = SITE_B.x;
  g.bombY = SITE_B.y;
  g.tick(.01);
  assert(!g.ended);
  g = lane();
  g.clock = .001;
  g.tick(.01);
  assert(g.ended && !g.won);
  // Smoke affects sight, not bullets or blast; flash, paid selection and throw.
  g = lane();
  g.bots = {target(10, 4), target(60, 4)};
  assert(g.visible(3, 4, 10, 4));
  g.smokes.push_back({6, 4, 12});
  assert(!g.visible(3, 4, 10, 4));
  assert(g.geometry({3, 4, 1.6}, direction(0), 7) > 6.9);
  g = lane();
  g.bots = {target(10, 4), target(60, 4)};
  g.smokes.push_back({10, 4, 12});
  g.grenades.push_back({{10, 4, .2}, {0, 0, 0}, 0, .01});
  g.tick(.02);
  assert(g.bots[0].hp == 0);
  g = lane();
  g.bots = {target(10, 4), target(60, 4)};
  g.bots[0].blind = 0;
  g.grenades.push_back({{8, 4, 1}, {0, 0, 0}, 2, .01});
  g.tick(.02);
  assert(g.bots[0].blind > 3);
  g = lane();
  g.utilities[1] = 1;
  g.utility = 1;
  g.utilityEquipped = true;
  g.trigger(true, true);
  assert(g.utilities[1] == 0 && !g.utilityEquipped && !g.grenades.empty());
  // Hard bots react quickly and fire meaningful bursts without seeing through
  // walls.
  g = lane();
  g.hp = 1000;
  g.bots = {target(10, 4)};
  auto &bot = g.bots[0];
  bot.blind = 0;
  bot.facing = PI;
  bot.cooldown = 0;
  simulate(g, 2);
  assert(g.hp < 750 && bot.ammo < 5);
  g = lane();
  g.x = 5;
  g.y = 32;
  g.bots = {target(12, 32)};
  g.bots[0].blind = 0;
  g.bots[0].facing = PI;
  g.tick(.02);
  assert(!g.bots[0].knows);
  g = lane();
  g.hp = 0;
  g.bots = {target(3, 4, true), target(60, 4)};
  g.tick(.01);
  assert(!g.ended && g.spectating() == 0 && g.eye().x == 3);
  for (bool t : {true, false}) {
    g = Game();
    g.attacker = t;
    g.startAttacker = t;
    g.round = 0;
    g.reset();
    g.hp = 0;
    for (int i = 0; i < 12000 && !g.ended; i++)
      g.tick(1. / 60);
    assert(g.ended);
    for (auto b : g.bots)
      assert(!arena.solid(b.x, b.y, .19));
  }
  // Mouse-driven weapon categories, secondary purchases, utility and match
  // flow.
  Interface menu;
  g = Game();
  auto click = [&](int id) {
    bool found = false;
    for (auto b : menu.buttons(g))
      if (b.action == id) {
        assert(b.enabled);
        menu.click(b.r.x + b.r.w / 2, b.r.y + b.r.h / 2, g);
        found = true;
        break;
      }
    assert(found);
  };
  click(1);
  assert(menu.page == ScreenPage::Buy && g.round == 1 && g.freeze == 8);
  click(101);
  assert(g.primary == 1 && g.money == 800);
  click(23);
  assert(menu.buyTab == 3);
  click(105);
  assert(g.secondary == 5 && g.money == 100);
  click(5);
  menu.wheel(1, g);
  assert(g.weapon == 1 || g.weapon == 5);
  menu.escape(g);
  click(3);
  assert(menu.page == ScreenPage::Settings);
  assert(menu.slider(795, 200));
  click(30);
  click(32);
  click(9);
  assert(menu.page == ScreenPage::Pause);
  assert(menu.prefs.save("/tmp/breach-settings-test.cfg"));
  Preferences prefs;
  prefs.load("/tmp/breach-settings-test.cfg");
  assert(prefs.sensitivity == 3 && prefs.fullscreen && !prefs.shadows);
  click(5);
  g.wins = 6;
  g.finish(true, "TEST");
  menu.update(.01, g);
  assert(g.matchOver && menu.page == ScreenPage::Result);
  click(10);
  assert(!g.matchOver && g.round == 1 && g.wins == 0 &&
         menu.page == ScreenPage::Buy);
  // Freeze, movement, crouch and landing.
  g = lane();
  g.freeze = 1;
  double clock = g.clock;
  g.fire();
  assert(g.ammo() == 12);
  simulate(g, .5);
  assert(g.clock == clock);
  simulate(g, .6);
  assert(g.freeze == 0 && g.clock < clock);
  g = lane();
  g.locomotion(.01, 1, 0, false, false);
  assert(g.vx > 0 && g.vx < 2.6);
  for (int i = 0; i < 30; i++)
    g.locomotion(.01, 0, 0, false, true);
  assert(g.vx == 0 && g.stance > .9 && g.eye().z < 1.2);
  g.stance = 0;
  g.jump();
  g.tick(.1);
  assert(g.jumpHeight > 0);
  simulate(g, 1);
  assert(g.jumpHeight == 0);
  // Downloaded CC0 geometry actually loads UVs and non-flat texture images.
  assert(loadArt("assets"));
  assert(lanternArt.faces.size() > 1000 && bottleArt.faces.size() > 1000);
  std::unordered_set<uint32_t> colors(bottleArt.images[0].pixels.begin(),
                                      bottleArt.images[0].pixels.end());
  assert(colors.size() > 1000);
  assert(!bottleArt.load("/tmp/does-not-exist-brm"));
  // All weapons produce distinct 3D views and reload poses while preserving
  // world depth. The scope magnifies only its lens and leaves the periphery.
  game = lane();
  aimBlend = 0;
  drawWorld();
  auto worldDepth = zbuffer;
  auto basePixels = pixels;
  for (int gun = 0; gun < WEAPON_COUNT; gun++) {
    game.weapon = gun;
    game.owned[gun] = true;
    pixels = basePixels;
    drawViewmodel();
    assert(pixels != basePixels && zbuffer == worldDepth);
    auto held = pixels;
    game.reload = weapons[gun].reload * .5;
    pixels = basePixels;
    drawViewmodel();
    assert(pixels != held);
    game.reload = 0;
  }
  game = lane();
  game.bots = {target(10, 4)};
  game.weapon = 3;
  game.owned[3] = true;
  aimBlend = 1;
  drawWorld();
  auto outside = pixels;
  drawScopeLens();
  assert(pixels != outside && pixels[20 * W + 20] == outside[20 * W + 20]);
  drawViewmodel();
  assert(pixels[H / 2 * W + W / 2] != 0x10181C &&
         pixels[H / 2 * W + W / 2] != 0x578D98);
  aimBlend = 0;
  // Near-plane clipping and depth ordering remain correct with textured meshes.
  std::fill(pixels.begin(), pixels.end(), 0);
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  raster({-1, -1, 4}, {1, -1, 4}, {0, 1, 4}, 0xFF0000);
  raster({-1, -1, 2}, {1, -1, 2}, {0, 1, 2}, 0x00FF00);
  assert(((pixels[H / 2 * W + W / 2] >> 8) & 255) >
         ((pixels[H / 2 * W + W / 2] >> 16) & 255));
  for (auto page : {ScreenPage::Home, ScreenPage::Pause, ScreenPage::Buy,
                    ScreenPage::Settings, ScreenPage::Utility,
                    ScreenPage::Result, ScreenPage::Roster}) {
    game = Game();
    ui.page = page;
    render();
    drawInterface();
    assert(!interfaceText.empty());
    for (auto b : ui.buttons(game))
      assert(b.r.x >= 0 && b.r.y >= 0 && b.r.x + b.r.w <= W &&
             b.r.y + b.r.h <= H);
  }
  ui = Interface();
  game = Game();
  game.x = 40;
  game.y = 18;
  game.a = .2;
  game.pitch = -.03;
  game.weapon = 9;
  game.owned[9] = true;
  game.equipAnim = 0;
  game.bots = {target(45, 19)};
  render();
  if (argc > 1)
    imageFile(argv[1]);
  puts("PASS: 64x64 map/spawn separation, 16 weapons and paid secondaries, "
       "two-slot inventory, six-round halves, first-to-seven match, paid "
       "utility persistence, burst/pellet/automatic cadence, armor, "
       "drops/pickups, bomb/timers, harder AI, autonomous rounds, mouse menus, "
       "movement, CC0 UV assets, 3D animations/viewmodels, transparent "
       "picture-in-picture scopes, depth and UI layouts");
  return 0;
}
