#pragma once
#include "game.hpp"
#include <fstream>
struct TextureImage {
  unsigned w = 0, h = 0;
  std::vector<uint32_t> pixels;
  uint32_t sample(double u, double v) const {
    if (!w || !h)
      return 0xAAAAAA;
    int x = std::clamp(int((u - floor(u)) * w), 0, int(w) - 1),
        y = std::clamp(int((v - floor(v)) * h), 0, int(h) - 1);
    return pixels[y * w + x];
  }
};
struct ArtVertex {
  Vec p;
  double u, v;
};
struct ArtFace {
  uint32_t a, b, c, material;
};
struct ArtMesh {
  std::vector<TextureImage> images;
  std::vector<ArtVertex> vertices;
  std::vector<ArtFace> faces;
  bool load(const std::string &path) {
    std::ifstream f(path, std::ios::binary);
    if (!f)
      return false;
    char magic[4] = {};
    f.read(magic, 4);
    if (std::string(magic, 4) != "BRM1")
      return false;
    auto number = [&]() {
      uint32_t n = 0;
      f.read(reinterpret_cast<char *>(&n), 4);
      return n;
    };
    unsigned ni = number(), nv = number(), nf = number();
    if (!ni || !nv || !nf || ni > 16 || nv > 100000 || nf > 100000)
      return false;
    std::vector<TextureImage> tex;
    for (unsigned i = 0; i < ni; i++) {
      TextureImage im;
      im.w = number();
      im.h = number();
      if (!im.w || !im.h || im.w > 1024 || im.h > 1024)
        return false;
      im.pixels.resize(im.w * im.h);
      f.read(reinterpret_cast<char *>(im.pixels.data()), im.pixels.size() * 4);
      tex.push_back(std::move(im));
    }
    std::vector<ArtVertex> vs;
    for (unsigned i = 0; i < nv; i++) {
      float a[5];
      f.read(reinterpret_cast<char *>(a), 20);
      for (float n : a)
        if (!std::isfinite(n))
          return false;
      vs.push_back({{a[0], a[1], a[2]}, a[3], a[4]});
    }
    std::vector<ArtFace> fs;
    for (unsigned i = 0; i < nf; i++) {
      ArtFace t{number(), number(), number(), number()};
      if (t.a >= nv || t.b >= nv || t.c >= nv || t.material >= ni)
        return false;
      fs.push_back(t);
    }
    if (!f)
      return false;
    images = std::move(tex);
    vertices = std::move(vs);
    faces = std::move(fs);
    return true;
  }
};
ArtMesh lanternArt, bottleArt;
bool loadArt(const std::string &directory) {
  bool a = lanternArt.load(directory + "/lantern.brm"),
       b = bottleArt.load(directory + "/bottle.brm");
  return a && b;
}
