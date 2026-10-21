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
      add(55, 110, 185, 42, "WEAPONS", "", 20);
      add(250, 110, 185, 42, "EQUIPMENT", "", 21);
      add(760, 40, 150, 42, "BACK TO GAME", "", 5);
      if (buyTab == 0) {
        for (int i = 1; i < 5; i++) {
          int n = i - 1, x = 55 + n % 2 * 435, y = 180 + n / 2 * 150;
          auto &w = weapons[i];
          std::string detail = g.owned[i]
                                   ? "OWNED / CLICK TO EQUIP"
                                   : "$" + std::to_string(w.price) + "  /  " +
                                         std::to_string(w.capacity) + " ROUNDS";
          add(x, y, 415, 128, w.name, detail, 100 + i,
              (g.owned[i] || (g.money >= w.price && g.buyAllowed())) &&
                  g.hp > 0);
        }
      } else {
        add(55, 180, 415, 128, "KEVLAR ARMOR", "$650 / REDUCED INCOMING DAMAGE",
            110, g.buyAllowed() && g.money >= 650 && g.armor < 100);
        add(490, 180, 415, 128, "UTILITY PACK", "$300 / FRAG + SMOKE + FLASH",
            111, g.buyAllowed() && g.money >= 300);
        add(55, 330, 850, 110, "SERVICE PISTOL", "ALWAYS OWNED / EQUIP", 100,
            g.hp > 0);
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
      add(295, 315, 370, 64, "NEXT ROUND",
          "Switch sides / replenish ammunition", 10);
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
      g.reset();
      g.freeze = 8;
      open(ScreenPage::Buy, g);
      return;
    }
    if (action == 20 || action == 21) {
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
    if (action >= 100 && action <= 104) {
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
      } else if (g.buy(gun == 4 ? 6 : gun))
        feedback = "PURCHASED " + std::string(weapons[gun].name);
      else
        feedback = "PURCHASE UNAVAILABLE";
      feedbackTime = 2;
    }
    if (action == 110 || action == 111) {
      feedback = g.buy(action == 110 ? 4 : 5) ? "EQUIPMENT PURCHASED"
                                              : "PURCHASE UNAVAILABLE";
      feedbackTime = 2;
    }
    if (action >= 200 && action <= 202 && g.utilities[action - 200] > 0) {
      g.utility = action - 200;
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
      for (int step = 1; step <= 5; step++) {
        int i = (g.weapon + (direction > 0 ? -step : step) + 10) % 5;
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
      buyTab = 1 - buyTab;
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
    feedbackTime = std::max(0., feedbackTime - dt);
    if (g.ended && simulates())
      open(ScreenPage::Result, g);
  }
};
Interface ui;
