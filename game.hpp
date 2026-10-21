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
  std::vector<Box> props, obstacles;
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
    for (int y = 0; y < MAP; y++)
      for (int x = 0; x < MAP; x++)
        if (cells[y][x] == '#')
          obstacles.push_back(
              {double(x), double(y), 0, 1, 1, wallHeight(x, y), 0});
    obstacles.insert(obstacles.end(), props.begin(), props.end());
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
  bool automatic = false;
};
const std::array<Weapon, 5> weapons = {
    {{"SERVICE PISTOL", 12, 34, 0, .22, 1.3, .008, 24, false},
     {"AUTO CARBINE", 30, 35, 2400, .065, 2.1, .012, 38, true},
     {"RAPID SMG", 32, 24, 1200, .05, 1.65, .023, 20, true},
     {"BOLT SNIPER", 5, 110, 3500, 1.2, 2.6, .001, 64, false},
     {"SCOUT DMR", 10, 72, 2800, .28, 2.2, .004, 55, false}}};
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
struct Game {
  double x = 3, y = 16, a = 0, pitch = 0, shot = 0, muzzle = 0, reload = 0,
         time = 0, hurt = 0, hit = 0, recoil = 0, steps = 0, flash = 0,
         clock = 115, plantProgress = 0, defuseProgress = 0, bombClock = 40,
         bombX = 0, bombY = 0, beep = 0, boltCycle = 0, freeze = 0, vx = 0,
         vy = 0, stance = 0, jumpHeight = 0, jumpVelocity = 0;
  bool sprinting = false;
  int hp = 100, armor = 0, kills = 0, round = 0, weapon = 0, money = 3200,
      wins = 0, losses = 0, spectator = 0;
  bool won = false, ended = false, attacker = true, planted = false,
       usingObjective = false, buyOpen = false, walking = false, ads = false;
  std::string result;
  std::array<bool, 5> owned{{true, false, false, false, false}};
  std::array<int, 5> magazines{{12, 30, 32, 5, 10}},
      reserve{{48, 90, 96, 15, 30}};
  std::array<int, 3> utilities{{1, 1, 1}};
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
  std::vector<std::string> feed;
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
        flash = plantProgress = defuseProgress = beep = boltCycle = 0;
    freeze = vx = vy = stance = jumpHeight = jumpVelocity = 0;
    sprinting = false;
    particles.clear();
    impacts.clear();
    spectator = 0;
    feed.clear();
    hp = 100;
    kills = 0;
    clock = 115;
    bombClock = 40;
    won = ended = planted = usingObjective = buyOpen = walking = ads = false;
    result.clear();
    bots.clear();
    const std::array<Vec, 5> tSpawn = {
        {{3, 13, 0}, {3, 19, 0}, {5.8, 17.5, 0}, {5, 20, 0}, {3, 22, 0}}};
    const std::array<Vec, 5> ctSpawn = {
        {{27, 5, 0}, {27, 26, 0}, {25, 15, 0}, {28, 18, 0}, {28, 22, 0}}};
    for (int team = 0; team < 2; team++)
      for (int i = 0; i < 5; i++) {
        Vec p = team ? tSpawn[i] : ctSpawn[i];
        Bot bot{p.x, p.y};
        bot.terrorist = team;
        bot.role = i;
        bot.gun = i == 4 ? 3 : i == 1 ? 2 : 1;
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
    utilities = {1, 1, 1};
    for (int i = 0; i < 5; i++) {
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
      owned = {true, false, false, false, false};
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
    for (const auto &obstacle : arena.obstacles)
      best = std::min(best, boxHit(origin, dir, obstacle));
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
    return hp > 0 && time < 22 && dist(x, y, attacker ? 3 : 28, 16) < 4 &&
           !ended;
  }
  bool buy(int item) {
    if (!buyAllowed())
      return false;
    if ((item >= 1 && item <= 3) || item == 6) {
      if (item == 6)
        item = 4;
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
    if (i < 0 || i > 4 || !owned[i] || ended || hp <= 0 || boltCycle > 0)
      return;
    weapon = i;
    reload = 0;
    shot = .2;
    plantProgress = defuseProgress = 0;
  }
  void startReload() {
    if (hp <= 0 || ended || boltCycle > 0 || reload > 0 ||
        ammo() == gun().capacity || reserve[weapon] <= 0)
      return;
    reload = gun().reload;
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
        boltCycle > 0 || reload > 0 || usingObjective)
      return;
    if (ammo() <= 0) {
      shot = .25;
      sounds.push_back(Empty);
      return;
    }
    ammo()--;
    shot = std::max(-.005, shot) + gun().delay;
    if (weapon == 3)
      boltCycle = .95;
    muzzle = .055;
    recoil = std::min(2., recoil + .65);
    sounds.push_back(Rifle);
    hear(x, y, 24);
    double spread = gun().spread *
                    (jumpHeight > .03 ? 5
                     : walking        ? 3
                     : stance > .5    ? .65
                                      : 1) *
                    (ads ? .4 : 1) * (1 + recoil * .4);
    Vec dir =
        direction(a + (rng() - .5) * spread, pitch + (rng() - .5) * spread);
    Vec origin = {x, y, 1.6 - stance * .5 + jumpHeight};
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
    particles.push_back({origin + Vec{-sin(a) * .2, cos(a) * .2, -.12},
                         {-sin(a) * 1.7, cos(a) * 1.7, 1.1},
                         1.,
                         0xC7A65C});
    if (!target) {
      Vec at = origin + dir * std::max(0., nearest - .025);
      impacts.push_back({at, 20});
      if (impacts.size() > 64)
        impacts.erase(impacts.begin());
      for (int i = 0; i < 4; i++)
        particles.push_back(
            {at,
             {(rng() - .5) * 1.5, (rng() - .5) * 1.5, rng() * 1.4},
             .25,
             0xFFC983});
    }
    if (target && target->terrorist != attacker) {
      damageBot(int(target - bots.data()), gun().damage * (head ? 3 : 1), -1);
      hit = .2;
      sounds.push_back(Hit);
    }
    checkElimination();
  }
  void trigger(bool held, bool pressed) {
    if (gun().automatic ? held : pressed)
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
    b.hp = std::max(0, b.hp - damage);
    b.suppression = 1.2;
    if (b.hp == 0) {
      b.plant = 0;
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
    if (hp <= 0) {
      plantProgress = defuseProgress = 0;
      usingObjective = buyOpen = false;
    }
    checkElimination();
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

#include "team_ai.hpp"
