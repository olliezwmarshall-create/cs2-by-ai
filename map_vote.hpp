#pragma once
#include "game.hpp"
struct MapVote {
  std::array<int, 11> ballots{};
  double elapsed = 0;
  bool running = false;
  int human = -1;
  uint32_t seed = 417;
  void begin(uint32_t salt = 0) {
    ballots.fill(-1);
    elapsed = 0;
    running = true;
    human = -1;
    seed = 417 + salt * 997;
  }
  void choose(int map) {
    if (running && map >= 0 && map < 3) {
      human = map;
      ballots[0] = map;
    }
  }
  std::array<int, 3> totals() const {
    std::array<int, 3> n{};
    for (int b : ballots)
      if (b >= 0 && b < 3)
        n[b]++;
    return n;
  }
  bool botsDone() const {
    for (int i = 1; i < 11; i++)
      if (ballots[i] < 0)
        return false;
    return true;
  }
  int winner() const {
    auto n = totals();
    int high = *std::max_element(n.begin(), n.end());
    if (human >= 0 && n[human] == high)
      return human;
    for (int i = 0; i < 3; i++)
      if (n[i] == high)
        return i;
    return 0;
  }
  void update(double dt) {
    if (!running)
      return;
    elapsed += std::max(0., dt);
    for (int i = 1; i < 11; i++)
      if (ballots[i] < 0 && elapsed >= .4 + i * .45) {
        seed = seed * 1664525 + 1013904223;
        ballots[i] = int((seed >> 10) % 3);
      }
  }
  bool expired() const { return running && elapsed >= 10; }
  void close() { running = false; }
};
