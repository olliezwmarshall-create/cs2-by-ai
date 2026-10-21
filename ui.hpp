#pragma once
#include "game.hpp"
#include <fstream>
#include <sstream>
enum class ScreenPage {
  Home,
  Play,
  Pause,
  Buy,
  Settings,
  Utility,
  Result,
  Roster
};
struct Area {
  int x, y, w, h;
  bool contains(int px, int py) const {
    return px >= x && py >= y && px < x + w && py < y + h;
  }
};
struct Button {
  Area r;
  std::string title, detail;
  int action;
  bool enabled = true;
};
struct Preferences {
  double sensitivity = 1, volume = .7, fov = 90;
  bool shadows = true, invertY = false, fullscreen = false;
  void clamp() {
    sensitivity = std::clamp(sensitivity, .25, 3.);
    volume = std::clamp(volume, 0., 1.);
    fov = std::clamp(fov, 75., 110.);
  }
  void load(const std::string &path) {
    std::ifstream f(path);
    double s, v, angle;
    int sh, inv;
    if (f >> s >> v >> angle >> sh >> inv && std::isfinite(s) &&
        std::isfinite(v) && std::isfinite(angle)) {
      sensitivity = s;
      volume = v;
      fov = angle;
      shadows = sh != 0;
      invertY = inv != 0;
      int fs = 0;
      if (f >> fs)
        fullscreen = fs != 0;
      clamp();
    }
  }
  bool save(const std::string &path) const {
    std::ofstream f(path);
    return bool(f << sensitivity << ' ' << volume << ' ' << fov << ' '
                  << shadows << ' ' << invertY << ' ' << fullscreen << '\n');
  }
};
struct Interface {
  ScreenPage page = ScreenPage::Home, settingsReturn = ScreenPage::Home;
  Preferences prefs;
  int mx = -1, my = -1, selectedUtility = 0, buyTab = 0, lastWeapon = 0;
  double feedbackTime = 0;
  std::string feedback;
  bool quit = false;
  bool capturesMouse() const { return page == ScreenPage::Play; }
  bool simulates() const {
    return page == ScreenPage::Play || page == ScreenPage::Buy ||
           page == ScreenPage::Utility;
  }
  std::vector<Button> buttons(const Game &g) const {
    std::vector<Button> b;
    auto add = [&](int x, int y, int w, int h, const std::string &t,
                   const std::string &d, int id, bool enabled = true) {
      b.push_back({{x, y, w, h}, t, d, id, enabled});
    };
    if (page == ScreenPage::Home) {
      add(70, 255, 320, 62, "PLAY / T SIDE",
          "Plant the bomb. Clear the sector.", 1);
      add(70, 330, 320, 62, "PLAY / CT SIDE",
          "Hold the sites. Defuse the device.", 2);
      add(70, 420, 150, 45, "SETTINGS", "", 3);
      add(235, 420, 155, 45, "QUIT", "", 4);
    }
    if (page == ScreenPage::Pause) {
      add(80, 210, 300, 52, "RESUME", "Return to the match", 5);
      add(80, 275, 300, 52, "LOADOUT", "Weapons and equipment", 6);
      add(80, 340, 300, 52, "SETTINGS", "Mouse / audio / graphics", 3);
      add(80, 405, 140, 45, "ROSTER", "", 7);
      add(235, 405, 145, 45, "MAIN MENU", "", 8);
    }
    if (page == ScreenPage::Buy) {
      const std::array<std::string, 5> tabs = {{"RIFLES / HEAVY",
                                                "SMG / SHOTGUN", "SNIPERS",
                                                "SIDEARMS", "EQUIPMENT"}};
      for (int i = 0; i < 5; i++)
        add(55 + i * 171, 110, 161, 42, tabs[i], "", 20 + i);
      add(760, 40, 150, 42, "BACK TO GAME", "", 5);
      const std::array<std::vector<int>, 4> categories = {
          {{1, 8, 9, 10, 13}, {2, 15, 11, 12}, {3, 4, 14}, {0, 5, 6, 7}}};
      if (buyTab < 4) {
        int n = 0;
        for (int i : categories[buyTab]) {
          int x = 55 + n % 3 * 285, y = 180 + n / 3 * 150;
          auto &w = weapons[i];
          std::string detail = g.owned[i]
                                   ? "OWNED / EQUIP"
                                   : "$" + std::to_string(w.price) + " / " +
                                         std::to_string(w.capacity) + " ROUNDS";
          add(x, y, 265, 128, w.name, detail, 100 + i,
              (g.owned[i] || (g.money >= w.price && g.buyAllowed())) &&
                  g.hp > 0);
          n++;
        }
      } else {
        add(55, 180, 415, 128, "KEVLAR ARMOR", "$650 / REDUCED DAMAGE", 300,
            g.buyAllowed() && g.money >= 650 && g.armor < 100);
        const std::array<std::string, 3> names = {
            {"FRAG GRENADE", "SMOKE GRENADE", "FLASH GRENADE"}};
        const int prices[3] = {300, 300, 200};
        for (int i = 0; i < 3; i++) {
          int n = i + 1;
          add(55 + n % 2 * 435, 180 + n / 2 * 150, 415, 128, names[i],
              "$" + std::to_string(prices[i]) + " / OWNED " +
                  std::to_string(g.utilities[i]),
              310 + i,
              g.buyAllowed() && g.money >= prices[i] &&
                  g.utilities[i] < (i == 2 ? 2 : 1));
        }
      }
    }
    if (page == ScreenPage::Settings) {
      add(725, 40, 180, 45, "SAVE & BACK", "", 9);
      add(560, 392, 240, 48, prefs.shadows ? "SHADOWS: ON" : "SHADOWS: OFF", "",
          30);
      add(80, 456, 340, 48,
          prefs.fullscreen ? "FULLSCREEN: ON" : "FULLSCREEN: OFF", "", 32);
      add(560, 456, 240, 48, prefs.invertY ? "INVERT Y: ON" : "INVERT Y: OFF",
          "", 31);
    }
    if (page == ScreenPage::Utility) {
      add(335, 155, 290, 85, "FRAG",
          "Blast / " + std::to_string(g.utilities[0]) + " remaining", 200,
          g.utilities[0] > 0);
      add(160, 300, 290, 85, "SMOKE",
          "Conceal / " + std::to_string(g.utilities[1]) + " remaining", 201,
          g.utilities[1] > 0);
      add(510, 300, 290, 85, "FLASH",
          "Blind / " + std::to_string(g.utilities[2]) + " remaining", 202,
          g.utilities[2] > 0);
      add(380, 452, 200, 48, "RETURN", "", 5);
    }
    if (page == ScreenPage::Result) {
      add(295, 315, 370, 64, g.matchOver ? "NEW MATCH" : "NEXT ROUND",
          g.matchOver        ? "Start a fresh first-to-seven match"
          : g.round % 6 == 0 ? "HALFTIME / SWITCH SIDES"
                             : "Same team / continue the match",
          10);
      add(295, 395, 180, 48, "SETTINGS", "", 3);
      add(490, 395, 175, 48, "MAIN MENU", "", 8);
    }
    if (page == ScreenPage::Roster)
      add(740, 470, 160, 48, "BACK", "", 5);
    return b;
  }
  void start(Game &g, bool t) {
    g = Game();
    g.attacker = t;
    g.round = 0;
    g.reset();
    g.freeze = 8;
    g.buyOpen = true;
    page = ScreenPage::Buy;
    buyTab = 0;
    feedback.clear();
    lastWeapon = 0;
  }
  void open(ScreenPage p, Game &g) {
    page = p;
    g.buyOpen = p == ScreenPage::Buy;
    g.usingObjective = false;
    g.vx = g.vy = 0;
    g.walking = g.sprinting = false;
  }
  void escape(Game &g) {
    switch (page) {
    case ScreenPage::Home:
      break;
    case ScreenPage::Play:
      open(ScreenPage::Pause, g);
      break;
    case ScreenPage::Settings:
      page = settingsReturn;
      g.buyOpen = page == ScreenPage::Buy;
      break;
    case ScreenPage::Pause:
    case ScreenPage::Buy:
    case ScreenPage::Utility:
    case ScreenPage::Roster:
      open(g.ended ? ScreenPage::Result : ScreenPage::Play, g);
      break;
    case ScreenPage::Result:
      open(ScreenPage::Home, g);
      break;
    }
  }
  void activate(int action, Game &g) {
    if (action == 1 || action == 2) {
      start(g, action == 1);
      return;
    }
    if (action == 3) {
      settingsReturn = page;
      open(ScreenPage::Settings, g);
      return;
    }
    if (action == 4) {
      quit = true;
      return;
    }
    if (action == 5) {
      open(g.ended ? ScreenPage::Result : ScreenPage::Play, g);
      return;
    }
    if (action == 6) {
      open(ScreenPage::Buy, g);
      return;
    }
    if (action == 7) {
      open(ScreenPage::Roster, g);
      return;
    }
    if (action == 8) {
      open(ScreenPage::Home, g);
      return;
    }
    if (action == 9) {
      page = settingsReturn;
      g.buyOpen = page == ScreenPage::Buy;
      return;
    }
    if (action == 10) {
      if (g.matchOver) {
        start(g, g.startAttacker);
        return;
      }
      g.reset();
      g.freeze = 8;
      open(ScreenPage::Buy, g);
      return;
    }
    if (action >= 20 && action <= 24) {
      buyTab = action - 20;
      return;
    }
    if (action == 30) {
      prefs.shadows = !prefs.shadows;
      return;
    }
    if (action == 32) {
      prefs.fullscreen = !prefs.fullscreen;
      return;
    }
    if (action == 31) {
      prefs.invertY = !prefs.invertY;
      return;
    }
    if (action >= 100 && action < 100 + WEAPON_COUNT) {
      int gun = action - 100;
      if (g.boltCycle > 0) {
        feedback = "WAIT FOR BOLT CYCLE";
        feedbackTime = 2;
        return;
      }
      if (g.owned[gun]) {
        lastWeapon = g.weapon;
        g.switchWeapon(gun);
        feedback = "EQUIPPED " + std::string(weapons[gun].name);
      } else if (g.buyGun(gun))
        feedback = "PURCHASED " + std::string(weapons[gun].name);
      else
        feedback = "PURCHASE UNAVAILABLE";
      feedbackTime = 2;
    }
    if (action == 300 || (action >= 310 && action <= 312)) {
      bool ok = action == 300 ? g.buy(4) : g.buyUtility(action - 310);
      feedback = ok ? "EQUIPMENT PURCHASED" : "PURCHASE UNAVAILABLE";
      feedbackTime = 2;
    }
    if (action >= 200 && action <= 202 && g.utilities[action - 200] > 0) {
      g.utility = action - 200;
      g.utilityEquipped = true;
      selectedUtility = g.utility;
      open(ScreenPage::Play, g);
    }
  }
  bool slider(int x, int y) {
    if (page != ScreenPage::Settings)
      return false;
    int row = -1;
    for (int i = 0; i < 3; i++)
      if (Area{330, 185 + i * 68, 470, 40}.contains(x, y))
        row = i;
    if (row < 0)
      return false;
    double t = std::clamp((x - 340) / 450., 0., 1.);
    if (row == 0)
      prefs.sensitivity = .25 + t * 2.75;
    if (row == 1)
      prefs.volume = t;
    if (row == 2)
      prefs.fov = 75 + t * 35;
    prefs.clamp();
    return true;
  }
  void click(int x, int y, Game &g) {
    mx = x;
    my = y;
    if (slider(x, y))
      return;
    for (auto b : buttons(g))
      if (b.enabled && b.r.contains(x, y)) {
        activate(b.action, g);
        return;
      }
  }
  void wheel(int direction, Game &g) {
    if (!direction)
      return;
    if (page == ScreenPage::Play) {
      if (g.hp <= 0) {
        g.spectator++;
        return;
      }
      for (int step = 1; step <= WEAPON_COUNT; step++) {
        int i = (g.weapon + (direction > 0 ? -step : step) + WEAPON_COUNT * 2) %
                WEAPON_COUNT;
        if (g.owned[i]) {
          if (i == g.weapon)
            break;
          lastWeapon = g.weapon;
          g.switchWeapon(i);
          break;
        }
      }
    } else if (page == ScreenPage::Utility) {
      for (int n = 1; n <= 3; n++) {
        int i = (g.utility + (direction > 0 ? -n : n) + 6) % 3;
        if (g.utilities[i] > 0) {
          g.utility = i;
          selectedUtility = i;
          break;
        }
      }
    } else if (page == ScreenPage::Buy)
      buyTab = (buyTab + (direction > 0 ? -1 : 1) + 5) % 5;
    else if (page == ScreenPage::Settings) {
      if (my >= 185 && my < 225)
        prefs.sensitivity += direction * .1;
      else if (my >= 253 && my < 293)
        prefs.volume += direction * .05;
      else if (my >= 321 && my < 361)
        prefs.fov += direction;
      prefs.clamp();
    }
  }
  void update(double dt, Game &g) {
    g.visualTime += dt;
    feedbackTime = std::max(0., feedbackTime - dt);
    if (g.ended && simulates())
      open(ScreenPage::Result, g);
  }
};
Interface ui;
