#include <cassert>
void simulate(Game &g, double seconds) {
  for (int i = 0; i < int(std::ceil(seconds / .02)); i++)
    g.tick(.02);
}
int main(int argc, char **argv) {
  Game g;
  // Every spawn and both bomb sites are traversable and connected.
  assert(!arena.solid(g.x, g.y, .23));
  for (auto b : g.bots) {
    assert(!arena.solid(b.x, b.y, .23));
    assert(!g.route(g.x, g.y, b.x, b.y).empty());
  }
  assert(!g.route(3, 16, 27, 5).empty());
  assert(!g.route(3, 16, 27, 26).empty());
  g.move(-20, 0);
  assert(g.x > 1.1);
  g.x = 3;
  g.y = 16;
  // Economy, ownership and ammo isolation.
  assert(g.buy(1));
  assert(g.weapon == 1 && g.money == 800 && g.owned[1]);
  assert(!g.buy(3));
  assert(g.buy(4) && g.armor == 100);
  g.switchWeapon(0);
  assert(g.ammo() == 12);
  g.switchWeapon(1);
  assert(g.ammo() == 30);
  g.ammo() = 0;
  g.reserve[1] = 7;
  g.startReload();
  g.fire();
  assert(g.ammo() == 0);
  g.bots = {{3, 3}};
  g.bots[0].blind = 1000;
  simulate(g, 1);
  assert(g.ammo() == 0);
  simulate(g, 1.2);
  assert(g.ammo() == 7 && g.reserve[1] == 0);
  // Explicit head/body hits and wall/prop occlusion.
  g = Game();
  g.bots = {{5, 16}};
  g.x = 3;
  g.y = 16;
  g.pitch = -.12;
  g.fire();
  assert(g.bots[0].hp == 66 && g.ammo() == 11 && !g.tracers.empty());
  g = Game();
  g.bots = {{5, 16}};
  g.fire();
  assert(g.ended && g.won);
  g = Game();
  g.bots = {{9, 8}};
  g.x = 5;
  g.y = 8;
  g.a = 0;
  g.fire();
  assert(g.bots[0].hp == 100);
  assert(g.geometry({3, 8.5, 1}, direction(0), 20) < 2);
  // Player plant, interrupted plant, countdown win, and bot defuse.
  g = Game();
  g.bots = {{3, 3}};
  g.x = 27;
  g.y = 5;
  g.usingObjective = true;
  simulate(g, 1);
  assert(!g.planted && g.plantProgress > .9);
  g.move(.2, 0);
  assert(g.plantProgress == 0);
  g.walking = false;
  simulate(g, 3.1);
  assert(g.planted);
  g.bots.clear();
  simulate(g, 40.1);
  assert(g.ended && g.won && g.result == "BOMB DETONATED");
  g = Game();
  g.x = 3;
  g.y = 16;
  g.bots = {{27, 5}};
  g.planted = true;
  g.bombX = 27;
  g.bombY = 5;
  simulate(g, 5.2);
  assert(g.ended && !g.won && g.result == "DEFENDERS DEFUSED THE BOMB");
  // Defender role, bot planting, defuse interruption and successful defuse.
  g = Game();
  g.reset();
  assert(!g.attacker);
  g.x = 3;
  g.y = 16;
  g.bots = {{27, 5}};
  simulate(g, 3.2);
  assert(g.planted);
  g.bots.clear();
  g.x = 27;
  g.y = 5;
  g.usingObjective = true;
  simulate(g, 1);
  g.usingObjective = false;
  g.tick(.02);
  assert(g.defuseProgress == 0);
  g.usingObjective = true;
  simulate(g, 5.1);
  assert(g.ended && g.won && g.result == "BOMB DEFUSED");
  // Timer and economy result accounting cannot happen twice.
  g = Game();
  g.clock = .01;
  g.tick(.02);
  assert(g.ended && !g.won);
  int money = g.money;
  g.finish(false, "duplicate");
  assert(g.money == money && g.losses == 1);
  // Smoke blocks sight but not bullets; flash blinds; grenades consume stock.
  g = Game();
  g.bots = {{10, 16}};
  assert(g.visible(3, 16, 10, 16));
  g.smokes.push_back({6, 16, 12});
  assert(!g.visible(3, 16, 10, 16));
  assert(g.geometry({3, 16, 1.6}, direction(0), 7) > 6.9);
  g.smokes.clear();
  g.grenades.push_back({{8, 16, 1}, {0, 0, 0}, 2, .01});
  g.tick(.02);
  assert(g.bots[0].blind > 3);
  g.utility = 1;
  g.throwUtility();
  assert(g.utilities[1] == 0);
  g.throwUtility();
  assert(g.grenades.size() == 1);
  g.bots = {{3, 3}};
  g.bots[0].blind = 1000;
  simulate(g, 1.4);
  assert(!g.smokes.empty());
  g = Game();
  g.bots = {{10, 16}};
  g.smokes.push_back({10, 16, 12});
  g.grenades.push_back({{10, 16, .2}, {0, 0, 0}, 0, .01});
  g.tick(.02);
  assert(g.bots[0].hp <= 0 && g.kills == 1);
  // AI routes around buildings and responds to sound without seeing through
  // walls.
  g = Game();
  g.bots = {{6, 8}};
  g.x = 13;
  g.y = 8;
  assert(!g.visible(6, 8, 13, 8));
  g.hear(g.x, g.y, 20);
  assert(g.bots[0].knows);
  double bx = g.bots[0].x, by = g.bots[0].y;
  simulate(g, 1);
  assert(dist(bx, by, g.bots[0].x, g.bots[0].y) > .1);
  assert(!g.bots[0].path.empty());
  // Buying is restricted, switching cancels reloads, and defenders have valid
  // spawns.
  g = Game();
  g.time = 23;
  assert(!g.buy(1));
  g.time = 0;
  g.x = 15;
  assert(!g.buy(1));
  g.x = 3;
  assert(g.buy(1));
  g.ammo() = 1;
  g.startReload();
  g.switchWeapon(0);
  assert(g.reload == 0 && g.magazines[1] == 1);
  g.reset();
  for (auto b : g.bots)
    assert(!arena.solid(b.x, b.y, .23));
  // AI dry magazines enter a reload rather than shooting without ammo.
  g = Game();
  g.bots = {{5, 16}};
  g.bots[0].ammo = 0;
  g.bots[0].cooldown = 0;
  g.tick(.02);
  assert(g.bots[0].reload > 0);
  // Render actual triangles and verify depth and perspective changes.
  game = Game();
  game.buy(1);
  aiming = false;
  render();
  auto initial = pixels;
  assert(*std::max_element(zbuffer.begin(), zbuffer.end()) > 0);
  game.pitch = .35;
  game.a = .4;
  render();
  assert(initial != pixels);
  std::fill(pixels.begin(), pixels.end(), 0);
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  raster({-1, -1, 4}, {1, -1, 4}, {0, 1, 4}, 0xFF0000);
  raster({-1, -1, 2}, {1, -1, 2}, {0, 1, 2}, 0x00FF00);
  assert(((pixels[H / 2 * W + W / 2] >> 8) & 255) >
         ((pixels[H / 2 * W + W / 2] >> 16) & 255));
  game = Game();
  game.smokes = {{game.x, game.y, 12}, {8, 16, 12}};
  game.blasts = {{{6, 16, 1}, .3}};
  game.grenades = {{{5, 16, 1}, {0, 0, 0}, 1, 1}};
  game.planted = true;
  game.bombX = 6;
  game.bombY = 16;
  game.flash = 1;
  game.pitch = 1.1;
  render();
  game.pitch = -1.1;
  aiming = true;
  game.weapon = 3;
  render();
  aiming = false;
  game = Game();
  game.buy(1);
  game.x = 13;
  game.y = 14;
  game.a = .25;
  game.pitch = -.06;
  render();
  if (argc > 1) {
    std::ofstream f(argv[1], std::ios::binary);
    f << "P6\n" << W << " " << H << "\n255\n";
    for (auto c : pixels) {
      char bytes[] = {char(c >> 16), char(c >> 8), char(c)};
      f.write(bytes, 3);
    }
  }
  puts("PASS: map connectivity, collision, economy, guns, headshots, cover, "
       "finite reloads, plant/defuse, timers, round accounting, smoke, flash, "
       "frag, AI navigation/hearing, 3D rendering, depth buffer, camera");
}
