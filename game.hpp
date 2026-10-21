#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <queue>
#include <string>
#include <vector>
constexpr double PI = 3.141592653589793;
constexpr int W = 960, H = 600, MAP = 32;
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
struct Box {
  double x, y, z, w, d, h;
  uint32_t color;
  bool cover = true;
};
struct Arena {
  std::array<std::string, MAP> cells;
  std::vector<Box> props;
  Arena() {
    for (int y = 0; y < MAP; y++) {
      cells[y] = std::string(MAP, '.');
      for (int x = 0; x < MAP; x++)
        if (!x || !y || x == MAP - 1 || y == MAP - 1)
          cells[y][x] = '#';
    }
    // Two long routes, central connector, and open objective courtyards.
    for (int y = 5; y <= 24; y++)
      for (int x = 7; x <= 11; x++)
        if (y < 13 || y > 17)
          cells[y][x] = '#';
    for (int y = 6; y <= 25; y++)
      for (int x = 19; x <= 23; x++)
        if (y < 14 || y > 18)
          cells[y][x] = '#';
    for (int x = 12; x < 19; x++)
      for (int y = 9; y < 12; y++)
        cells[y][x] = '#';
    for (int x = 12; x < 18; x++)
      for (int y = 22; y < 25; y++)
        cells[y][x] = '#';
    props = {{4, 8, 0, 1.3, 1.3, 1.25, 0x8F714E},
             {4, 15, 0, 1.3, 2, 1.25, 0x8F714E},
             {13, 15, 0, 1.4, 1.4, 1.1, 0x96794E},
             {16, 17, 0, 1.4, 1.4, 1.7, 0x88704F},
             {26, 10, 0, 1.5, 2, 1.4, 0x687878},
             {26, 19, 0, 1.5, 2, 1.4, 0x687878},
             {14, 4, 0, 2, 1, 1.2, 0x937B53},
             {17, 28, 0, 1.7, 1, 1.2, 0x937B53},
             {24.5, 27.5, 0, 1, 1, 1.1, 0x597269}};
  }
  double wallHeight(int x, int y) const {
    return (x == 0 || y == 0 || x == MAP - 1 || y == MAP - 1)
               ? 3.
               : 3.4 + (x / 5 % 2) * .8;
  }
  bool wall(double x, double y) const {
    return x < 0 || y < 0 || x >= MAP || y >= MAP ||
           cells[int(y)][int(x)] == '#';
  }
  bool solid(double x, double y, double margin = 0) const {
    if (wall(x - margin, y - margin) || wall(x + margin, y - margin) ||
        wall(x - margin, y + margin) || wall(x + margin, y + margin))
      return true;
    for (auto p : props)
      if (p.cover && x + margin > p.x && x - margin < p.x + p.w &&
          y + margin > p.y && y - margin < p.y + p.d)
        return true;
    return false;
  }
};
Arena arena;
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
};
const std::array<Weapon, 4> weapons = {
    {{"SERVICE PISTOL", 12, 34, 0, .25, 1.3, .008, 24},
     {"CARBINE", 30, 35, 2400, .105, 2.1, .012, 38},
     {"COMPACT SMG", 32, 24, 1200, .075, 1.65, .023, 20},
     {"MARKSMAN", 5, 100, 3500, 1.1, 2.6, .003, 60}}};
