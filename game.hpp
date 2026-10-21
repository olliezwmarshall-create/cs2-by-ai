#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <queue>
#include <string>
#include <vector>
constexpr double PI = 3.141592653589793;
constexpr int W = 960, H = 600, MAP = 96;
struct Vec {
  double x = 0, y = 0, z = 0;
  Vec operator+(Vec b) const { return {x + b.x, y + b.y, z + b.z}; }
  Vec operator-(Vec b) const { return {x - b.x, y - b.y, z - b.z}; }
  Vec operator*(double s) const { return {x * s, y * s, z * s}; }
};
double length(Vec v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
double dist(double x, double y, double xx, double yy) {
  return std::hypot(x - xx, y - yy);
}
double wrap(double a) { return atan2(sin(a), cos(a)); }
Vec HOME_T{5, 40, 0}, HOME_CT{75, 40, 0}, SITE_A{70, 14, 0}, SITE_B{70, 66, 0};
constexpr int WEAPON_COUNT = 16;
bool secondaryGun(int gun) { return gun == 0 || (gun >= 5 && gun <= 7); }
bool scopedGun(int gun) { return gun == 3 || gun == 4 || gun == 14; }
bool boltGun(int gun) { return gun == 3 || gun == 14; }
struct Box {
  double x, y, z, w, d, h;
  uint32_t color;
  bool cover = true;
};
struct RoundSolid {
  double x, y, r, h;
  bool dome = false;
  uint32_t color = 0x829599;
};
const std::array<const char *, 3> MAP_NAMES = {
    {"SHIPYARD", "OLD QUARTER", "RADAR STATION"}};
const std::array<int, 3> MAP_SIZES = {{80, 88, 96}};
struct Arena {
  int id = 0, size = 80, revision = 0;
  std::array<std::string, MAP> cells;
  std::array<float, MAP * MAP> heights{};
  std::vector<Box> props, obstacles, structures, extra;
  std::vector<RoundSolid> rounds;
  std::vector<Vec> lamps, bottles, trucks, arches;
  Vec homeT, homeCT, siteA, siteB, showcase;
  double showcaseAngle = .3;
  Arena() { load(0); }
  void load(int map) {
    id = std::clamp(map, 0, 2);
    size = MAP_SIZES[id];
    revision++;
    props.clear();
    obstacles.clear();
    structures.clear();
    rounds.clear();
    extra.clear();
    lamps.clear();
    bottles.clear();
    trucks.clear();
    arches.clear();
    heights.fill(0);
    homeT = {5, double(size / 2), 0};
    homeCT = {double(size - 5), double(size / 2), 0};
    siteA = {double(size - 10), 14, 0};
    siteB = {double(size - 10), double(size - 14), 0};
    for (int y = 0; y < MAP; y++) {
      cells[y] = std::string(MAP, '#');
      for (int x = 0; x < MAP; x++) {
        bool edge = x == 0 || y == 0 || x >= size - 1 || y >= size - 1;
        cells[y][x] = edge ? '#' : '.';
        if (edge)
          heights[y * MAP + x] = 3.2;
      }
    }
    auto building = [&](int x0, int y0, int x1, int y1, double h) {
      for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++) {
          cells[y][x] = '#';
          heights[y * MAP + x] = float(h);
        }
      structures.push_back({double(x0), double(y0), 0, double(x1 - x0 + 1),
                            double(y1 - y0 + 1), h,
                            id == 1   ? 0xC8B18Au
                            : id == 2 ? 0x8D9FA6u
                                      : 0xA8B7B5u});
    };
    int mid = size / 2;
    building(8, mid - 3, 10, mid + 3, 3.8);
    building(size - 12, mid - 4, size - 10, mid + 4, 4.4);
    if (id == 0) {
      building(16, 10, 26, mid - 10, 5.2);
      building(16, mid + 10, 26, size - 11, 5.2);
      building(36, 10, 44, mid - 13, 5.8);
      building(36, mid + 13, 44, size - 11, 5.8);
      building(31, mid - 4, 45, mid + 3, 6.1);
      building(size - 23, mid - 13, size - 17, mid - 7, 4.8);
      building(size - 23, mid + 7, size - 17, mid + 14, 4.8);
      showcase = {24, 31, 0};
      showcaseAngle = .3;
    }
    if (id == 1) {
      building(16, 12, 27, 30, 5.8);
      building(16, mid + 12, 27, size - 13, 5.8);
      building(37, 8, 45, mid - 13, 6.4);
      building(35, mid + 13, 45, size - 9, 6.4);
      building(33, mid - 5, 46, mid + 4, 5.4);
      building(size - 24, 24, size - 18, 31, 5.5);
      building(size - 25, size - 31, size - 18, size - 23, 5.5);
      showcase = {28, 34, 0};
      showcaseAngle = .2;
      arches = {{30, double(mid - 10), 0}, {49, double(mid + 10), 0}};
    }
    if (id == 2) {
      building(17, 12, 29, mid - 15, 5.2);
      building(17, mid + 15, 29, size - 13, 5.2);
      building(39, 9, 49, mid - 17, 4.7);
      building(39, mid + 17, 49, size - 10, 4.7);
      building(36, mid - 5, 52, mid + 4, 5.9);
      building(size - 27, mid - 15, size - 20, mid - 8, 4.8);
      building(size - 27, mid + 8, size - 20, mid + 15, 4.8);
      showcase = {50.5, 17, 0};
      showcaseAngle = -.8;
    }
    props = {{29, 18, 0, 1.6, 2, 1.4, 0x8E7957},
             {29, double(size - 19), 0, 1.6, 2, 1.4, 0x8E7957},
             {double(mid + 7), 18, 0, 2, 1.5, 1.3, 0x718A8A},
             {double(mid + 8), double(size - 19), 0, 2, 1.5, 1.3, 0x718A8A},
             {siteA.x + 2, siteA.y - 5, 0, 1.5, 1.5, 1.2, 0x9B8059},
             {siteB.x + 2, siteB.y + 4, 0, 1.5, 1.5, 1.2, 0x9B8059},
             {19, double(mid - 5), 0, 2.5, 1.5, 1.2, 0x93744A},
             {23, double(mid + 5), 0, 2.5, 1.5, 1.2, 0x93744A}};
    for (Vec p :
         std::array<Vec, 4>{{{31, 10, 0},
                             {31, double(size - 11), 0},
                             {double(size - 16), 20, 0},
                             {double(size - 16), double(size - 21), 0}}})
      rounds.push_back({p.x, p.y, id == 2 ? 2. : 1.3, id == 1 ? 1.5 : 2.8,
                        id != 1, id == 1 ? 0xBCB09Au : 0x789395u});
    if (id == 1) {
      rounds.push_back(
          {double(mid + 8), double(mid + 1), 2.2, .6, false, 0x9BADA6});
    }
    if (id == 2) {
      rounds.push_back({double(mid + 7), 8, 3., 2.2, true, 0xB7C6C5});
      rounds.push_back(
          {double(mid + 7), double(size - 9), 3., 2.2, true, 0xB7C6C5});
    }
    lamps = {{15, double(mid - 8), 0},
             {30, double(mid + 9), 0},
             {double(size - 25), 19, 0},
             {double(size - 16), double(size - 19), 0}};
    bottles = {{props[0].x + .4, props[0].y + .4, props[0].h},
               {props[4].x + .4, props[4].y + .4, props[4].h}};
    trucks = {{double(mid - 10), double(mid - 10), 0},
              {double(size - 17), double(size - 13), 0}};
    for (auto p : trucks)
      extra.push_back({p.x - 1.3, p.y - 2.27, 0, 2.6, 4.54, 2.4, 0});
    for (auto p : arches)
      for (int side : {-1, 1})
        extra.push_back({p.x - .25, p.y + side * 2.5 - .25, 0, .5, .5, 3.8, 0});
    for (int y = 0; y < size; y++)
      for (int x = 0; x < size; x++)
        if (cells[y][x] == '#')
          obstacles.push_back(
              {double(x), double(y), 0, 1, 1, wallHeight(x, y), 0});
    obstacles.insert(obstacles.end(), props.begin(), props.end());
    obstacles.insert(obstacles.end(), extra.begin(), extra.end());
  }
  double wallHeight(int x, int y) const {
    return x < 0 || y < 0 || x >= size || y >= size ? 3.2
                                                    : heights[y * MAP + x];
  }
  bool wall(double x, double y) const {
    return x < 0 || y < 0 || x >= size || y >= size ||
           cells[int(y)][int(x)] == '#';
  }
  bool solid(double x, double y, double margin = 0) const {
    if (wall(x - margin, y - margin) || wall(x + margin, y - margin) ||
        wall(x - margin, y + margin) || wall(x + margin, y + margin))
      return true;
    for (const auto &list : {&props, &extra})
      for (auto p : *list)
        if (p.cover && x + margin > p.x && x - margin < p.x + p.w &&
            y + margin > p.y && y - margin < p.y + p.d)
          return true;
    for (auto c : rounds)
      if (dist(x, y, c.x, c.y) < c.r + margin)
        return true;
    return false;
  }
  double heightAt(double x, double y) const {
    if (x < 0 || y < 0 || x >= size || y >= size)
      return 0;
    double h = wall(x, y) ? wallHeight(int(x), int(y)) : 0;
    for (const auto &list : {&props, &extra})
      for (auto p : *list)
        if (x > p.x && x < p.x + p.w && y > p.y && y < p.y + p.d)
          h = std::max(h, p.z + p.h);
    for (auto c : rounds) {
      double d = dist(x, y, c.x, c.y);
      if (d < c.r)
        h = std::max(h, c.h + (c.dome ? sqrt(c.r * c.r - d * d) : 0));
    }
    return h;
  }
  Vec safe(Vec p) const {
    if (!solid(p.x, p.y, .3))
      return p;
    for (int radius = 1; radius < 9; radius++)
      for (int dy = -radius; dy <= radius; dy++)
        for (int dx = -radius; dx <= radius; dx++) {
          Vec q = {floor(p.x) + dx + .5, floor(p.y) + dy + .5, 0};
          if (!solid(q.x, q.y, .3))
            return q;
        }
    return homeT;
  }
};
Arena arena;
void selectMap(int id) {
  arena.load(id);
  HOME_T = arena.homeT;
  HOME_CT = arena.homeCT;
  SITE_A = arena.siteA;
  SITE_B = arena.siteB;
}
bool solid(double x, double y) { return arena.solid(x, y); }
double boxHit(Vec o, Vec dir, Box b) {
  double lo = 0, hi = 1000;
  double a[] = {o.x, o.y, o.z}, v[] = {dir.x, dir.y, dir.z},
         mn[] = {b.x, b.y, b.z}, mx[] = {b.x + b.w, b.y + b.d, b.z + b.h};
  for (int i = 0; i < 3; i++) {
    if (std::abs(v[i]) < 1e-8) {
      if (a[i] < mn[i] || a[i] > mx[i])
        return 1000;
    } else {
      double p = (mn[i] - a[i]) / v[i], q = (mx[i] - a[i]) / v[i];
      if (p > q)
        std::swap(p, q);
      lo = std::max(lo, p);
      hi = std::min(hi, q);
      if (lo > hi)
        return 1000;
    }
  }
  return lo;
}
double roundHit(Vec o, Vec d, RoundSolid c) {
  double best = 1000, ox = o.x - c.x, oy = o.y - c.y, A = d.x * d.x + d.y * d.y,
         B = 2 * (ox * d.x + oy * d.y), C = ox * ox + oy * oy - c.r * c.r;
  if (A > 1e-9) {
    double disc = B * B - 4 * A * C;
    if (disc >= 0)
      for (double t :
           {(-B - sqrt(disc)) / (2 * A), (-B + sqrt(disc)) / (2 * A)})
        if (t >= 0 && o.z + d.z * t >= 0 && o.z + d.z * t <= c.h)
          best = std::min(best, t);
  }
  if (std::abs(d.z) > 1e-9)
    for (double z : {0., c.h}) {
      double t = (z - o.z) / d.z;
      if (t >= 0 && dist(o.x + d.x * t, o.y + d.y * t, c.x, c.y) <= c.r)
        best = std::min(best, t);
    }
  if (c.dome) {
    Vec v = o - Vec{c.x, c.y, c.h};
    double q = d.x * v.x + d.y * v.y + d.z * v.z,
           disc = q * q - (v.x * v.x + v.y * v.y + v.z * v.z - c.r * c.r);
    if (disc >= 0)
      for (double t : {-q - sqrt(disc), -q + sqrt(disc)})
        if (t >= 0 && o.z + d.z * t >= c.h)
          best = std::min(best, t);
  }
  return best;
}
Vec direction(double a, double pitch = 0) {
  return {cos(a) * cos(pitch), sin(a) * cos(pitch), sin(pitch)};
}
enum Sound {
  Rifle,
  EnemyShot,
  MagOut,
  MagIn,
  Bolt,
  Step,
  Hit,
  Empty,
  Explosion,
  BombBeep,
  WinSound
};
struct Weapon {
  const char *name;
  int capacity, damage, price;
  double delay, reload, spread, range;
  bool automatic = false;
};
const std::array<Weapon, WEAPON_COUNT> weapons = {
    {{"SERVICE PISTOL", 12, 34, 0, .22, 1.3, .008, 32, false},
     {"AUTO CARBINE", 30, 35, 2400, .065, 2.1, .012, 64, true},
     {"RAPID SMG", 32, 24, 1200, .05, 1.65, .023, 35, true},
     {"BOLT SNIPER", 5, 110, 3500, 1.2, 2.6, .001, 96, false},
     {"SCOUT DMR", 10, 72, 2800, .28, 2.2, .004, 85, false},
     {"HEAVY PISTOL", 7, 65, 700, .3, 1.7, .01, 48, false},
     {"COMPACT PISTOL", 20, 24, 400, .14, 1.25, .013, 28, false},
     {"BURST PISTOL", 18, 28, 650, .065, 1.5, .018, 32, false},
     {"SUPPRESSED RIFLE", 25, 36, 2900, .085, 2.2, .008, 65, true},
     {"BATTLE RIFLE", 20, 48, 3000, .11, 2.4, .014, 75, true},
     {"BULLPUP RIFLE", 30, 33, 2700, .075, 2.2, .01, 65, true},
     {"PUMP SHOTGUN", 8, 13, 1600, .85, 2.8, .16, 18, false},
     {"AUTO SHOTGUN", 7, 10, 2400, .28, 2.8, .18, 16, true},
     {"BELT MACHINE GUN", 80, 30, 4500, .085, 4.2, .035, 65, true},
     {"LIGHT SNIPER", 10, 80, 2000, 1., 2.3, .002, 90, false},
     {"COMPACT PDW", 50, 22, 1900, .055, 2.4, .024, 42, true}}};
