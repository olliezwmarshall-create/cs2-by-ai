#pragma once
// Camera-space meshes, not screen rectangles. Every part is transformed,
// near-clipped, lit and depth-tested just like the world geometry.
struct WeaponModel {
  Camera camera{{0, 0, 0}, 0, 0};
  double yaw, roll, pitch;
  Vec offset;
  WeaponModel() {
    double phase = game.reload > 0 ? 1 - game.reload / game.gun().reload : 0;
    double anim = sin(phase * PI);
    yaw = -anim * .35 + (moving ? sin(game.time * 5.5) * .015 : 0);
    roll = anim * .7;
    pitch = anim * .35 + game.recoil * .07;
    offset = {.22 - game.recoil * .045, aiming ? 0. : .22,
              aiming ? -.15 : -.32};
    offset.z -= anim * .18;
    if (moving)
      offset.z += sin(game.time * 11) * .008;
  }
  Vec point(Vec p) const {
    double x = p.x * cos(pitch) - p.z * sin(pitch),
           z = p.x * sin(pitch) + p.z * cos(pitch), y = p.y;
    double yy = y * cos(roll) - z * sin(roll),
           zz = y * sin(roll) + z * cos(roll);
    return Vec{x * cos(yaw) - yy * sin(yaw), x * sin(yaw) + yy * cos(yaw), zz} +
           offset;
  }
  void face(Vec a, Vec b, Vec c, Vec d, unsigned color) {
    quad(point(a), point(b), point(c), point(d), color, camera);
  }
  void box(Box b) {
    double x = b.x, y = b.y, z = b.z, X = x + b.w, Y = y + b.d, Z = z + b.h;
    face({x, y, z}, {X, y, z}, {X, y, Z}, {x, y, Z}, shade(b.color, .72));
    face({X, Y, z}, {x, Y, z}, {x, Y, Z}, {X, Y, Z}, shade(b.color, .9));
    face({x, Y, z}, {x, y, z}, {x, y, Z}, {x, Y, Z}, shade(b.color, .65));
    face({X, y, z}, {X, Y, z}, {X, Y, Z}, {X, y, Z}, b.color);
    face({x, y, Z}, {X, y, Z}, {X, Y, Z}, {x, Y, Z}, shade(b.color, 1.2));
    face({x, y, z}, {x, Y, z}, {X, Y, z}, {X, y, z}, shade(b.color, .55));
  }
  void cylinder(double start, double end, double y, double z, double radius,
                unsigned color) {
    constexpr int N = 12;
    for (int i = 0; i < N; i++) {
      double a = 2 * PI * i / N, b = 2 * PI * (i + 1) / N;
      Vec p = {start, y + cos(a) * radius, z + sin(a) * radius},
          q = {start, y + cos(b) * radius, z + sin(b) * radius},
          r = {end, q.y, q.z}, s = {end, p.y, p.z};
      face(p, q, r, s, shade(color, .7 + .25 * (sin(a) + 1)));
      triangle(point({start, y, z}), point(q), point(p), shade(color, .5),
               camera);
      triangle(point({end, y, z}), point(s), point(r), shade(color, .8),
               camera);
    }
  }
};
std::vector<double> modelDepth(W *H);
void drawViewmodel() {
  if (game.hp <= 0 || game.ended)
    return;
  std::swap(zbuffer, modelDepth);
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  projectionOverride = 630;
  WeaponModel m;
  unsigned steel = 0x414D55, black = 0x202A30, skin = 0xAE8E70,
           glove = 0x36443A;
  // Arms and gloves have independent 3D volume.
  m.box({.0, -.16, -.3, .35, .14, .14, glove});
  m.box({.1, .04, -.34, .35, .15, .15, glove});
  m.box({.35, -.13, -.22, .12, .11, .12, skin});
  m.box({.34, .035, -.22, .13, .12, .12, skin});
  double phase = game.reload > 0 ? 1 - game.reload / game.gun().reload : 0,
         magTravel = sin(phase * PI) * .38;
  if (game.weapon == 0) {
    double slide = game.muzzle > 0 ? -.04 : 0;
    m.box({.25, -.045, -.22, .1, .09, .2, black});
    m.box({.26 + slide, -.055, -.015, .36, .11, .09, 0x8A969A});
    m.cylinder(.56, .65, 0, .03, .027, black);
    m.box({.29, -.055, .07, .035, .025, .025, black});
    m.box({.29, .03, .07, .035, .025, .025, black});
    m.box({.58, -.01, .075, .025, .02, .025, 0xBFCAC4});
    m.box(
        {.265 - magTravel * .3, -.04, -.29 - magTravel, .08, .08, .15, steel});
  } else {
    bool sniper = game.weapon == 3 || game.weapon == 4;
    unsigned body = sniper ? 0x626C4C : game.weapon == 2 ? 0x637078 : steel;
    double front = game.weapon == 2 ? .78 : sniper ? 1.12 : .95;
    m.box({.22, -.062, -.07, .34, .124, .13, body});
    m.box({.04, -.065, -.12, .2, .13, .14, black});
    m.box({.0, -.09, -.17, .065, .18, .22, black}); // stock / buttplate
    m.box({.25, -.045, -.25, .095, .09, .21, black});
    m.box({.33, -.055, .065, .26, .11, .027, black});
    for (int i = 0; i < 7; i++)
      m.box({.35 + i * .03, -.06, .091, .012, .12, .012, steel});
    m.box({.55, -.052, -.055, front - .55, .104, .105, body});
    m.cylinder(front, front + .22, 0, .005, .024, black);
    m.cylinder(front + .2, front + .26, 0, .005, .034, steel);
    for (int i = 0; i < 5; i++) {
      m.box({.58 + i * .035, -.057, -.02, .016, .012, .035, black});
      m.box({.58 + i * .035, .045, -.02, .016, .012, .035, black});
    }
    m.box(
        {.48 - magTravel * .2, -.047, -.34 - magTravel, .13, .094, .29, steel});
    for (int i = 0; i < 4; i++)
      m.box({.49 - magTravel * .2, -.051, -.3 - magTravel + i * .05, .105, .102,
             .008, black});
    m.box({.31, -.067, -.08, .025, .013, .045, 0xACB1A7});
    m.box({.28, .064, -.01, .1, .012, .02, black});
    if (sniper) {
      m.box({.32, -.025, .09, .08, .05, .065, black});
      m.box({.53, -.025, .09, .08, .05, .065, black});
      m.cylinder(.27, .68, 0, .19, game.weapon == 3 ? .066 : .045, black);
      m.cylinder(.26, .275, 0, .19, game.weapon == 3 ? .052 : .035, 0x578D98);
      m.cylinder(.66, .7, 0, .19, .07, steel);
      m.box({.43, -.025, .235, .04, .05, .05, steel});
      double cycle =
          game.boltCycle > 0 ? sin((1 - game.boltCycle / .95) * PI) * .11 : 0;
      m.box({.38 - cycle, .055, .02, .04, .1, .03, steel});
      m.cylinder(.38 - cycle, .43 - cycle, .15, .035, .025, black);
    } else {
      m.box({.31, -.055, .095, .025, .023, .045, black});
      m.box({.31, .032, .095, .025, .023, .045, black});
      m.box({front - .025, -.017, .07, .025, .034, .075, black});
      m.box({front - .02, -.008, .14, .018, .016, .012, 0xC2D7C7});
    }
  }
  if (game.muzzle > 0 && game.reload <= 0) {
    double tip = game.weapon == 0   ? .69
                 : game.weapon == 2 ? 1.08
                 : game.weapon >= 3 ? 1.42
                                    : 1.25;
    m.cylinder(tip, tip + .16, 0, .005, .045, 0xFFF0B2);
    m.face({tip, 0, -.1}, {tip + .22, 0, .005}, {tip, 0, .11},
           {tip - .03, 0, .005}, 0xFFC363);
    m.face({tip, -.1, .005}, {tip + .22, 0, .005}, {tip, .1, .005},
           {tip - .03, 0, .005}, 0xFFE5A1);
  }
  projectionOverride = 0;
  std::swap(zbuffer, modelDepth);
}