struct Bot {
  double x, y;
  int hp = 100;
  double cooldown = 1, flash = 0;
  double alert = 0, lastX = 0, lastY = 0, repath = 0, blind = 0, reload = 0,
         plant = 0;
  int ammo = 20;
  bool knows = false;
  std::vector<int> path{};
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
};
struct Cloud {
  double x, y, time;
};
struct Blast {
  Vec p;
  double time;
};
struct Game {
  double x = 3, y = 16, a = 0, pitch = 0, shot = 0, muzzle = 0, reload = 0,
         time = 0, hurt = 0, hit = 0, recoil = 0, steps = 0, flash = 0,
         clock = 115, plantProgress = 0, defuseProgress = 0, bombClock = 40,
         bombX = 0, bombY = 0, beep = 0;
  int hp = 100, armor = 0, kills = 0, round = 0, weapon = 0, money = 3200,
      wins = 0, losses = 0;
  bool won = false, ended = false, attacker = true, planted = false,
       usingObjective = false, buyOpen = false, walking = false, ads = false;
  std::string result;
  std::array<bool, 4> owned{{true, false, false, false}};
  std::array<int, 4> magazines{{12, 30, 32, 5}}, reserve{{48, 90, 96, 15}};
  std::array<int, 3> utilities{{1, 1, 1}};
  int utility = 0;
  uint32_t random = 913;
  std::vector<Bot> bots;
  std::vector<Tracer> tracers;
  std::vector<Sound> sounds;
  std::vector<Grenade> grenades;
  std::vector<Cloud> smokes;
  std::vector<Blast> blasts;
  Game() { reset(); }
  int &ammo() { return magazines[weapon]; }
  const Weapon &gun() const { return weapons[weapon]; }
  double rng() {
    random = random * 1664525 + 1013904223;
    return double(random >> 8) / 16777216.;
  }
  void reset() {
    round++;
    if (round > 1)
      attacker = !attacker;
    x = attacker ? 3 : 28;
    y = 16;
    a = attacker ? 0 : PI;
    pitch = shot = muzzle = reload = time = hurt = hit = recoil = steps =
        flash = plantProgress = defuseProgress = beep = 0;
    hp = 100;
    kills = 0;
    clock = 115;
    bombClock = 40;
    won = ended = planted = usingObjective = buyOpen = walking = ads = false;
    result.clear();
    bots.clear();
    for (auto p : std::array<Vec, 5>{{{27, 5, 0},
                                      {27, 26, 0},
                                      {16, 15, 0},
                                      {25, 16, 0},
                                      {15, 27, 0}}}) {
      if (!attacker)
        p.x = MAP - p.x;
      bots.push_back({p.x, p.y});
    }
    tracers.clear();
    sounds.clear();
    grenades.clear();
    smokes.clear();
    blasts.clear();
    utilities = {1, 1, 1};
    for (int i = 0; i < 4; i++) {
      magazines[i] = weapons[i].capacity;
      reserve[i] = weapons[i].capacity * 3;
    }
    if (!owned[weapon])
      weapon = 0;
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
      armor = 0;
      owned = {true, false, false, false};
    }
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
    for (int yy = 0; yy < MAP; yy++)
      for (int xx = 0; xx < MAP; xx++)
        if (arena.cells[yy][xx] == '#')
          best = std::min(best, boxHit(origin, dir,
                                       {double(xx), double(yy), 0, 1, 1,
                                        arena.wallHeight(xx, yy), 0}));
    for (auto p : arena.props)
      best = std::min(best, boxHit(origin, dir, p));
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
    if (ended)
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
  bool buyAllowed() const {
    return time < 22 && dist(x, y, attacker ? 3 : 28, 16) < 4 && !ended;
  }
  bool buy(int item) {
    if (!buyAllowed())
      return false;
    if (item >= 1 && item <= 3) {
      if (owned[item] || money < weapons[item].price)
        return false;
      money -= weapons[item].price;
      owned[item] = true;
      switchWeapon(item);
      return true;
    }
    if (item == 4 && money >= 650 && armor < 100) {
      money -= 650;
      armor = 100;
      return true;
    }
    if (item == 5 && money >= 300) {
      money -= 300;
      for (auto &n : utilities)
        n = std::min(2, n + 1);
      return true;
    }
    return false;
  }
  void switchWeapon(int i) {
    if (i < 0 || i > 3 || !owned[i] || ended)
      return;
    weapon = i;
    reload = 0;
    shot = .25;
    plantProgress = defuseProgress = 0;
  }
  void startReload() {
    if (hp <= 0 || ended || reload > 0 || ammo() == gun().capacity ||
        reserve[weapon] <= 0)
      return;
    reload = gun().reload;
    sounds.push_back(MagOut);
    plantProgress = defuseProgress = 0;
  }
  void hear(double ox, double oy, double radius) {
    for (auto &b : bots)
      if (b.hp > 0 && dist(b.x, b.y, ox, oy) < radius) {
        b.knows = true;
        b.lastX = ox;
        b.lastY = oy;
        b.alert = 5;
        b.repath = 0;
      }
  }
  void fire() {
    if (hp <= 0 || ended || buyOpen || shot > 0 || reload > 0 || usingObjective)
      return;
    if (ammo() <= 0) {
      shot = .25;
      sounds.push_back(Empty);
      return;
    }
    ammo()--;
    shot = gun().delay;
    muzzle = .055;
    recoil = std::min(2., recoil + .65);
    sounds.push_back(Rifle);
    hear(x, y, 24);
    double spread =
        gun().spread * (walking ? 3 : 1) * (ads ? .4 : 1) * (1 + recoil * .4);
    Vec dir =
        direction(a + (rng() - .5) * spread, pitch + (rng() - .5) * spread);
    Vec origin = {x, y, 1.6};
    double nearest = geometry(origin, dir, gun().range);
    Bot *target = nullptr;
    bool head = false;
    for (auto &b : bots) {
      if (b.hp <= 0)
        continue;
      double body = boxHit(origin, dir,
                           {b.x - .24, b.y - .24, .2, .48, .48, 1.23, 0}),
             skull = boxHit(origin, dir,
                            {b.x - .17, b.y - .17, 1.43, .34, .34, .37, 0});
      double d = std::min(body, skull);
      if (d < nearest) {
        nearest = d;
        target = &b;
        head = skull < body;
      }
    }
    tracers.push_back({origin, origin + dir * nearest, .09, false});
    if (target) {
      target->hp -= gun().damage * (head ? 3 : 1);
      hit = .2;
      sounds.push_back(Hit);
      if (target->hp <= 0) {
        kills++;
        money = std::min(16000, money + 300);
      }
    }
    if (!planted && kills == int(bots.size()))
      finish(true, "OPPOSITION ELIMINATED");
  }
  int site(double xx, double yy) const {
    if (dist(xx, yy, 27, 5) < 3)
      return 1;
    if (dist(xx, yy, 27, 26) < 3)
      return 2;
    return 0;
  }
  void throwUtility() {
    if (ended || hp <= 0 || buyOpen || utilities[utility] <= 0)
      return;
    utilities[utility]--;
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
    time += dt;
    clock -= dt;
    muzzle = std::max(0., muzzle - dt);
    shot = std::max(0., shot - dt);
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
          for (auto &b : bots)
            if (b.hp > 0) {
              double d = dist(b.x, b.y, g.p.x, g.p.y);
              if (d < 5 && clearLine(g.p.x, g.p.y, b.x, b.y)) {
                b.hp -= int((1 - d / 5) * 160);
                if (b.hp <= 0) {
                  kills++;
                  money = std::min(16000, money + 300);
                }
              }
            }
          double d = dist(x, y, g.p.x, g.p.y);
          if (d < 4 && clearLine(g.p.x, g.p.y, x, y)) {
            hp -= int((1 - d / 4) * 70);
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
    if (usingObjective && reload <= 0 && !walking) {
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
    for (size_t i = 0; i < bots.size(); i++) {
      auto &b = bots[i];
      b.flash = std::max(0., b.flash - dt);
      b.blind = std::max(0., b.blind - dt);
      if (b.hp <= 0)
        continue;
      b.cooldown -= dt;
      b.alert = std::max(0., b.alert - dt);
      b.repath -= dt;
      if (b.reload > 0) {
        b.reload -= dt;
        if (b.reload <= 0)
          b.ammo = 20;
      }
      double d = dist(x, y, b.x, b.y);
      bool sees = b.blind <= 0 && d < 23 && visible(b.x, b.y, x, y);
      if (sees) {
        b.knows = true;
        b.lastX = x;
        b.lastY = y;
        b.alert = 5;
        for (auto &other : bots)
          if (other.hp > 0 && dist(other.x, other.y, b.x, b.y) < 9) {
            other.knows = true;
            other.lastX = x;
            other.lastY = y;
            other.alert = 4;
          }
      }
      double tx = b.lastX, ty = b.lastY;
      bool objective = planted || (!attacker && !sees);
      if (objective) {
        tx = planted ? bombX : 27;
        ty = planted ? bombY : (i % 2 ? 26 : 5);
      } else if (!b.knows || b.alert <= 0) {
        tx = i % 2 ? 26 : 15;
        ty = i % 2 ? 6 : 26;
      }
      bool hold = sees && !objective && d > 4 && d < 13 && b.reload <= 0;
      if (!hold && b.blind <= 0) {
        if (b.repath <= 0) {
          b.path = route(b.x, b.y, tx, ty);
          b.repath = .7 + i * .07;
        }
        if (!b.path.empty()) {
          int p = b.path.front();
          double nx = p % MAP + .5, ny = p / MAP + .5,
                 dd = dist(b.x, b.y, nx, ny);
          if (dd < .13)
            b.path.erase(b.path.begin());
          else {
            double speed = (b.reload > 0 ? .7 : 1.45) * dt;
            double xx = b.x + (nx - b.x) / dd * std::min(speed, dd),
                   yy = b.y + (ny - b.y) / dd * std::min(speed, dd);
            if (!arena.solid(xx, b.y, .2))
              b.x = xx;
            if (!arena.solid(b.x, yy, .2))
              b.y = yy;
          }
        }
      } else if (hold) {
        double strafe = sin(time * 1.7 + i * 2) * dt * .6;
        double xx = b.x - (y - b.y) / std::max(1., d) * strafe,
               yy = b.y + (x - b.x) / std::max(1., d) * strafe;
        if (!arena.solid(xx, yy, .23)) {
          b.x = xx;
          b.y = yy;
        }
      }
      if (planted && attacker && dist(b.x, b.y, bombX, bombY) < 1.6 &&
          b.blind <= 0) {
        b.plant += dt;
        if (b.plant >= 5) {
          finish(false, "DEFENDERS DEFUSED THE BOMB");
          return;
        }
      } else if (!attacker && !planted && site(b.x, b.y) && !sees &&
                 b.blind <= 0) {
        b.plant += dt;
        if (b.plant >= 3) {
          planted = true;
          bombX = b.x;
          bombY = b.y;
          bombClock = 40;
          for (auto &enemy : bots)
            enemy.plant = 0;
          sounds.push_back(BombBeep);
        }
      } else
        b.plant = 0;
      if (sees && d < 21 && b.cooldown <= 0 && b.reload <= 0) {
        if (b.ammo <= 0) {
          b.reload = 2;
          continue;
        }
        b.ammo--;
        b.flash = .08;
        b.cooldown = .35 + rng() * .45;
        tracers.push_back({{b.x, b.y, 1.4}, {x, y, 1.35}, .12, true});
        sounds.push_back(EnemyShot);
        if (rng() < std::clamp(.65 - d * .022, .12, .6)) {
          int damage = armor > 0 ? 5 : 9;
          armor = std::max(0, armor - 4);
          hp -= damage;
          hurt = .25;
        }
      }
    }
    if (hp <= 0) {
      hp = 0;
      finish(false, "ELIMINATED");
      return;
    }
    if (kills == int(bots.size()) && (!planted || !attacker)) {
      if (!planted)
        finish(true, "OPPOSITION ELIMINATED");
    }
  }
  std::string objectiveText() const {
    if (ended)
      return result + " | Enter: next round";
    if (planted)
      return std::string(attacker ? "DEFEND THE BOMB  "
                                  : "DEFUSE: HOLD F NEAR BOMB  ") +
             std::to_string(int(std::ceil(bombClock))) + "s";
    return attacker ? "ATTACK: Plant at A or B - hold F for 3 seconds"
                    : "DEFEND: Protect A and B; defuse planted bombs with F";
  }
};
