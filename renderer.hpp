#pragma once
#include "game.hpp"
Game game;
std::vector<uint32_t> pixels(W *H);
std::vector<double> zbuffer(W *H);
bool aiming = false, moving = false;
unsigned rgb(int r, int g, int b) {
  return (std::clamp(r, 0, 255) << 16) | (std::clamp(g, 0, 255) << 8) |
         std::clamp(b, 0, 255);
}
unsigned shade(unsigned c, double v) {
  return rgb(int(((c >> 16) & 255) * v), int(((c >> 8) & 255) * v),
             int((c & 255) * v));
}
void rect(int x, int y, int w, int h, unsigned c) {
  for (int yy = std::max(0, y); yy < std::min(H, y + h); yy++)
    for (int xx = std::max(0, x); xx < std::min(W, x + w); xx++)
      pixels[yy * W + xx] = c;
}
void line(int x, int y, int ex, int ey, unsigned c, int width = 1) {
  int n = std::max(std::abs(ex - x), std::abs(ey - y));
  if (n > 10000)
    return;
  for (int i = 0; i <= n; i++) {
    double t = n ? double(i) / n : 0;
    rect(int(x + (ex - x) * t), int(y + (ey - y) * t), width, width, c);
  }
}
void ellipse(int cx, int cy, int rx, int ry, unsigned c) {
  if (rx <= 0 || ry <= 0)
    return;
  for (int y = -ry; y <= ry; y++)
    for (int x = -rx; x <= rx; x++)
      if (double(x * x) / (rx * rx) + double(y * y) / (ry * ry) <= 1)
        rect(cx + x, cy + y, 1, 1, c);
}
double focal() { return aiming ? (game.weapon == 3 ? 1150 : 850) : 630; }
struct Camera {
  double ca, sa, cp, sp;
  Camera()
      : ca(cos(game.a)), sa(sin(game.a)), cp(cos(game.pitch)),
        sp(sin(game.pitch)) {}
  Vec transform(Vec p) const {
    p = p - Vec{game.x, game.y, 1.6};
    double forward = p.x * ca + p.y * sa;
    return {-p.x * sa + p.y * ca, p.z * cp - forward * sp,
            forward * cp + p.z * sp};
  }
};
struct Screen {
  double x, y, iz;
};
Screen screen(Vec p) {
  return {W * .5 + p.x * focal() / p.z, H * .5 - p.y * focal() / p.z, 1 / p.z};
}
void raster(Vec a, Vec b, Vec c, unsigned color) {
  auto p = screen(a), q = screen(b), r = screen(c);
  double area = (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
  if (std::abs(area) < .01)
    return;
  int minx = int(std::max(0., floor(std::min({p.x, q.x, r.x})))),
      maxx = int(std::min(double(W - 1), ceil(std::max({p.x, q.x, r.x}))));
  int miny = int(std::max(0., floor(std::min({p.y, q.y, r.y})))),
      maxy = int(std::min(double(H - 1), ceil(std::max({p.y, q.y, r.y}))));
  if (minx > maxx || miny > maxy)
    return;
  for (int y = miny; y <= maxy; y++)
    for (int x = minx; x <= maxx; x++) {
      double u = ((q.x - x - .5) * (r.y - y - .5) -
                  (q.y - y - .5) * (r.x - x - .5)) /
                 area,
             v = ((r.x - x - .5) * (p.y - y - .5) -
                  (r.y - y - .5) * (p.x - x - .5)) /
                 area,
             w = 1 - u - v;
      if (u < -.00001 || v < -.00001 || w < -.00001)
        continue;
      double iz = u * p.iz + v * q.iz + w * r.iz;
      int index = y * W + x;
      if (iz <= zbuffer[index])
        continue;
      zbuffer[index] = iz;
      double fog = std::min(.8, 1 / (iz * 65));
      unsigned cc = shade(color, 1 - fog);
      pixels[index] =
          rgb(((cc >> 16) & 255) + int(55 * fog),
              ((cc >> 8) & 255) + int(70 * fog), (cc & 255) + int(81 * fog));
    }
}
void triangle(Vec a, Vec b, Vec c, unsigned color, const Camera &camera) {
  std::vector<Vec> poly = {camera.transform(a), camera.transform(b),
                           camera.transform(c)},
                   clipped;
  for (size_t i = 0; i < poly.size(); i++) {
    Vec p = poly[i], q = poly[(i + 1) % poly.size()];
    bool pin = p.z >= .08, qin = q.z >= .08;
    if (pin)
      clipped.push_back(p);
    if (pin != qin) {
      double t = (.08 - p.z) / (q.z - p.z);
      clipped.push_back(p + (q - p) * t);
    }
  }
  for (size_t i = 1; i + 1 < clipped.size(); i++)
    raster(clipped[0], clipped[i], clipped[i + 1], color);
}
void quad(Vec a, Vec b, Vec c, Vec d, unsigned color, const Camera &camera) {
  triangle(a, b, c, color, camera);
  triangle(a, c, d, color, camera);
}
void cube(Box box, const Camera &camera) {
  double x = box.x, y = box.y, z = box.z, xx = x + box.w, yy = y + box.d,
         zz = z + box.h;
  quad({x, y, z}, {xx, y, z}, {xx, y, zz}, {x, y, zz}, shade(box.color, .83),
       camera);
  quad({xx, yy, z}, {x, yy, z}, {x, yy, zz}, {xx, yy, zz},
       shade(box.color, .68), camera);
  quad({x, yy, z}, {x, y, z}, {x, y, zz}, {x, yy, zz}, shade(box.color, .72),
       camera);
  quad({xx, y, z}, {xx, yy, z}, {xx, yy, zz}, {xx, y, zz}, box.color, camera);
  quad({x, y, zz}, {xx, y, zz}, {xx, yy, zz}, {x, yy, zz},
       shade(box.color, 1.15), camera);
}
void beam(Vec a, Vec b, double radius, unsigned color, const Camera &camera) {
  Vec right = {-sin(game.a) * radius, cos(game.a) * radius, 0},
      up = {0, 0, radius};
  quad(a - right, b - right, b + right, a + right, color, camera);
  quad(a - up, b - up, b + up, a + up, color, camera);
}
void drawWorld() {
  Camera camera;
  std::fill(zbuffer.begin(), zbuffer.end(), 0);
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++)
      pixels[y * W + x] = rgb(48 + y / 24, 69 + y / 30, 91 + y / 40);
  for (int y = 0; y < MAP; y++)
    for (int x = 0; x < MAP; x++) {
      if (dist(game.x, game.y, x + .5, y + .5) > 42)
        continue;
      double xx = x, yy = y;
      unsigned ground = (x + y) % 2 ? 0x79786C : 0x727468;
      if (game.site(x + .5, y + .5))
        ground = 0xAA8D57;
      quad({xx, yy, 0}, {xx + 1, yy, 0}, {xx + 1, yy + 1, 0}, {xx, yy + 1, 0},
           ground, camera);
      if (arena.cells[y][x] == '#') {
        double height = arena.wallHeight(x, y);
        unsigned color = x > 18 ? 0x9EAAAD : 0xB3A58C;
        cube({xx, yy, 0, 1, 1, height, color},
             camera); // parapets and plinths give buildings real depth
        if (x % 3 == 0 && y % 3 == 0)
          cube({xx - .04, yy - .04, 2.8, 1.08, 1.08, .18, 0x5E696C}, camera);
      }
    }
  for (auto p : arena.props) {
    cube(p, camera);
    cube(
        {p.x - .025, p.y - .025, p.h * .3, p.w + .05, p.d + .05, .07, 0x414A48},
        camera);
    cube(
        {p.x - .025, p.y - .025, p.h * .8, p.w + .05, p.d + .05, .07, 0x414A48},
        camera);
  }
  // Physical overhead pipe bridges and support posts, well above the player.
  for (int xx : {5, 17, 25}) {
    cube({double(xx), 13, 3.5, .25, 7, .25, 0x637375, false}, camera);
    cube({double(xx), 13, 0, .18, .18, 3.5, 0x637375, false}, camera);
  }
  for (auto site : std::array<Vec, 2>{{{27, 5, 0}, {27, 26, 0}}}) {
    for (int i = 0; i < 4; i++) {
      double x = site.x - 2 + i;
      quad({x, site.y - 2, .012}, {x + .5, site.y - 2, .012},
           {x + .5, site.y - 1.88, .012}, {x, site.y - 1.88, .012}, 0xE6BC4C,
           camera);
    }
    cube({site.x + 1.8, site.y - 1, .01, .12, .12, 2.4, 0xF5C969, false},
         camera);
    cube({site.x + 1.78, site.y - 1.04, 1.6, .65, .2, .65, 0xA54832, false},
         camera);
  }
  for (size_t i = 0; i < game.bots.size(); i++) {
    auto b = game.bots[i];
    if (b.hp <= 0) {
      cube({b.x - .45, b.y - .2, .03, .9, .4, .2, 0x3C4C4D}, camera);
      continue;
    }
    double anim = b.path.empty() ? 0 : sin(game.time * 9 + i) * .13;
    unsigned cloth = game.attacker ? 0x456478 : 0x827149;
    cube({b.x - .23, b.y - .17, .7, .46, .34, .7, cloth}, camera);
    cube({b.x - .19, b.y - .21, .88, .38, .42, .44, 0x303C40}, camera);
    cube({b.x - .16, b.y - .15, 1.43, .32, .3, .32, 0xB9A07D}, camera);
    cube({b.x - .19, b.y - .18, 1.62, .38, .36, .2, 0x34444A}, camera);
    cube({b.x - .28, b.y - .17, 1.18, .13, .3, .28, cloth}, camera);
    cube({b.x + .15, b.y - .17, 1.18, .13, .3, .28, cloth}, camera);
    cube({b.x - .2, b.y - .13 + anim, .14, .16, .25, .59, cloth}, camera);
    cube({b.x + .04, b.y - .13 - anim, .14, .16, .25, .59, cloth}, camera);
    cube({b.x - .2, b.y - .21 + anim, .04, .17, .36, .16, 0x263035}, camera);
    cube({b.x + .04, b.y - .21 - anim, .04, .17, .36, .16, 0x263035}, camera);
    double angle = atan2(game.y - b.y, game.x - b.x);
    Vec muzzle = {b.x + cos(angle) * .55, b.y + sin(angle) * .55, 1.3};
    beam({b.x, b.y, 1.3}, muzzle, .055, 0x202B30, camera);
    if (b.flash > 0)
      cube({muzzle.x - .09, muzzle.y - .09, 1.23, .18, .18, .16, 0xFFE2A0,
            false},
           camera);
  }
  for (auto t : game.tracers)
    beam(t.from, t.to, .012, t.enemy ? 0xFFAE67 : 0xFFDF96, camera);
  for (auto g : game.grenades)
    cube({g.p.x - .1, g.p.y - .1, g.p.z - .1, .2, .2, .2,
          g.kind == 0   ? 0x65733D
          : g.kind == 1 ? 0x8B9290u
                        : 0xD0D0B7u,
          false},
         camera);
  if (game.planted) {
    cube({game.bombX - .23, game.bombY - .15, .02, .46, .3, .18, 0x343E37,
          false},
         camera);
    cube({game.bombX - .12, game.bombY - .1, .21, .24, .2, .025,
          int(game.time * 7) % 2 ? 0xFA4737u : 0x793127u, false},
         camera);
  }
  for (auto b : game.blasts) {
    double s = (.5 - b.time) * 4 + .2;
    cube({b.p.x - s / 2, b.p.y - s / 2, .1, s, s, s, 0xF8B263, false}, camera);
  }
  // Smoke volumes use depth-aware translucent screen discs; geometry remains
  // fully 3D.
  for (auto c : game.smokes) {
    Vec pos = camera.transform({c.x, c.y, 1.3});
    if (pos.z < .1) {
      if (dist(game.x, game.y, c.x, c.y) < 2.4)
        for (auto &p : pixels)
          p = rgb(int(((p >> 16) & 255) * .12 + 115),
                  int(((p >> 8) & 255) * .12 + 122),
                  int((p & 255) * .12 + 124));
      continue;
    }
    auto s = screen(pos);
    int radius = std::min(1800, int(focal() * 2.6 / pos.z));
    int left = std::max(0, int(s.x) - radius),
        right = std::min(W - 1, int(s.x) + radius),
        top = std::max(0, int(s.y) - radius),
        bottom = std::min(H - 1, int(s.y) + radius);
    for (int y = top; y <= bottom; y++)
      for (int x = left; x <= right; x++) {
        double dx = (x - s.x) / radius, dy = (y - s.y) / radius;
        if (dx * dx + dy * dy > 1 ||
            zbuffer[y * W + x] > 1 / std::max(.08, pos.z - 2))
          continue;
        auto &p = pixels[y * W + x];
        double alpha = std::min(.96, (1 - dx * dx - dy * dy) * 1.6);
        p = rgb(int(((p >> 16) & 255) * (1 - alpha) + 128 * alpha),
                int(((p >> 8) & 255) * (1 - alpha) + 136 * alpha),
                int((p & 255) * (1 - alpha) + 139 * alpha));
      }
  }
}
void render() {
  drawWorld();
  double phase = game.reload > 0 ? 1 - game.reload / game.gun().reload : 0;
  int drop = game.reload > 0 ? int(sin(phase * PI) * 110) : 0;
  int bob = moving ? int(sin(game.time * 11) * 4) : 0;
  int gx = (aiming ? W / 2 : W / 2 + 100) +
           (moving ? int(cos(game.time * 5.5) * 3) : 0),
      gy = H - 85 - (aiming ? 60 : 0) + drop + bob + int(game.recoil * 17);
  ellipse(gx - 65, gy + 95, 65, 85, 0x303E37);
  ellipse(gx + 90, gy + 105, 70, 85, 0x303E37);
  rect(gx - 48, gy - 20, 25, 85, 0x9B836A);
  rect(gx + 39, gy + 4, 28, 65, 0x9B836A);
  if (game.weapon == 0) {
    rect(gx - 19, gy - 85, 38, 125, 0x252D34);
    rect(gx - 19, gy - 90, 38, 50, 0x7F8B8C);
    rect(gx - 14, gy - 84, 28, 8, 0x313B42);
    rect(gx - 7, gy - 96, 14, 8, 0xC6D7CB);
  } else {
    unsigned color = game.weapon == 2   ? 0x4E5A60
                     : game.weapon == 3 ? 0x667252
                                        : 0x3D484F;
    int barrel = game.weapon == 3 ? 190 : game.weapon == 2 ? 125 : 151;
    rect(gx - 24, gy - 104, 48, 142, 0x171F24);
    rect(gx - 18, gy - 100, 36, 125, color);
    rect(gx - 12, gy - 99, 8, 120, shade(color, 1.4));
    rect(gx - 13, gy - barrel, 26, barrel - 87, 0x252F35);
    rect(gx - 18, gy - 109, 36, 10, 0x11191D);
    for (int i = 0; i < 7; i++)
      rect(gx - 21, gy - 80 + i * 9, 42, 3, 0x1D272E);
    rect(gx - 32, gy + 13, 64, 54, 0x202A30);
    rect(gx - 17, gy + 18, 34, 34, 0x4C565B);
    rect(gx - 7, gy - 155, 14, 5, 0x899795);
    if (game.weapon == 3) {
      ellipse(gx, gy - 30, 24, 32, 0x151F24);
      ellipse(gx, gy - 30, 16, 24, 0x526E75);
    }
  }
  int magdrop = game.reload > 0 ? int(sin(phase * PI) * 100) : 0;
  rect(gx - 19 - magdrop / 3, gy + 56 + magdrop, 38, 75, 0x343D3F);
  for (int i = 0; i < 5; i++)
    rect(gx - 15 - magdrop / 3, gy + 63 + magdrop + i * 12, 29, 3, 0x202A2C);
  if (game.muzzle > 0 && game.reload <= 0) {
    int flashY = gy - (game.weapon == 0 ? 106 : game.weapon == 3 ? 200 : 166);
    ellipse(gx, flashY, 22, 12, 0xFFB44C);
    line(gx - 35, flashY - 9, gx + 35, flashY + 11, 0xFFE9AF, 4);
    line(gx, flashY - 24, gx, flashY + 26, 0xFFE9AF, 4);
  }
  if (aiming && game.weapon == 3) {
    for (int y = 0; y < H; y++)
      for (int x = 0; x < W; x++)
        if ((x - W / 2) * (x - W / 2) + (y - H / 2) * (y - H / 2) > 260 * 260)
          pixels[y * W + x] = 0x10181C;
    line(W / 2 - 250, H / 2, W / 2 + 250, H / 2, 0x12222A);
    line(W / 2, H / 2 - 250, W / 2, H / 2 + 250, 0x12222A);
  }
  for (int y = 0; y < H; y++)
    for (int x = 0; x < W; x++) {
      double dx = (x - W * .5) / (W * .5), dy = (y - H * .5) / (H * .5);
      auto &c = pixels[y * W + x];
      c = shade(c, 1 - .18 * std::min(1., dx * dx + dy * dy));
      if (game.hurt > 0 && (std::abs(dx) > .9 || std::abs(dy) > .87))
        c = 0x8D302B;
      if (game.flash > 0) {
        double alpha = std::min(.98, game.flash / 2);
        c = rgb(int(((c >> 16) & 255) * (1 - alpha) + 245 * alpha),
                int(((c >> 8) & 255) * (1 - alpha) + 245 * alpha),
                int((c & 255) * (1 - alpha) + 230 * alpha));
      }
    }
  unsigned cross = game.hit > 0 ? 0xF0AE65 : 0xD8E2DA;
  int gap = 5 + int(game.recoil * 5) + (moving ? 4 : 0);
  line(W / 2 - gap - 5, H / 2, W / 2 - gap, H / 2, cross);
  line(W / 2 + gap, H / 2, W / 2 + gap + 5, H / 2, cross);
  line(W / 2, H / 2 - gap - 5, W / 2, H / 2 - gap, cross);
  line(W / 2, H / 2 + gap, W / 2, H / 2 + gap + 5, cross);
  if (game.hit > 0) {
    line(W / 2 - 8, H / 2 - 8, W / 2 - 3, H / 2 - 3, cross);
    line(W / 2 + 8, H / 2 + 8, W / 2 + 3, H / 2 + 3, cross);
  }
  rect(16, 16, 136, 136, 0x111B21);
  for (int y = 0; y < MAP; y++)
    for (int x = 0; x < MAP; x++)
      rect(20 + x * 4, 20 + y * 4, 3, 3,
           arena.cells[y][x] == '#'    ? 0x64706C
           : game.site(x + .5, y + .5) ? 0xB49B5D
                                       : 0x202C30);
  int mx = 20 + int(game.x * 4), my = 20 + int(game.y * 4);
  rect(mx - 2, my - 2, 4, 4, 0x85D7C4);
  line(mx, my, mx + int(cos(game.a) * 8), my + int(sin(game.a) * 8), 0x85D7C4);
  if (game.planted)
    rect(20 + int(game.bombX * 4) - 2, 20 + int(game.bombY * 4) - 2, 4, 4,
         0xF35A41);
  rect(18, H - 65, 250, 48, 0x19252B);
  rect(W - 280, H - 65, 262, 48, 0x19252B);
  rect(28, H - 25, 120, 3, 0x394447);
  rect(28, H - 25, game.hp * 120 / 100, 3, 0x90B9A2);
  if (game.plantProgress > 0 || game.defuseProgress > 0) {
    rect(W / 2 - 100, H - 100, 200, 8, 0x283C41);
    rect(W / 2 - 100, H - 100,
         int(200 * std::max(game.plantProgress / 3, game.defuseProgress / 5)),
         8, 0xE6C379);
  }
  if (game.buyOpen)
    rect(240, 140, 485, 280, 0x182A33);
}
