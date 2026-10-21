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
  assert(MAP == 96 && WEAPON_COUNT == 16);
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
  g.y = HOME_T.y;
  g.bots = {target(12, HOME_T.y)};
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
  g.y = HOME_T.y;
  g.bots = {target(12, HOME_T.y)};
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
  // Map voting accepts one revisable human ballot, ten staggered AI votes,
  // deterministic ties and timeout. Only the winning map becomes active.
  MapVote ballot;
  ballot.begin(3);
  ballot.choose(1);
  ballot.choose(2);
  assert(ballot.totals()[1] == 0 && ballot.totals()[2] == 1);
  ballot.update(2);
  assert(!ballot.botsDone() && !ballot.expired());
  ballot.update(4);
  assert(ballot.botsDone());
  int total = 0;
  for (int n : ballot.totals())
    total += n;
  assert(total == 11);
  ballot.ballots.fill(-1);
  ballot.choose(2);
  ballot.ballots[1] = 1;
  assert(ballot.winner() == 2);
  ballot.human = -1;
  ballot.ballots[0] = -1;
  ballot.ballots[2] = 2;
  assert(ballot.winner() == 1);
  Interface voting;
  g = Game();
  voting.beginVote(g, false);
  voting.vote.choose(1);
  voting.vote.ballots.fill(1);
  voting.update(10.1, g);
  assert(arena.id == 1 && arena.size == 88 && g.attacker == false &&
         g.round == 1 && voting.page == ScreenPage::Buy);
  selectMap(0);
  // Every map has separate spawns, long reachable routes and real round cover.
  std::array<size_t, 3> routes;
  for (int map = 0; map < 3; map++) {
    selectMap(map);
    g = Game();
    assert(arena.size == MAP_SIZES[map] && arena.size > 64);
    assert(HOME_T.x == arena.homeT.x && SITE_B.y == arena.siteB.y);
    assert(!arena.solid(g.x, g.y, .23));
    for (auto b : g.bots) {
      assert(!arena.solid(b.x, b.y, .23));
      assert(!g.route(HOME_T.x, HOME_T.y, b.x, b.y).empty());
      if (!b.terrorist)
        assert(!g.visible(g.x, g.y, b.x, b.y));
    }
    routes[map] = g.route(HOME_T.x, HOME_T.y, SITE_A.x, SITE_A.y).size();
    assert(routes[map] > 60);
    assert(!g.route(HOME_T.x, HOME_T.y, SITE_B.x, SITE_B.y).empty());
    for (auto c : arena.rounds)
      assert(arena.solid(c.x, c.y));
    // The larger layout must still support a complete autonomous round.
    g.hp = 0;
    for (int i = 0; i < 12000 && !g.ended; i++)
      g.tick(1. / 60);
    assert(g.ended);
    int casualties = 0;
    for (auto b : g.bots) {
      assert(!arena.solid(b.x, b.y, .19));
      casualties += b.hp <= 0;
    }
    assert(casualties > 0);
  }
  assert(routes[0] != routes[1] || routes[1] != routes[2]);
  selectMap(0);
  RoundSolid cylinder{3, 0, 1, 2, false, 0};
  assert(std::abs(roundHit({0, 0, 1}, {1, 0, 0}, cylinder) - 2) < .001);
  cylinder.dome = true;
  assert(std::abs(roundHit({3, 0, 4}, {0, 0, -1}, cylinder) - 1) < .001);
  // Rounds on a selected map retain six-round halves and first-to-seven scores.
  for (int map = 0; map < 3; map++) {
    selectMap(map);
    g = Game();
    for (int r = 0; r < 6; r++) {
      g.finish(true, "TEST");
      g.reset();
    }
    assert(g.round == 7 && !g.attacker && g.wins == 6);
    g.finish(true, "TEST");
    assert(g.matchOver && g.matchWon);
  }
  selectMap(0);
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
  assert(menu.page == ScreenPage::Vote && !menu.simulates());
  click(400);
  menu.vote.ballots.fill(0);
  click(410);
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
  assert(menu.page == ScreenPage::Vote);
  click(400);
  menu.vote.ballots.fill(0);
  click(410);
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
  // Held rifle fire follows a repeatable climb/weave regardless of the
  // random spread seed; counter-steering the view keeps the muzzle on target.
  auto spray = [](uint32_t seed, bool compensate) {
    Game trial = lane();
    trial.weapon = 1;
    trial.owned[1] = true;
    trial.random = seed;
    std::vector<Vec> directions;
    for (int n = 0; n < 20; n++) {
      if (compensate) {
        trial.a = -trial.recoilYaw;
        trial.pitch = -trial.recoilPitch;
      }
      trial.fire();
      const auto &t = trial.tracers.back();
      Vec ray = t.to - t.from;
      directions.push_back(ray * (1 / length(ray)));
      trial.tick(trial.gun().delay);
    }
    assert(trial.magazines[1] == 10 && trial.sprayShot == 20);
    assert(trial.recoilPitch > .05);
    assert(std::abs(trial.viewPitch() - trial.pitch - trial.recoilPitch) <
           1e-9);
    simulate(trial, 1);
    assert(trial.sprayShot == 0 && std::abs(trial.recoilPitch) < .0001 &&
           std::abs(trial.recoilYaw) < .0001 &&
           std::abs(trial.weaponKick) < .0001);
    return directions;
  };
  auto rising = spray(13, false), compensated = spray(13, true),
       alternate = spray(99, false);
  assert(rising.back().z > rising.front().z + .04);
  assert(std::abs(compensated.back().z) < .02);
  assert(rising[19].y - compensated[19].y >
         rising[10].y - compensated[10].y + .015);
  assert(std::abs(rising[15].z - alternate[15].z) < .025);
  // The same elapsed movement at different fixed update rates has similar
  // distance, gait and aim recovery. Small substeps must still count as
  // walking.
  auto travel = [](double dt) {
    Game trial = lane();
    for (int i = 0; i < int(1 / dt); i++) {
      trial.locomotion(dt, 1, .5, false, false);
      trial.tick(dt);
    }
    assert(trial.walking && trial.moveBlend > .9 && trial.walkPhase > 10);
    for (int i = 0; i < int(.5 / dt); i++) {
      trial.locomotion(dt, 0, 0, false, false);
      trial.tick(dt);
    }
    assert(!trial.walking && trial.moveBlend < .05 && trial.vx == 0 &&
           trial.vy == 0);
    return Vec{trial.x, trial.y, trial.walkPhase};
  };
  Vec coarse = travel(1. / 60), fine = travel(1. / 240);
  assert(dist(coarse.x, coarse.y, fine.x, fine.y) < .06);
  double springX = .3, springV = 4, subdividedX = springX,
         subdividedV = springV;
  spring(springX, springV, 0, 25, .1);
  for (int i = 0; i < 24; i++)
    spring(subdividedX, subdividedV, 0, 25, .1 / 24);
  assert(std::abs(springX - subdividedX) < 1e-10 &&
         std::abs(springV - subdividedV) < 1e-10);
  // Scoped weapons have only their lens reticle, at either aim state. Normal
  // guns retain a hip-fire crosshair and utility does not display one.
  game = lane();
  for (int gun : {3, 4, 14})
    for (double blend : {0., 1.}) {
      game.weapon = gun;
      aimBlend = blend;
      std::fill(pixels.begin(), pixels.end(), 0x123456);
      drawAimIndicators();
      assert(std::all_of(pixels.begin(), pixels.end(),
                         [](uint32_t c) { return c == 0x123456; }));
    }
  game.weapon = 0;
  drawAimIndicators();
  assert(pixels[H / 2 * W + W / 2 + 8] != 0x123456);
  // A billboard uses the supplied camera, independent of the player's world
  // heading. This also protects viewmodels and the magnified lens pass.
  Camera fixedCamera({0, 0, 0}, 0, 0);
  auto beamImage = [&](double heading) {
    game.a = heading;
    std::fill(pixels.begin(), pixels.end(), 0);
    std::fill(zbuffer.begin(), zbuffer.end(), 0);
    activeMaterial = 0;
    beam({3, -.15, 0}, {4, .15, .1}, .04, 0xABCDEF, fixedCamera);
    return pixels;
  };
  assert(beamImage(0) == beamImage(2));
  // Completing a reload returns continuously to the resting pose.
  game = lane();
  game.reload = .00001;
  Vec almostRest = WeaponModel().point({.4, .1, .1});
  game.reload = 0;
  Vec rest = WeaponModel().point({.4, .1, .1});
  assert(length(almostRest - rest) < .00001);
  // Downloaded licensed geometry actually loads UVs and non-flat texture
  // images.
  assert(loadArt("assets"));
  assert(lanternArt.faces.size() > 1000 && bottleArt.faces.size() > 1000 &&
         truckArt.faces.size() > 1000);
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
  for (int map = 0; map < 3; map++) {
    selectMap(map);
    game = Game();
    game.x = arena.showcase.x;
    game.y = arena.showcase.y;
    game.a = .25;
    game.weapon = 1;
    game.owned[1] = true;
    render();
    assert(shadowField.revision == arena.revision);
    assert(*std::max_element(zbuffer.begin(), zbuffer.end()) > 0);
  }
  selectMap(0);
  // Filtered texture sampling wraps correctly and blends four texels.
  TextureImage checker;
  checker.w = checker.h = 2;
  checker.pixels = {0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF};
  assert(checker.sample(.25, .25) == 0xFF0000);
  assert(checker.sample(.5, .5) == 0x7F7F7F);
  assert(checker.sample(-.75, 1.25) == 0xFF0000);
  assert(std::abs(textures.sample({1, 2, 0}, {0, 0, 1}, 1, 7) -
                  textures.sample({1.38, 2.77, 0}, {0, 0, 1}, 1, 7)) < 1e-6);
  // Check optimized scanline coverage and perspective depth against direct
  // barycentric evaluation, for both triangle windings and offscreen edges.
  for (bool reverse : {false, true}) {
    Vec a{-.8, -.7, 1.5}, b{1.2, -.3, 4}, c{.1, .9, 2.2};
    if (reverse)
      std::swap(a, c);
    std::fill(zbuffer.begin(), zbuffer.end(), 0);
    raster(a, b, c, 0xEEEEEE);
    auto p = screen(a), q = screen(b), r = screen(c);
    double area = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    for (int y = 0; y < H; y += 3)
      for (int x = 0; x < W; x += 3) {
        double u = ((q.x - x - .5) * (r.y - y - .5) -
                    (q.y - y - .5) * (r.x - x - .5)) /
                   area;
        double v = ((r.x - x - .5) * (p.y - y - .5) -
                    (r.y - y - .5) * (p.x - x - .5)) /
                   area;
        double w = 1 - u - v;
        if (std::min({u, v, w}) > .00002)
          assert(std::abs(zbuffer[y * W + x] -
                          (u * p.iz + v * q.iz + w * r.iz)) < 1e-8);
        if (std::min({u, v, w}) < -.00002)
          assert(zbuffer[y * W + x] == 0);
      }
  }
  // Near-plane clipping and depth ordering remain correct with textured meshes.
  std::fill(pixels.begin(), pixels.end(), 0);
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  raster({-1, -1, 4}, {1, -1, 4}, {0, 1, 4}, 0xFF0000);
  raster({-1, -1, 2}, {1, -1, 2}, {0, 1, 2}, 0x00FF00);
  assert(((pixels[H / 2 * W + W / 2] >> 8) & 255) >
         ((pixels[H / 2 * W + W / 2] >> 16) & 255));
  for (auto page : {ScreenPage::Home, ScreenPage::Pause, ScreenPage::Buy,
                    ScreenPage::Settings, ScreenPage::Utility,
                    ScreenPage::Result, ScreenPage::Roster, ScreenPage::Vote}) {
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
  game.x = arena.showcase.x;
  game.y = arena.showcase.y;
  game.a = .2;
  game.pitch = -.03;
  game.weapon = 9;
  game.owned[9] = true;
  game.equipAnim = 0;
  game.bots = {target(45, 19)};
  render();
  if (argc > 1)
    imageFile(argv[1]);
  puts("PASS: three larger maps, 11-ballot voting and map activation, round "
       "cover, map/spawn separation, 16 weapons and paid secondaries, "
       "two-slot inventory, six-round halves, first-to-seven match, paid "
       "utility persistence, burst/pellet/automatic cadence, armor, "
       "drops/pickups, bomb/timers, harder AI, autonomous rounds, mouse menus, "
       "movement, licensed UV assets, 3D animations/viewmodels, transparent "
       "picture-in-picture scopes, recoil compensation/recovery, motion at "
       "different update rates, sniper crosshairs, texture filtering, camera "
       "billboards, scanline coverage, depth and UI layouts");
  return 0;
}
