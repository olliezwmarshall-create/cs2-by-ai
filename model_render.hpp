#pragma once
void texturedTriangle(Vec a, Vec b, Vec c, Vec ta, Vec tb, Vec tc,
                      const TextureImage &image, const Camera &camera) {
  Vec ab = b - a, ac = c - a,
      n = {ab.y * ac.z - ab.z * ac.y, ab.z * ac.x - ab.x * ac.z,
           ab.x * ac.y - ab.y * ac.x};
  Vec to = camera.origin - a;
  if (n.x * to.x + n.y * to.y + n.z * to.z <= 0)
    return;
  double len = length(n);
  if (len > 0)
    n = n * (1 / len);
  struct V {
    Vec p, w, uv;
  };
  std::vector<V> poly = {{camera.transform(a), a, ta},
                         {camera.transform(b), b, tb},
                         {camera.transform(c), c, tc}},
                 out;
  for (size_t i = 0; i < poly.size(); i++) {
    auto p = poly[i], q = poly[(i + 1) % poly.size()];
    bool pin = p.p.z >= .08, qin = q.p.z >= .08;
    if (pin)
      out.push_back(p);
    if (pin != qin) {
      double t = (.08 - p.p.z) / (q.p.z - p.p.z);
      out.push_back({p.p + (q.p - p.p) * t, p.w + (q.w - p.w) * t,
                     p.uv + (q.uv - p.uv) * t});
    }
  }
  for (size_t i = 1; i + 1 < out.size(); i++) {
    std::array<Vec, 3> wp = {out[0].w, out[i].w, out[i + 1].w},
                       uv = {out[0].uv, out[i].uv, out[i + 1].uv};
    raster(out[0].p, out[i].p, out[i + 1].p, 0xFFFFFF, &wp, n, 0, &uv, &image);
  }
}
void drawArt(const ArtMesh &mesh, Vec origin, double scale, double heading,
             const Camera &camera) {
  if (mesh.vertices.empty())
    return;
  Vec center = camera.transform(origin + Vec{0, 0, scale * .5});
  if (center.z < -.5 ||
      std::abs(center.x) > std::max(1., center.z) * W / (2 * focal()) + scale)
    return;
  std::vector<Vec> positions;
  positions.reserve(mesh.vertices.size());
  for (auto v : mesh.vertices) {
    Vec p = v.p * scale;
    positions.push_back(origin + Vec{p.x * cos(heading) - p.y * sin(heading),
                                     p.x * sin(heading) + p.y * cos(heading),
                                     p.z});
  }
  for (auto f : mesh.faces) {
    auto a = mesh.vertices[f.a], b = mesh.vertices[f.b], c = mesh.vertices[f.c];
    texturedTriangle(positions[f.a], positions[f.b], positions[f.c],
                     {a.u, a.v, 0}, {b.u, b.v, 0}, {c.u, c.v, 0},
                     mesh.images[f.material], camera);
  }
}
// A tapered 10-sided limb segment can rotate around its joint. Texture detail
// remains anchored in world space while the mesh is animated.
void segment(Vec a, Vec b, double radiusA, double radiusB, unsigned color,
             const Camera &camera) {
  Vec axis = b - a;
  double d = length(axis);
  if (d < .001)
    return;
  axis = axis * (1 / d);
  Vec right = {-axis.y, axis.x, 0};
  double rl = length(right);
  if (rl < .01)
    right = {1, 0, 0};
  else
    right = right * (1 / rl);
  Vec up = {axis.y * right.z - axis.z * right.y,
            axis.z * right.x - axis.x * right.z,
            axis.x * right.y - axis.y * right.x};
  for (int i = 0; i < 10; i++) {
    double t = 2 * PI * i / 10, u = 2 * PI * (i + 1) / 10;
    Vec p = right * cos(t) + up * sin(t), q = right * cos(u) + up * sin(u);
    quad(a + p * radiusA, a + q * radiusA, b + q * radiusB, b + p * radiusB,
         color, camera);
    triangle(a, a + q * radiusA, a + p * radiusA, color, camera);
    triangle(b, b + p * radiusB, b + q * radiusB, color, camera);
  }
}
void drawCharacter(const Bot &b, int id, const Camera &camera) {
  double d = dist(camera.origin.x, camera.origin.y, b.x, b.y);
  if (d > 65)
    return;
  Vec at = camera.transform({b.x, b.y, 1});
  if (at.z < -.5 || std::abs(at.x) > std::max(1., at.z) * W / (2 * focal()) + 1)
    return;
  double death =
      b.hp > 0 ? 0 : std::clamp((game.visualTime - b.deathTime) / .65, 0., 1.);
  double lean = death * PI * .48;
  double stride =
      b.hp > 0 ? sin(b.walkPhase) * std::min(.22, b.locomotion * .1) : 0;
  auto point = [&](Vec p) {
    if (b.plant > 0)
      p.z -= .30;
    double z = p.z * cos(lean) - p.x * sin(lean),
           x = p.x * cos(lean) + p.z * sin(lean);
    return Vec{b.x + x * cos(b.facing) - p.y * sin(b.facing),
               b.y + x * sin(b.facing) + p.y * cos(b.facing), std::max(.08, z)};
  };
  unsigned cloth = b.terrorist ? 0x9B8864 : 0x4B7085, vest = 0x34423F,
           skin = 0xBBA384;
  int previous = activeMaterial;
  activeMaterial = 5;
  segment(point({0, 0, .77}), point({.015, 0, 1.4}), .20, .25, cloth, camera);
  segment(point({.04, 0, .92}), point({.05, 0, 1.30}), .23, .25, vest, camera);
  for (int side : {-1, 1}) {
    double swing = stride * side;
    Vec hip = point({0, side * .125, .82}),
        knee = point({swing, side * .13, .46}),
        foot = point({-swing, side * .13, .13});
    segment(hip, knee, .12, .095, cloth, camera);
    segment(knee, foot, .095, .075, cloth, camera);
    segment(foot, point({.16 - swing, side * .13, .11}), .10, .11, 0x253137,
            camera);
    Vec shoulder = point({0, side * .24, 1.35}),
        elbow = point({.19, side * .29, 1.1}),
        hand = point(
            {.41, side * .10,
             1.25 - (b.reload > 0 && side < 0
                         ? .3 * sin((1 - b.reload / weapons[b.gun].reload) * PI)
                         : 0)});
    segment(shoulder, elbow, .10, .08, cloth, camera);
    segment(elbow, hand, .08, .06, skin, camera);
    segment(hand, point({.46, side * .08, 1.26}), .07, .07, vest, camera);
  }
  activeMaterial = 1;
  segment(point({0, 0, 1.42}), point({0, 0, 1.65}), .14, .15, skin, camera);
  activeMaterial = 4;
  segment(point({-.015, 0, 1.60}), point({-.015, 0, 1.77}), .19, .13, 0x3B494A,
          camera);
  segment(point({.15, -.12, 1.59}), point({.15, .12, 1.59}), .045, .045,
          0x1D3038, camera);
  // Pouches, shoulder armor and a facing-oriented rifle avoid featureless
  // blocks.
  for (int side : {-1, 1})
    segment(point({.18, side * .13, 1.02}), point({.18, side * .13, 1.17}),
            .065, .065, 0x556055, camera);
  segment(point({.3, 0, 1.27}), point({.8, 0, 1.27}), .055, .025, 0x283238,
          camera);
  if (b.flash > 0) {
    activeMaterial = 0;
    segment(point({.81, 0, 1.27}), point({1.03, 0, 1.27}), .10, .01, 0xFFE7A6,
            camera);
  }
  if (b.hp > 0 && b.terrorist == game.attacker) {
    activeMaterial = 0;
    cube({b.x - .05, b.y - .05, 2.01, .1, .1, .1, 0x8FE5A7, false}, camera);
  }
  activeMaterial = previous;
  (void)id;
}