struct Bot {
  double x, y;
  int hp = 100;
  double cooldown = 1, flash = 0;
  double alert = 0, lastX = 0, lastY = 0, repath = 0, blind = 0, reload = 0,
         plant = 0;
  int ammo = 20;
  bool knows = false;
  std::vector<int> path{};
  bool terrorist = false, usedUtility = false;
  int gun = 1, frags = 0, role = 0, target = -2;
  double facing = 0, reaction = .25, suppression = 0, communicate = 0,
         coverTime = 0;
  double coverX = 0, coverY = 0, senseTimer = 0, senseX = 0, senseY = 0;
  std::string state = "ADVANCE";
  double deathTime = -1, walkPhase = 0, locomotion = 0;
  int armor = 70;
};
struct Tracer {
  Vec from, to;
  double life;
  bool enemy;
};
struct Grenade {
  Vec p, v;
  int kind;
  double fuse = 1.2;
  int owner = -1;
};
struct Cloud {
  double x, y, time;
};
struct Blast {
  Vec p;
  double time;
};
struct Particle {
  Vec p, v;
  double life;
  uint32_t color;
};
struct Impact {
  Vec p;
  double life;
};
struct DroppedWeapon {
  Vec p, v;
  double angle;
  int gun, ammo, reserve;
};
struct Game {
  double x = 3, y = 16, a = 0, pitch = 0, shot = 0, muzzle = 0, reload = 0,
         time = 0, hurt = 0, hit = 0, recoil = 0, steps = 0, flash = 0,
         clock = 150, plantProgress = 0, defuseProgress = 0, bombClock = 40,
         bombX = 0, bombY = 0, beep = 0, boltCycle = 0, freeze = 0, vx = 0,
         vy = 0, stance = 0, jumpHeight = 0, jumpVelocity = 0;
  bool sprinting = false;
  int hp = 100, armor = 0, kills = 0, round = 0, weapon = 0, money = 3200,
      wins = 0, losses = 0, spectator = 0;
  bool won = false, ended = false, attacker = true, planted = false,
       usingObjective = false, buyOpen = false, walking = false, ads = false;
  std::string result;
  bool matchOver = false, matchWon = false, startAttacker = true,
       playerDropped = false, utilityEquipped = false;
  double visualTime = 0, equipAnim = 0;
  int primary = -1, secondary = 0, burstRemaining = 0;
  std::array<bool, WEAPON_COUNT> owned{{true}};
  std::array<int, WEAPON_COUNT> magazines{}, reserve{};
  std::array<int, 3> utilities{{0, 0, 0}};
  int utility = 0;
  uint32_t random = 913;
  std::vector<Bot> bots;
  std::vector<Tracer> tracers;
  std::vector<Sound> sounds;
  std::vector<Grenade> grenades;
  std::vector<Cloud> smokes;
  std::vector<Blast> blasts;
  std::vector<Particle> particles;
  std::vector<Impact> impacts;
  std::vector<DroppedWeapon> drops;
  std::vector<std::string> feed;
  Game() { reset(); }
  int &ammo() { return magazines[weapon]; }
  const Weapon &gun() const { return weapons[weapon]; }
  double rng() {
    random = random * 1664525 + 1013904223;
    return double(random >> 8) / 16777216.;
  }
  void reset() {
    if (matchOver)
      return;
    if (round == 0)
      startAttacker = attacker;
    round++;
    attacker = startAttacker ^ (((round - 1) / 6) % 2 != 0);
    x = attacker ? HOME_T.x : HOME_CT.x;
    y = attacker ? HOME_T.y : HOME_CT.y;
    playerDropped = false;
    utilityEquipped = false;
    burstRemaining = 0;
    equipAnim = 0;
    drops.clear();
    a = attacker ? -PI / 2 : PI / 2;
    pitch = shot = muzzle = reload = time = hurt = hit = recoil = steps =
        flash = plantProgress = defuseProgress = beep = boltCycle = 0;
    freeze = vx = vy = stance = jumpHeight = jumpVelocity = 0;
    sprinting = false;
    particles.clear();
    impacts.clear();
    spectator = 0;
    feed.clear();
    hp = 100;
    kills = 0;
    clock = 150;
    bombClock = 40;
    won = ended = planted = usingObjective = buyOpen = walking = ads = false;
    result.clear();
    bots.clear();
    std::array<Vec, 5> tSpawn = {
        {HOME_T + Vec{0, -4, 0}, HOME_T + Vec{0, 3, 0}, HOME_T + Vec{0, 6, 0},
         HOME_T + Vec{-2, -1, 0}, HOME_T + Vec{-2, 2, 0}}};
    std::array<Vec, 5> ctSpawn = {
        {SITE_A + Vec{3, 0, 0}, SITE_B + Vec{3, 0, 0}, HOME_CT + Vec{-1, -7, 0},
         HOME_CT + Vec{0, 7, 0}, HOME_CT + Vec{1, 2, 0}}};
    for (int team = 0; team < 2; team++)
      for (int i = 0; i < 5; i++) {
        Vec p = arena.safe(team ? tSpawn[i] : ctSpawn[i]);
        Bot bot{p.x, p.y};
        bot.terrorist = team;
        bot.role = i;
        bot.gun = i == 4 ? 3 : i == 1 ? 15 : i == 2 ? 8 : i == 3 ? 9 : 1;
        bot.ammo = weapons[bot.gun].capacity;
        bot.facing = team ? 0 : PI;
        bot.cooldown = .5 + i * .12;
        bots.push_back(bot);
      }
    tracers.clear();
    sounds.clear();
    grenades.clear();
    smokes.clear();
    blasts.clear();
    for (int i = 0; i < WEAPON_COUNT; i++) {
      magazines[i] = weapons[i].capacity;
      reserve[i] = weapons[i].capacity * 3;
    }
    if (secondary < 0) {
      secondary = 0;
      owned[0] = true;
    }
    if (!owned[weapon])
      weapon = secondary;
  }
  void finish(bool victory, const std::string &why) {
    if (ended)
      return;
    ended = true;
    won = victory;
    result = why;
    if (victory) {
      wins++;
      money += 3000;
    } else {
      losses++;
      money += 1900;
    }
    if (hp <= 0) {
      owned.fill(false);
      owned[0] = true;
      primary = -1;
      secondary = 0;
      armor = 0;
      utilities = {0, 0, 0};
    }
    matchOver = wins >= 7 || losses >= 7;
    matchWon = wins >= 7;
    money = std::min(16000, money);
    sounds.push_back(victory ? WinSound : Explosion);
  }
  double ray(double ox, double oy, double angle, double limit = 64) const {
    for (double d = .04; d < limit; d += .06)
      if (arena.solid(ox + cos(angle) * d, oy + sin(angle) * d))
        return d;
    return limit;
  }
  double geometry(Vec origin, Vec dir, double limit = 64) const {
    double best = limit;
    if (dir.z < 0)
      best = std::min(best, -origin.z / dir.z);
    for (const auto &obstacle : arena.obstacles)
      best = std::min(best, boxHit(origin, dir, obstacle));
    for (auto c : arena.rounds)
      best = std::min(best, roundHit(origin, dir, c));
    return best;
  }
  bool obscured(double ox, double oy, double tx, double ty) const {
    double dx = tx - ox, dy = ty - oy, den = dx * dx + dy * dy;
    for (auto c : smokes) {
      double t =
          den ? std::clamp(((c.x - ox) * dx + (c.y - oy) * dy) / den, 0., 1.)
              : 0;
      if (dist(c.x, c.y, ox + t * dx, oy + t * dy) < 2.4)
        return true;
    }
    return false;
  }
  bool clearLine(double ox, double oy, double tx, double ty) const {
    double d = dist(ox, oy, tx, ty);
    Vec dir = {(tx - ox) / std::max(d, .001), (ty - oy) / std::max(d, .001), 0};
    return geometry({ox, oy, 1.3}, dir, d + .1) > d - .15;
  }
  bool visible(double ox, double oy, double tx, double ty) const {
    return clearLine(ox, oy, tx, ty) && !obscured(ox, oy, tx, ty);
  }
  void move(double dx, double dy) {
    if (ended || hp <= 0 || freeze > 0)
      return;
    double ox = x, oy = y;
    int n = std::max(1, int(hypot(dx, dy) / .08) + 1);
    for (int i = 0; i < n; i++) {
      if (!arena.solid(x + dx / n, y, .22))
        x += dx / n;
      if (!arena.solid(x, y + dy / n, .22))
        y += dy / n;
    }
    double distance = dist(x, y, ox, oy);
    walking = distance > .001;
    steps += distance;
    if (distance > .005)
      plantProgress = defuseProgress = 0;
    if (steps > .95) {
      steps = 0;
      sounds.push_back(Step);
    }
  }
  void locomotion(double dt, double forward, double side, bool sprint,
                  bool crouch) {
    if (ended || hp <= 0 || freeze > 0) {
      vx = vy = 0;
      walking = false;
      return;
    }
    stance += std::clamp((crouch ? 1. : 0.) - stance, -dt * 6, dt * 6);
    sprinting = sprint && !ads && !crouch;
    double speed = crouch ? 1.25 : sprinting ? 3.7 : ads ? 1.45 : 2.6;
    double n = std::max(1., hypot(forward, side));
    forward *= speed / n;
    side *= speed / n;
    double tx = cos(a) * forward - sin(a) * side,
           ty = sin(a) * forward + cos(a) * side;
    double acceleration = (forward != 0 || side != 0) ? 18 : 25;
    auto approach = [&](double value, double target) {
      return value +
             std::clamp(target - value, -acceleration * dt, acceleration * dt);
    };
    vx = approach(vx, tx);
    vy = approach(vy, ty);
    move(vx * dt, vy * dt);
  }
  void jump() {
    if (hp > 0 && !ended && freeze <= 0 && jumpHeight <= 0 && stance < .2)
      jumpVelocity = 3.5;
  }
  bool buyAllowed() const {
    return hp > 0 && time < 22 &&
           dist(x, y, attacker ? HOME_T.x : HOME_CT.x,
                attacker ? HOME_T.y : HOME_CT.y) < 5 &&
           !ended;
  }
  bool buyGun(int gun) {
    if (gun < 0 || gun >= WEAPON_COUNT || boltCycle > 0 || !buyAllowed() ||
        owned[gun] || money < weapons[gun].price)
      return false;
    int previous = secondaryGun(gun) ? secondary : primary;
    if (previous >= 0 && owned[previous])
      dropGun(previous);
    money -= weapons[gun].price;
    owned[gun] = true;
    if (secondaryGun(gun))
      secondary = gun;
    else
      primary = gun;
    magazines[gun] = weapons[gun].capacity;
    reserve[gun] = weapons[gun].capacity * 3;
    switchWeapon(gun);
    return true;
  }
  bool buyUtility(int type) {
    static constexpr int cost[3] = {300, 300, 200};
    if (type < 0 || type >= 3 || !buyAllowed() || money < cost[type] ||
        utilities[type] >= (type == 2 ? 2 : 1))
      return false;
    money -= cost[type];
    utilities[type]++;
    return true;
  }
  bool buy(int item) {
    if (item >= 100 && item < 100 + WEAPON_COUNT)
      return buyGun(item - 100);
    if (item >= 200 && item < 203)
      return buyUtility(item - 200);
    if (item == 4 && buyAllowed() && money >= 650 && armor < 100) {
      money -= 650;
      armor = 100;
      return true;
    }
    return false;
  }
  void dropGun(int gun) {
    if (gun < 0 || gun >= WEAPON_COUNT || !owned[gun])
      return;
    drops.push_back({{x, y, 1.1},
                     {cos(a) * 1.4, sin(a) * 1.4, 1},
                     a,
                     gun,
                     magazines[gun],
                     reserve[gun]});
    owned[gun] = false;
    if (primary == gun)
      primary = -1;
    if (secondary == gun)
      secondary = -1;
    if (weapon == gun) {
      reload = boltCycle = shot = 0;
      burstRemaining = 0;
      utilityEquipped = false;
      if (primary >= 0)
        weapon = primary;
      else if (secondary >= 0)
        weapon = secondary;
    }
  }
  int nearbyDrop() const {
    int id = -1;
    double best = 1.8;
    for (size_t i = 0; i < drops.size(); i++) {
      double d = dist(x, y, drops[i].p.x, drops[i].p.y);
      if (d < best && clearLine(x, y, drops[i].p.x, drops[i].p.y)) {
        best = d;
        id = int(i);
      }
    }
    return id;
  }
  bool pickup() {
    int id = nearbyDrop();
    if (hp <= 0 || ended || boltCycle > 0 || id < 0)
      return false;
    auto drop = drops[id];
    drops.erase(drops.begin() + id);
    int previous = secondaryGun(drop.gun) ? secondary : primary;
    if (previous >= 0 && owned[previous])
      dropGun(previous);
    owned[drop.gun] = true;
    if (secondaryGun(drop.gun))
      secondary = drop.gun;
    else
      primary = drop.gun;
    magazines[drop.gun] = drop.ammo;
    reserve[drop.gun] = drop.reserve;
    switchWeapon(drop.gun);
    return true;
  }
  void playerDeath() {
    if (hp > 0 || playerDropped)
      return;
    playerDropped = true;
    for (int i = 0; i < WEAPON_COUNT; i++)
      if (owned[i])
        dropGun(i);
    utilities = {0, 0, 0};
    utilityEquipped = false;
    armor = 0;
  }
  void switchWeapon(int i) {
    if (i < 0 || i >= WEAPON_COUNT || !owned[i] || ended || hp <= 0 ||
        boltCycle > 0)
      return;
    weapon = i;
    utilityEquipped = false;
    burstRemaining = 0;
    equipAnim = .3;
    reload = 0;
    shot = .2;
    plantProgress = defuseProgress = 0;
  }
  void startReload() {
    if (hp <= 0 || ended || boltCycle > 0 || reload > 0 ||
        ammo() == gun().capacity || reserve[weapon] <= 0)
      return;
    reload = gun().reload;
    utilityEquipped = false;
    burstRemaining = 0;
    sounds.push_back(MagOut);
    plantProgress = defuseProgress = 0;
  }
  void hear(double ox, double oy, double radius) {
    for (auto &b : bots)
      if (b.hp > 0 && b.terrorist != attacker &&
          dist(b.x, b.y, ox, oy) < radius) {
        b.knows = true;
        b.lastX = ox;
        b.lastY = oy;
        b.alert = 5;
        b.repath = 0;
      }
  }
  void fire() {
    if (hp <= 0 || ended || freeze > 0 || buyOpen || shot > .000001 ||
        boltCycle > 0 || reload > 0 || usingObjective || !owned[weapon])
      return;
    if (ammo() <= 0) {
      shot = .25;
      burstRemaining = 0;
      sounds.push_back(Empty);
      return;
    }
    ammo()--;
    shot = std::max(-.005, shot) + gun().delay;
    if (boltGun(weapon) || weapon == 11)
      boltCycle = weapon == 11 ? .55 : .8;
    if (burstRemaining > 0) {
      burstRemaining--;
      if (burstRemaining == 0)
        shot += .2;
    }
    muzzle = .055;
    recoil = std::min(2., recoil + .65);
    sounds.push_back(Rifle);
    hear(x, y, weapon == 8 ? 12 : 35);
    int pellets = weapon == 11 || weapon == 12 ? 8 : 1;
    double spread = gun().spread *
                    (jumpHeight > .03 ? 5
                     : walking        ? 3
                     : stance > .5    ? .65
                                      : 1) *
                    (ads ? .4 : 1) * (1 + recoil * .4);
    Vec origin = {x, y, 1.6 - stance * .5 + jumpHeight};
    for (int pellet = 0; pellet < pellets; pellet++) {
      Vec dir =
          direction(a + (rng() - .5) * spread, pitch + (rng() - .5) * spread);
      double nearest = geometry(origin, dir, gun().range);
      int target = -1;
      bool head = false;
      for (size_t i = 0; i < bots.size(); i++) {
        auto &b = bots[i];
        if (b.hp <= 0)
          continue;
        double body = boxHit(origin, dir,
                             {b.x - .24, b.y - .24, .2, .48, .48, 1.23, 0}),
               skull = boxHit(origin, dir,
                              {b.x - .17, b.y - .17, 1.43, .34, .34, .37, 0}),
               d = std::min(body, skull);
        if (d < nearest) {
          nearest = d;
          target = int(i);
          head = skull < body;
        }
      }
      tracers.push_back({origin, origin + dir * nearest, .09, false});
      if (target >= 0 && bots[target].terrorist != attacker) {
        damageBot(target, gun().damage * (head ? 3 : 1), -1);
        hit = .2;
        sounds.push_back(Hit);
      } else if (target < 0) {
        Vec at = origin + dir * std::max(0., nearest - .025);
        impacts.push_back({at, 20});
        if (impacts.size() > 64)
          impacts.erase(impacts.begin());
        for (int i = 0; i < 3; i++)
          particles.push_back(
              {at,
               {(rng() - .5) * 1.5, (rng() - .5) * 1.5, rng() * 1.4},
               .25,
               0xFFC983});
      }
    }
    particles.push_back({origin + Vec{-sin(a) * .2, cos(a) * .2, -.12},
                         {-sin(a) * 1.7, cos(a) * 1.7, 1.1},
                         1.,
                         0xC7A65C});
    checkElimination();
  }
  void trigger(bool held, bool pressed) {
    if (utilityEquipped) {
      if (pressed)
        throwUtility();
      return;
    }
    if (weapon == 7) {
      if (pressed && shot <= 0 && reload <= 0)
        burstRemaining = 3;
      if (burstRemaining > 0)
        fire();
    } else if (gun().automatic ? held : pressed)
      fire();
  }
  int alive(bool terrorists) const {
    int n = (hp > 0 && attacker == terrorists) ? 1 : 0;
    for (auto &b : bots)
      if (b.hp > 0 && b.terrorist == terrorists)
        n++;
    return n;
  }
  std::string botName(int i) const {
    return std::string(bots[i].terrorist ? "T" : "CT") + "-" +
           std::to_string(bots[i].role + 1);
  }
  void log(const std::string &text) {
    feed.push_back(text);
    if (feed.size() > 5)
      feed.erase(feed.begin());
  }
  void finishTeam(bool terroristWin, const std::string &why) {
    finish(terroristWin == attacker, why);
  }
  bool checkElimination() {
    if (ended)
      return true;
    if (alive(false) == 0) {
      finishTeam(true, "T WIN - ALL CT ELIMINATED");
      return true;
    }
    if (alive(true) == 0 && !planted) {
      finishTeam(false, "CT WIN - ALL T ELIMINATED");
      return true;
    }
    return false;
  }
  void damageBot(int i, int damage, int source) {
    auto &b = bots[i];
    if (b.hp <= 0)
      return;
    if (b.armor > 0) {
      int gun = source == -1 ? weapon : source >= 0 ? bots[source].gun : 1;
      damage =
          int(damage * ((scopedGun(gun) || gun == 5 || gun == 9) ? .96 : .78));
      b.armor = std::max(0, b.armor - damage / 3);
    }
    b.hp = std::max(0, b.hp - damage);
    b.suppression = 1.2;
    if (b.hp == 0) {
      b.plant = 0;
      b.deathTime = visualTime;
      drops.push_back({{b.x, b.y, 1.1},
                       {cos(b.facing), sin(b.facing), 1},
                       b.facing,
                       b.gun,
                       b.ammo,
                       weapons[b.gun].capacity * 2});
      std::string from = source == -1  ? "YOU"
                         : source >= 0 ? botName(source)
                                       : "BLAST";
      log(from + " > " + botName(i));
      if (source == -1 && b.terrorist != attacker) {
        kills++;
        money = std::min(16000, money + 300);
      } else if (source >= 0)
        bots[source].frags++;
    }
  }
  int spectating() const {
    if (hp > 0)
      return -1;
    std::vector<int> ids;
    for (size_t i = 0; i < bots.size(); i++)
      if (bots[i].hp > 0 && bots[i].terrorist == attacker)
        ids.push_back(int(i));
    return ids.empty() ? -1 : ids[spectator % ids.size()];
  }
  Vec eye() const {
    int i = spectating();
    return i >= 0 ? Vec{bots[i].x, bots[i].y, 1.6}
                  : Vec{x, y, 1.6 - stance * .5 + jumpHeight};
  }
  double viewAngle() const {
    int i = spectating();
    return i >= 0 ? bots[i].facing : a;
  }
  void tickBots(double dt);
  int site(double xx, double yy) const {
    if (dist(xx, yy, SITE_A.x, SITE_A.y) < 4)
      return 1;
    if (dist(xx, yy, SITE_B.x, SITE_B.y) < 4)
      return 2;
    return 0;
  }
  void throwUtility() {
    if (ended || freeze > 0 || hp <= 0 || buyOpen || utilities[utility] <= 0)
      return;
    utilities[utility]--;
    utilityEquipped = false;
    Vec dir = direction(a, pitch);
    grenades.push_back({{x, y, 1.4},
                        {dir.x * 8, dir.y * 8, std::max(1., dir.z * 6 + 3)},
                        utility,
                        1.25});
    plantProgress = defuseProgress = 0;
  }
  std::vector<int> route(double ox, double oy, double tx, double ty) const {
    int start = int(oy) * MAP + int(ox),
        goal = std::clamp(int(ty), 0, MAP - 1) * MAP +
               std::clamp(int(tx), 0, MAP - 1);
    std::array<int, MAP * MAP> prev;
    prev.fill(-1);
    std::queue<int> q;
    q.push(start);
    prev[start] = start;
    while (!q.empty() && prev[goal] < 0) {
      int p = q.front();
      q.pop();
      for (auto delta : std::array<int, 4>{{1, -1, MAP, -MAP}}) {
        int n = p + delta;
        if (n < 0 || n >= MAP * MAP ||
            std::abs(n % MAP - p % MAP) + std::abs(n / MAP - p / MAP) != 1 ||
            prev[n] >= 0 || arena.solid(n % MAP + .5, n / MAP + .5, .23))
          continue;
        prev[n] = p;
        q.push(n);
      }
    }
    std::vector<int> out;
    if (prev[goal] < 0)
      return out;
    for (int p = goal; p != start; p = prev[p])
      out.push_back(p);
    std::reverse(out.begin(), out.end());
    return out;
  }
  void tick(double dt) {
    if (ended)
      return;
    equipAnim = std::max(0., equipAnim - dt);
    for (auto &d : drops) {
      d.v.z -= 9.8 * dt;
      Vec next = d.p + d.v * dt;
      if (!arena.solid(next.x, next.y, .1)) {
        d.p.x = next.x;
        d.p.y = next.y;
      } else
        d.v.x = d.v.y = 0;
      d.p.z = next.z;
      if (d.p.z < .09) {
        d.p.z = .09;
        d.v = d.v * .0;
      }
    }
    if (freeze > 0) {
      freeze = std::max(0., freeze - dt);
      return;
    }
    if (jumpVelocity != 0 || jumpHeight > 0) {
      jumpVelocity -= 9.8 * dt;
      jumpHeight += jumpVelocity * dt;
      if (jumpHeight <= 0) {
        jumpHeight = jumpVelocity = 0;
        sounds.push_back(Step);
      }
    }
    for (auto &p : particles) {
      p.life -= dt;
      p.v.z -= 9.8 * dt;
      p.p = p.p + p.v * dt;
      if (p.p.z < .03) {
        p.p.z = .03;
        p.v.z = std::abs(p.v.z) * .3;
        p.v.x *= .8;
        p.v.y *= .8;
      }
    }
    particles.erase(std::remove_if(particles.begin(), particles.end(),
                                   [](auto p) { return p.life <= 0; }),
                    particles.end());
    for (auto &p : impacts)
      p.life -= dt;
    impacts.erase(std::remove_if(impacts.begin(), impacts.end(),
                                 [](auto p) { return p.life <= 0; }),
                  impacts.end());
    time += dt;
    if (!planted)
      clock = std::max(0., clock - dt);
    double oldBolt = boltCycle;
    boltCycle = std::max(0., boltCycle - dt);
    if (oldBolt > .5 && boltCycle <= .5)
      sounds.push_back(Bolt);
    muzzle = std::max(0., muzzle - dt);
    shot = shot > 0 ? shot - dt : 0;
    hurt = std::max(0., hurt - dt);
    hit = std::max(0., hit - dt);
    flash = std::max(0., flash - dt);
    recoil = std::max(0., recoil - dt * 4);
    for (auto &t : tracers)
      t.life -= dt;
    tracers.erase(std::remove_if(tracers.begin(), tracers.end(),
                                 [](Tracer t) { return t.life <= 0; }),
                  tracers.end());
    for (auto &c : smokes)
      c.time -= dt;
    smokes.erase(std::remove_if(smokes.begin(), smokes.end(),
                                [](Cloud c) { return c.time <= 0; }),
                 smokes.end());
    for (auto &b : blasts)
      b.time -= dt;
    blasts.erase(std::remove_if(blasts.begin(), blasts.end(),
                                [](Blast b) { return b.time <= 0; }),
                 blasts.end());
    if (reload > 0) {
      double old = reload;
      reload = std::max(0., reload - dt);
      if (old > .6 && reload <= .6)
        sounds.push_back(MagIn);
      if (old > .2 && reload <= .2)
        sounds.push_back(Bolt);
      if (reload == 0) {
        int n = std::min(gun().capacity - ammo(), reserve[weapon]);
        ammo() += n;
        reserve[weapon] -= n;
      }
    }
    for (auto &g : grenades) {
      g.fuse -= dt;
      g.v.z -= 9.8 * dt;
      Vec next = g.p + g.v * dt;
      if (arena.solid(next.x, g.p.y)) {
        g.v.x *= -.45;
      } else
        g.p.x = next.x;
      if (arena.solid(g.p.x, next.y)) {
        g.v.y *= -.45;
      } else
        g.p.y = next.y;
      g.p.z = next.z;
      if (g.p.z < .12) {
        g.p.z = .12;
        g.v.z = std::abs(g.v.z) * .35;
        g.v.x *= .8;
        g.v.y *= .8;
      }
      if (g.fuse <= 0) {
        if (g.kind == 1)
          smokes.push_back({g.p.x, g.p.y, 12});
        else if (g.kind == 0) {
          blasts.push_back({g.p, .45});
          sounds.push_back(Explosion);
          bool sourceTeam = g.owner < 0 ? attacker : bots[g.owner].terrorist;
          for (size_t i = 0; i < bots.size(); i++) {
            auto &b = bots[i];
            if (b.hp <= 0 || b.terrorist == sourceTeam)
              continue;
            double d = dist(b.x, b.y, g.p.x, g.p.y);
            if (d < 5 && clearLine(g.p.x, g.p.y, b.x, b.y))
              damageBot(int(i), int((1 - d / 5) * 160), g.owner);
          }
          double d = dist(x, y, g.p.x, g.p.y);
          if (hp > 0 && d < 4 && (g.owner < 0 || sourceTeam != attacker) &&
              clearLine(g.p.x, g.p.y, x, y)) {
            hp = std::max(0, hp - int((1 - d / 4) * 70));
            hurt = .4;
          }
        } else {
          sounds.push_back(Empty);
          for (auto &b : bots)
            if (dist(b.x, b.y, g.p.x, g.p.y) < 10 &&
                visible(g.p.x, g.p.y, b.x, b.y))
              b.blind = 4;
          double angle = atan2(g.p.y - y, g.p.x - x);
          if (dist(x, y, g.p.x, g.p.y) < 10 &&
              std::abs(wrap(angle - a)) < 1.5 && visible(x, y, g.p.x, g.p.y))
            flash = 3;
        }
      }
    }
    grenades.erase(std::remove_if(grenades.begin(), grenades.end(),
                                  [](Grenade g) { return g.fuse <= 0; }),
                   grenades.end());
    playerDeath();
    if (checkElimination())
      return;
    if (hp > 0 && usingObjective && reload <= 0 && !walking) {
      if (attacker && !planted && site(x, y)) {
        plantProgress += dt;
        if (plantProgress >= 3) {
          planted = true;
          bombX = x;
          bombY = y;
          bombClock = 40;
          sounds.push_back(BombBeep);
          plantProgress = 0;
        }
      } else if (!attacker && planted && dist(x, y, bombX, bombY) < 1.8) {
        defuseProgress += dt;
        if (defuseProgress >= 5) {
          finish(true, "BOMB DEFUSED");
          return;
        }
      } else
        plantProgress = defuseProgress = 0;
    } else
      plantProgress = defuseProgress = 0;
    if (planted) {
      bombClock -= dt;
      beep -= dt;
      if (beep <= 0) {
        sounds.push_back(BombBeep);
        beep = std::max(.15, bombClock / 40);
      }
      if (bombClock <= 0) {
        finish(attacker, "BOMB DETONATED");
        return;
      }
    } else if (clock <= 0) {
      finish(!attacker, "ROUND TIME EXPIRED");
      return;
    }
    tickBots(dt);
    hp = std::max(0, hp);
    playerDeath();
    if (hp <= 0) {
      plantProgress = defuseProgress = 0;
      usingObjective = buyOpen = false;
    }
    checkElimination();
  }
  std::string objectiveText() const {
    if (matchOver)
      return std::string(matchWon ? "MATCH WON " : "MATCH LOST ") +
             std::to_string(wins) + " : " + std::to_string(losses);
    if (ended)
      return result + " | Next round";
    if (planted)
      return std::string(attacker ? "DEFEND THE BOMB  "
                                  : "DEFUSE: HOLD F NEAR BOMB  ") +
             std::to_string(int(std::ceil(bombClock))) + "s";
    return attacker ? "ATTACK: Plant at A or B - hold F for 3 seconds"
                    : "DEFEND: Protect A and B; defuse planted bombs with F";
  }
};

#include "team_ai.hpp"
