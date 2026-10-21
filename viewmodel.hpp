#pragma once
// Camera-space meshes, not screen rectangles. Every part is transformed,
// near-clipped, lit and depth-tested just like the world geometry.
struct WeaponModel {
  Camera camera{{0, 0, 0}, 0, 0};
  double yaw, roll, pitch;
  Vec offset;
  double scale = 1;
  WeaponModel(Vec origin, double heading, double size)
      : camera(), yaw(heading), roll(PI / 2), pitch(0), offset(origin),
        scale(size) {}
  WeaponModel() {
    double phase = game.reload > 0 ? 1 - game.reload / game.gun().reload : 0;
    double anim = sin(phase * PI);
    yaw = -anim * .35 + (game.sprinting ? .13 : 0) +
          (moving ? sin(game.time * 5.5) * .015 : 0);
    roll = anim * .7;
    pitch =
        anim * .35 +
        game.recoil * (scopedGun(game.weapon) && aimBlend > .8 ? .005 : .07) -
        (game.sprinting ? .16 : 0);
    offset = {.22 * (scopedGun(game.weapon) ? 1 - aimBlend : 1) -
                  game.recoil * .02,
              .22 * (1 - aimBlend),
              -.32 + (scopedGun(game.weapon) ? .13 : .17) * aimBlend};
    offset.z -= game.equipAnim * .6;
    offset.z -= anim * .18;
    if (moving)
      offset.z += sin(game.time * 11) * .008;
  }
  Vec point(Vec p) const {
    p = p * scale;
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
    // Chamfered extrusions give receivers, stocks, grips and hands physical
    // edge bevels instead of six square faces.
    double y = b.y, z = b.z, Y = y + b.d, Z = z + b.h,
           r = std::min({b.d * .18, b.h * .18, .025});
    std::array<Vec, 8> ring = {{{0, y + r, z},
                                {0, Y - r, z},
                                {0, Y, z + r},
                                {0, Y, Z - r},
                                {0, Y - r, Z},
                                {0, y + r, Z},
                                {0, y, Z - r},
                                {0, y, z + r}}};
    for (int i = 0; i < 8; i++) {
      Vec p = ring[i], q = ring[(i + 1) % 8];
      p.x = q.x = b.x;
      Vec P = p, Q = q;
      P.x = Q.x = b.x + b.w;
      face(p, q, Q, P, shade(b.color, i % 2 ? 1.08 : .87));
      triangle(point({b.x, (y + Y) * .5, (z + Z) * .5}), point(q), point(p),
               shade(b.color, .7), camera);
      triangle(point({b.x + b.w, (y + Y) * .5, (z + Z) * .5}), point(P),
               point(Q), b.color, camera);
    }
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
  void hollow(double start, double end, double y, double z, double outer,
              double inner, unsigned color) {
    for (int i = 0; i < 16; i++) {
      double t = 2 * PI * i / 16, u = 2 * PI * (i + 1) / 16;
      Vec p = {0, cos(t), sin(t)}, q = {0, cos(u), sin(u)}, a = {start, y, z},
          b = {end, y, z};
      face(a + p * outer, a + q * outer, b + q * outer, b + p * outer, color);
      face(a + q * inner, a + p * inner, b + p * inner, b + q * inner,
           shade(color, .4));
      face(a + q * outer, a + p * outer, a + p * inner, a + q * inner,
           shade(color, 1.3));
      face(b + p * outer, b + q * outer, b + q * inner, b + p * inner, color);
    }
  }
};
std::vector<double> modelDepth(W *H);
void paintWeapon(WeaponModel &m, int gun, bool animate) {
  const unsigned black = 0x202A30, steel = 0x53616A;
  double phase = animate && game.reload > 0
                     ? 1 - game.reload / weapons[gun].reload
                     : 0,
         mag = sin(phase * PI) * .38;
  if (secondaryGun(gun)) {
    double length = gun == 5   ? .43
                    : gun == 6 ? .28
                               : .36,
           width = gun == 5 ? .13 : .10,
           slide = animate && game.muzzle > 0 ? -.045 : 0;
    m.box({.25, -width * .4, -.22, .10, width * .8, .22, black});
    m.box({.26 + slide, -width * .5, -.015, length, width, .09,
           gun == 5   ? 0xB6B2A4
           : gun == 7 ? 0x677A82
                      : steel});
    m.cylinder(.26 + length - .04, .29 + length, 0, .025,
               gun == 5 ? .035 : .025, black);
    m.box({.29, -width * .5, .075, .025, .025, .025, black});
    m.box({.29, width * .25, .075, .025, .025, .025, black});
    m.box({.24 + length, -.008, .075, .02, .016, .025, 0xCDE0BA});
    m.box({.265 - mag * .3, -.04, -.29 - mag, .08, .08, .15, steel});
    if (gun == 7)
      m.box({.3, .05, .03, .04, .015, .03, 0xB99F56});
  } else {
    bool scope = scopedGun(gun), smg = gun == 2 || gun == 15,
         shotgun = gun == 11 || gun == 12, mg = gun == 13, bullpup = gun == 10;
    double front = smg       ? .77
                   : scope   ? (gun == 14 ? .96 : 1.12)
                   : shotgun ? 1.16
                   : mg      ? 1.14
                             : .95;
    unsigned body = gun == 8   ? 0x586D68
                    : gun == 9 ? 0x89694C
                    : bullpup  ? 0x9C8D68
                    : shotgun  ? 0x827258
                    : scope    ? 0x667452
                    : mg       ? 0x62706A
                    : smg      ? 0x637078
                               : steel;
    m.box({.22, -.064, -.07, smg ? .28 : .34, mg ? .16 : .128, .13, body});
    m.box({.04, -.065, -.12, .2, .13, .14, black});
    m.box({0, -.09, -.17, .065, .18, .22, body});
    m.box({.25, -.045, -.25, .095, .09, .21, black});
    m.box({.33, -.055, .065, .26, .11, .027, black});
    for (int i = 0; i < 7; i++)
      m.box({.35 + i * .03, -.06, .092, .012, .12, .012, steel});
    m.box({.55, -.052, -.055, std::max(.1, front - .55), .104, .105, body});
    m.cylinder(front, front + .22, 0, .005, shotgun ? .038 : .024, black);
    m.cylinder(front + .2, front + .26, 0, .005, shotgun ? .044 : .034, steel);
    if (gun == 8)
      m.cylinder(front + .12, front + .46, 0, .005, .048, 0x3F4B50);
    for (int i = 0; i < 5; i++) {
      m.box({.58 + i * .035, -.057, -.02, .016, .012, .035, black});
      m.box({.58 + i * .035, .045, -.02, .016, .012, .035, black});
    }
    double mx = bullpup ? .18 : .48;
    if (mg) {
      m.cylinder(.35, .6, 0, -.2 - mag, .14, black);
      for (int i = 0; i < 5; i++)
        m.cylinder(.4, .47, .08 + i * .028, .01, .014, 0xC6AA62);
    } else if (gun == 11) {
      m.cylinder(.55, 1.13, 0, -.065, .028, steel);
      double pump = animate && game.boltCycle > 0
                        ? sin((1 - game.boltCycle / .55) * PI) * .10
                        : 0;
      m.cylinder(.72 - pump, .91 - pump, 0, -.065, .057, body);
    } else {
      m.box({mx - mag * .2, -.047, -.34 - mag, .13, .094, smg ? .24 : .29,
             steel});
      for (int i = 0; i < 4; i++)
        m.box({mx + .01 - mag * .2, -.051, -.3 - mag + i * .05, .105, .102,
               .008, black});
    }
    m.box({.31, -.067, -.08, .025, .013, .045, 0xACB1A7});
    m.box({.28, .064, -.01, .1, .012, .02, black});
    if (scope) {
      double radius = gun == 3    ? .066
                      : gun == 14 ? .06
                                  : .052,
             inside = radius * .78;
      m.box({.32, -.025, .09, .08, .05, .065, black});
      m.box({.53, -.025, .09, .08, .05, .065, black});
      m.hollow(.26, .68, 0, .19, radius, inside, black);
      m.hollow(.26, .285, 0, .19, radius * 1.1, inside, steel);
      m.hollow(.64, .7, 0, .19, radius * 1.2, inside, steel);
      m.box({.43, -.025, .235, .04, .05, .05, steel});
      double cycle = animate && game.boltCycle > 0
                         ? sin((1 - game.boltCycle / .8) * PI) * .11
                         : 0;
      m.box({.38 - cycle, .055, .02, .04, .1, .03, steel});
      m.cylinder(.38 - cycle, .43 - cycle, .15, .035, .025, black);
    } else {
      m.box({.31, -.055, .095, .025, .023, .045, black});
      m.box({.31, .032, .095, .025, .023, .045, black});
      m.box({front - .025, -.017, .07, .025, .034, .075, black});
      m.box({front - .02, -.008, .14, .018, .016, .012, 0xC2D7C7});
    }
  }
}
void drawViewmodel() {
  if (game.hp <= 0 || game.ended || !game.owned[game.weapon])
    return;
  std::swap(zbuffer, modelDepth);
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  projectionOverride = 630;
  int previous = activeMaterial;
  activeMaterial = 4;
  WeaponModel m;
  segment(m.point({0, -.16, -.30}), m.point({.36, -.12, -.22}), .085, .065,
          0x36443A, m.camera);
  segment(m.point({.1, .13, -.34}), m.point({.43, .06, -.22}), .09, .065,
          0x36443A, m.camera);
  segment(m.point({.34, -.12, -.22}), m.point({.46, -.08, -.18}), .07, .065,
          0xAE8E70, m.camera);
  segment(m.point({.39, .06, -.22}), m.point({.48, .035, -.18}), .07, .065,
          0xAE8E70, m.camera);
  if (game.utilityEquipped) {
    m.cylinder(.3, .47, 0, -.03, .09,
               game.utility == 0   ? 0x678044
               : game.utility == 1 ? 0x8C9694
                                   : 0xB8C2AE);
    m.box({.37, -.025, .04, .1, .05, .04, 0x283439});
    m.hollow(.33, .35, 0, .08, .035, .026, 0x9DA9A5);
  } else {
    paintWeapon(m, game.weapon, true);
    if (game.muzzle > 0 && game.reload <= 0) {
      double tip = secondaryGun(game.weapon) ? .7
                   : scopedGun(game.weapon)  ? 1.42
                                             : 1.25;
      m.cylinder(tip, tip + .16, 0, .005, .045, 0xFFF0B2);
      m.face({tip, 0, -.1}, {tip + .22, 0, .005}, {tip, 0, .11},
             {tip - .03, 0, .005}, 0xFFC363);
    }
  }
  projectionOverride = 0;
  activeMaterial = previous;
  std::swap(zbuffer, modelDepth);
}
void drawDroppedWeapons() {
  int previous = activeMaterial;
  activeMaterial = 4;
  for (auto d : game.drops) {
    if (dist(game.eye().x, game.eye().y, d.p.x, d.p.y) > 20)
      continue;
    WeaponModel m(d.p, d.angle, .75);
    paintWeapon(m, d.gun, false);
  }
  activeMaterial = previous;
}
void drawScopeLens() {
  if (!scopedGun(game.weapon) || aimBlend < .88 || game.reload > 0 ||
      game.utilityEquipped || game.hp <= 0)
    return;
  WeaponModel model;
  projectionOverride = 630;
  Vec center = model.camera.transform(model.point({.26, 0, .19}));
  auto screenCenter = screen(center);
  double radius = (game.weapon == 3    ? .066
                   : game.weapon == 14 ? .06
                                       : .052) *
                  .78 * 630 / center.z;
  projectionOverride = 0;
  int cx = int(screenCenter.x), cy = int(screenCenter.y), r = int(radius);
  if (r < 1 || r > 300)
    return;
  auto outside = pixels;
  auto outsideDepth = zbuffer;
  projectionOverride = game.weapon == 4    ? 1100
                       : game.weapon == 14 ? 1250
                                           : 1500;
  renderRegion = {W / 2 - r, H / 2 - r, 2 * r, 2 * r};
  drawWorld();
  drawDroppedWeapons();
  renderRegion = {0, 0, W, H};
  auto lens = pixels;
  pixels = std::move(outside);
  zbuffer = std::move(outsideDepth);
  projectionOverride = 0;
  for (int y = std::max(0, cy - r); y < std::min(H, cy + r); y++)
    for (int x = std::max(0, cx - r); x < std::min(W, cx + r); x++)
      if ((x - cx) * (x - cx) + (y - cy) * (y - cy) < r * r) {
        int sx = W / 2 + (x - cx), sy = H / 2 + (y - cy);
        if (sx >= 0 && sx < W && sy >= 0 && sy < H)
          pixels[y * W + x] = lens[sy * W + sx];
      }
  line(cx - r + 8, cy, cx + r - 8, cy, 0x213A36);
  line(cx, cy - r + 8, cx, cy + r - 8, 0x213A36);
  ellipse(cx, cy, 2, 2, 0xCEB76A);
}