void drawProp(Box p, int type, const Camera &camera) {
  int previous = activeMaterial;
  if (type % 4 == 2) {
    activeMaterial = 4;
    double radius = std::min(p.w, p.d) * .47, cx = p.x + p.w * .5,
           cy = p.y + p.d * .5;
    segment({cx, cy, .04}, {cx, cy, p.h}, radius, radius, 0x6D8586, camera);
    for (double t : {.18, .75})
      segment({cx, cy, p.h * t}, {cx, cy, p.h * t + .07}, radius * 1.035,
              radius * 1.035, 0x344A50, camera);
    segment({cx, cy, p.h}, {cx, cy, p.h + .03}, radius * .94, radius * .94,
            0x9AA7A2, camera);
    segment({cx + radius * .4, cy, p.h + .03},
            {cx + radius * .4, cy, p.h + .045}, .04, .04, 0x354749, camera);
  } else if (type % 4 == 3) {
    activeMaterial = 1;
    double x = p.x, y = p.y, X = x + p.w, Y = y + p.d, h = p.h;
    quad({x, y, 0}, {X, y, 0}, {X, y + .22, h}, {x, y + .22, h}, 0xA3A49B,
         camera);
    quad({X, Y, 0}, {x, Y, 0}, {x, Y - .22, h}, {X, Y - .22, h}, 0x8C9590,
         camera);
    quad({x, y + .22, h}, {X, y + .22, h}, {X, Y - .22, h}, {x, Y - .22, h},
         0xB2B4A7, camera);
    quad({x, Y, 0}, {x, y, 0}, {x, y + .22, h}, {x, Y - .22, h}, 0x8D9895,
         camera);
    quad({X, y, 0}, {X, Y, 0}, {X, Y - .22, h}, {X, y + .22, h}, 0xA0ABA4,
         camera);
    activeMaterial = 4;
    for (int i = 0; i < 6; i++) {
      double xx = x + i * p.w / 6;
      cube({xx, y - .015, h * .4, p.w / 12, .03, .18,
            i % 2 ? 0x374445u : 0xCFAC5Au, false},
           camera);
    }
  } else {
    activeMaterial = type % 4 == 0 ? 3 : 4;
    cube(p, camera);
    for (double z : {p.h * .2, p.h * .8})
      cube({p.x - .025, p.y - .025, z, p.w + .05, p.d + .05, .07, 0x40504C},
           camera);
    for (double xx : {p.x + .08, p.x + p.w - .12})
      cube({xx, p.y - .04, .05, .04, .07, p.h - .1, 0xC2A879}, camera);
    segment({p.x + .1, p.y - .055, .15},
            {p.x + p.w - .1, p.y - .055, p.h - .15}, .035, .035, 0xBCA377,
            camera);
    segment({p.x + .1, p.y - .06, p.h - .15}, {p.x + p.w - .1, p.y - .06, .15},
            .035, .035, 0xBCA377, camera);
    if (type % 4 == 1)
      for (int i = 0; i < 5; i++)
        cube({p.x + i * p.w / 5, p.y - .025, .1, .025, .05, p.h - .2, 0x567078},
             camera);
  }
  activeMaterial = previous;
}
