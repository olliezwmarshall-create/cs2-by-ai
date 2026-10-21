#pragma once
#include "model_render.hpp"
void dome(Vec center, double rx, double ry, double rz, bool hemisphere,
          unsigned color, const Camera &camera) {
  double radius = std::max({rx, ry, rz});
  Vec check = camera.transform(center);
  if (check.z < -radius ||
      std::abs(check.x) >
          std::max(0., check.z) * W / (2 * focal()) + radius * 1.5)
    return;
  bool small = focal() * radius / std::max(.1, check.z) < 7;
  int rings = hemisphere ? (small ? 3 : 6) : (small ? 6 : 12),
      slices = small ? 10 : 20;
  for (int j = 0; j < rings; j++)
    for (int i = 0; i < slices; i++) {
      double a = 2 * PI * i / slices, b = 2 * PI * (i + 1) / slices,
             p = (hemisphere ? 0 : -PI / 2) +
                 j * (hemisphere ? PI / 2 : PI) / rings,
             q = (hemisphere ? 0 : -PI / 2) +
                 (j + 1) * (hemisphere ? PI / 2 : PI) / rings;
      auto v = [&](double u, double t) {
        return center +
               Vec{rx * cos(t) * cos(u), ry * cos(t) * sin(u), rz * sin(t)};
      };
      quad(v(a, p), v(b, p), v(b, q), v(a, q), color, camera);
    }
}
void drawRoundSolid(RoundSolid c, const Camera &camera) {
  if (dist(camera.origin.x, camera.origin.y, c.x, c.y) > 48)
    return;
  int previous = activeMaterial;
  activeMaterial = 4;
  segment({c.x, c.y, .03}, {c.x, c.y, c.h}, c.r, c.r, c.color, camera);
  for (double z : {.16, c.h * .72})
    segment({c.x, c.y, z}, {c.x, c.y, z + .07}, c.r * 1.02, c.r * 1.02,
            0x465B60, camera);
  if (c.dome)
    dome({c.x, c.y, c.h}, c.r, c.r, c.r, true, shade(c.color, 1.15), camera);
  else if (arena.id == 1) {
    activeMaterial = 2;
    dome({c.x, c.y, c.h}, c.r * .92, c.r * .92, .22, true, 0x71847B, camera);
  }
  activeMaterial = previous;
}
void arch(Vec at, const Camera &camera) {
  int previous = activeMaterial;
  activeMaterial = 1;
  for (int side : {-1, 1})
    segment(at + Vec{0, side * 2.5, 0}, at + Vec{0, side * 2.5, 3.8}, .25, .25,
            0xC5B496, camera);
  for (int i = 0; i < 20; i++) {
    double a = i * PI / 20, b = (i + 1) * PI / 20;
    Vec p = at + Vec{0, cos(a) * 2.5, 3.7 + sin(a) * 2.5},
        q = at + Vec{0, cos(b) * 2.5, 3.7 + sin(b) * 2.5};
    segment(p, q, .24, .24, i % 2 ? 0xCCBEA3 : 0xBDAE91, camera);
  }
  activeMaterial = previous;
}
void drawArchitecture(const Camera &camera) {
  int previous = activeMaterial;
  for (auto b : arena.structures) {
    Vec v = camera.transform({b.x + b.w * .5, b.y + b.d * .5, b.h * .5});
    if (v.z < -b.w - b.d || dist(camera.origin.x, camera.origin.y,
                                 b.x + b.w * .5, b.y + b.d * .5) > 70)
      continue;
    double x = b.x, y = b.y, X = x + b.w, Y = y + b.d, h = b.h;
    activeMaterial = arena.id == 1 ? 3 : 4;
    unsigned roof = arena.id == 1   ? 0xA46F54
                    : arena.id == 2 ? 0x687F85
                                    : 0x627B79;
    // A pitched roof spans the entire structure, with overhanging eaves and
    // triangular gables rather than stacked voxel tops.
    double ridge = arena.id == 1 ? 2.3 : 1.4, mid = (y + Y) * .5;
    quad({x - .25, y - .28, h}, {X + .25, y - .28, h},
         {X + .25, mid, h + ridge}, {x - .25, mid, h + ridge}, roof, camera);
    quad({X + .25, Y + .28, h}, {x - .25, Y + .28, h},
         {x - .25, mid, h + ridge}, {X + .25, mid, h + ridge}, shade(roof, .82),
         camera);
    triangle({x, y, h}, {x, mid, h + ridge}, {x, Y, h}, shade(b.color, .9),
             camera);
    triangle({X, Y, h}, {X, mid, h + ridge}, {X, y, h}, b.color, camera);
    activeMaterial = 1;
    segment({x - .08, y - .08, h - .12}, {X + .08, y - .08, h - .12}, .07, .07,
            0xD1CDC0, camera);
    segment({x - .08, Y + .08, h - .12}, {X + .08, Y + .08, h - .12}, .07, .07,
            0xB8BDB2, camera);
    activeMaterial = 4;
    for (double xx : {x + .07, X - .07})
      segment({xx, y - .08, .2}, {xx, y - .08, h}, .035, .035, 0x657B7B,
              camera);
    if (arena.id == 0) {
      segment({x + b.w * .5, mid, h + ridge},
              {x + b.w * .5, mid, h + ridge + .7}, .25, .25, 0x8DA09C, camera);
      dome({x + b.w * .5, mid, h + ridge + .7}, .36, .36, .15, true, 0xA9B5AF,
           camera);
    }
    if (arena.id == 1) {
      for (int side : {-1, 1}) {
        double yy = side < 0 ? y - .16 : Y + .16;
        for (double xx = x + 1; xx < X - 1; xx += 3) {
          Vec sill = {xx, yy, 1.3};
          segment(sill + Vec{-.48, 0, 0}, sill + Vec{.48, 0, 0}, .08, .08,
                  0xBDB39D, camera);
          for (int j = 0; j < 12; j++) {
            double a = j * PI / 12, c = (j + 1) * PI / 12;
            segment({xx + cos(a) * .44, yy, 2.5 + sin(a) * .44},
                    {xx + cos(c) * .44, yy, 2.5 + sin(c) * .44}, .04, .04,
                    0xDDD0B8, camera);
          }
        }
      }
    }
  }
  for (auto at : arena.arches)
    arch(at, camera);
  if (arena.id == 0) {
    for (double x : {arena.size * .37, arena.size * .69}) {
      double y = arena.size * .5 - 7;
      activeMaterial = 4;
      segment({x, y, 0}, {x, y, 10}, .16, .13, 0x8A9C91, camera);
      segment({x, y, 10}, {x, y + 12, 10}, .14, .10, 0xA2B1A0, camera);
      for (int i = 0; i < 5; i++) {
        segment({x, y + i * 2.4, 10}, {x, y + (i + 1) * 2.4, 11}, .035, .035,
                0xAAB7A5, camera);
        segment({x, y + i * 2.4, 11}, {x, y + (i + 1) * 2.4, 10}, .035, .035,
                0xAAB7A5, camera);
      }
      segment({x, y + 10, 10}, {x, y + 10, 5.5}, .02, .02, 0x555E58, camera);
    }
  }
  if (arena.id == 2) {
    for (auto at : std::array<Vec, 2>{
             {{arena.size * .5 + 7, 8, 0},
              {arena.size * .5 + 7, double(arena.size - 9), 0}}}) {
      activeMaterial = 4;
      segment(at + Vec{0, 0, 2}, at + Vec{0, 0, 7}, .08, .06, 0xB3C2BF, camera);
      dome(at + Vec{0, 0, 7}, 1.5, .25, 1.4, false, 0x9DB7BD, camera);
    }
  }
  activeMaterial = previous;
}

void drawLandscape(const Camera &camera) {
  if (arena.id == 0)
    return;
  int previous = activeMaterial;
  activeMaterial = 6;
  for (int i = 0; i < 7; i++) {
    double x = 10 + i * 15,
           h = arena.id == 2 ? 10 + (i % 3) * 5 : 7 + (i % 3) * 3;
    unsigned color = arena.id == 2 ? 0x607C82u : 0xA69B85u;
    dome({x, -12, -2}, 18, 22, h, true, color, camera);
    dome({x, double(arena.size + 14), -2}, 20, 22, h * .85, true, color,
         camera);
  }
  activeMaterial = previous;
}
