#pragma once
#include "renderer.hpp"
struct UiText {
  int x, y, size;
  unsigned color;
  std::string text;
};
std::vector<UiText> interfaceText;
void caption(int x, int y, int size, const std::string &text,
             unsigned color = 0xE3ECEA) {
  interfaceText.push_back({x, y, size, color, text});
}
void blendRect(Area r, unsigned color, double alpha) {
  for (int y = std::max(0, r.y); y < std::min(H, r.y + r.h); y++)
    for (int x = std::max(0, r.x); x < std::min(W, r.x + r.w); x++) {
      auto &p = pixels[y * W + x];
      p = rgb(
          int(((p >> 16) & 255) * (1 - alpha) + ((color >> 16) & 255) * alpha),
          int(((p >> 8) & 255) * (1 - alpha) + ((color >> 8) & 255) * alpha),
          int((p & 255) * (1 - alpha) + (color & 255) * alpha));
    }
}
void rounded(Area r, unsigned color, int radius = 7) {
  rect(r.x + radius, r.y, r.w - 2 * radius, r.h, color);
  rect(r.x, r.y + radius, r.w, r.h - 2 * radius, color);
  for (auto p : std::array<Vec, 4>{
           {{double(r.x + radius), double(r.y + radius), 0},
            {double(r.x + r.w - radius - 1), double(r.y + radius), 0},
            {double(r.x + radius), double(r.y + r.h - radius - 1), 0},
            {double(r.x + r.w - radius - 1), double(r.y + r.h - radius - 1),
             0}}})
    ellipse(int(p.x), int(p.y), radius, radius, color);
}
void weaponIcon(int x, int y, int gun, unsigned c) {
  if (secondaryGun(gun)) {
    rect(x + 15, y, gun == 5 ? 80 : 60, 13, c);
    rect(x + 25, y + 10, 15, 30, c);
    rect(x + 75, y + 5, 15, 4, c);
    return;
  }
  int barrel = boltGun(gun) ? 125 : gun == 2 || gun == 15 ? 65 : 100;
  rect(x, y, 90, 12, c);
  rect(x + 80, y + 3, barrel - 60, 5, c);
  rect(x - 25, y + 4, 30, 16, c);
  rect(x + 28, y + 8, 12, 30, c);
  rect(x + (gun == 10 ? 10 : 55), y + 10, 14, 25, c);
  if (scopedGun(gun)) {
    rect(x + 25, y - 15, 55, 9, c);
    rect(x + 35, y - 7, 4, 10, c);
    rect(x + 65, y - 7, 4, 10, c);
  } else {
    rect(x + 70, y - 6, 5, 10, c);
    rect(x + 15, y - 6, 5, 10, c);
  }
  if (gun == 13)
    ellipse(x + 50, y + 25, 15, 12, c);
  if (gun == 8)
    rect(x + 105, y, 35, 12, c);
  if (gun == 11 || gun == 12)
    rect(x + 65, y + 10, 60, 4, c);
}
void drawInterface() {
  interfaceText.clear();
  auto page = ui.page;
  if (page == ScreenPage::Play) {
    caption(180, 20, 16,
            "HARBOR / " + std::string(game.attacker ? "T" : "CT"), 0x9FB6B8);
    rounded({W / 2 - 110, 15, 220, 46}, 0x17252B);
    caption(W / 2 - 92, 24, 22, std::to_string(game.alive(true)) + " T",
            0xEBC57B);
    caption(W / 2 + 35, 24, 22, "CT " + std::to_string(game.alive(false)),
            0x86BCE3);
    caption(180, 46, 15,
            "$" + std::to_string(game.money) + "  /  ROUND " +
                std::to_string(game.round));
    caption(W / 2 - 68, 69, 17,
            game.freeze > 0
                ? "BUY PHASE " + std::to_string(int(ceil(game.freeze)))
            : game.planted
                ? "BOMB " + std::to_string(int(ceil(game.bombClock))) + "s"
                : std::to_string(int(ceil(game.clock))) + "s",
            game.planted ? 0xFFAA76 : 0xE4EBE3);
    caption(180, 91, 13, game.objectiveText(), 0xBDC9C7);
    caption(28, H - 59, 18,
            "+ " + std::to_string(game.hp) + "    ARMOR " +
                std::to_string(game.armor));
    caption(
        W - 265, H - 88, 13,
        std::array<std::string, 3>{{"FRAG", "SMOKE", "FLASH"}}[game.utility] +
            " x" + std::to_string(game.utilities[game.utility]),
        0xBCA77D);
    caption(W - 265, H - 60, 15, game.gun().name, 0x9FB6B8);
    caption(W - 265, H - 40, 22,
            std::to_string(game.ammo()) + " / " +
                std::to_string(game.reserve[game.weapon]));
    caption(180, 120, 12, "ESC MENU / B BUY / MIDDLE: UTILITY / E PICKUP",
            0x8FA5A9);
    int n = 0;
    for (int i = 0; i < WEAPON_COUNT; i++)
      if (game.owned[i]) {
        int x = 300 + n * 70;
        rounded({x, H - 54, 64, 36}, i == game.weapon ? 0xC39B58 : 0x203039, 4);
        caption(x + 10, H - 48, 13, secondaryGun(i) ? "PISTOL" : "PRIMARY",
                i == game.weapon ? 0x14232A : 0xA8B7BB);
        n++;
      }
    for (size_t i = 0; i < game.feed.size(); i++) {
      rounded({660, 115 + int(i) * 24, 280, 22}, 0x263139, 3);
      caption(670, 116 + int(i) * 24, 13, game.feed[i], 0xE4CBA4);
    }
    caption(180, 68, 13,
            "MATCH " + std::to_string(game.wins) + " : " +
                std::to_string(game.losses) + " / FIRST TO 7",
            0xBCA77D);
    if (game.nearbyDrop() >= 0)
      caption(300, H - 145, 15,
              "E: PICK UP " +
                  std::string(weapons[game.drops[game.nearbyDrop()].gun].name),
              0xE5C584);
    if (game.utilityEquipped)
      caption(300, H - 115, 15, "LEFT CLICK: THROW / Q: WEAPON", 0xE5C584);
    if (game.reload > 0)
      caption(W / 2 - 55, H - 116, 15, "RELOADING", 0xE9C77C);
    if (game.boltCycle > 0)
      caption(W / 2 - 60, H - 116, 15, "CYCLING BOLT", 0xE9C77C);
    if (game.hp <= 0) {
      int id = game.spectating();
      caption(300, 180, 19,
              id >= 0
                  ? "SPECTATING " + game.botName(id) + "  /  SCROLL TO CHANGE"
                  : "ELIMINATED / ROUND CONTINUES");
    }
    return;
  }
  blendRect({0, 0, W, H}, 0x081219, page == ScreenPage::Home ? .35 : .82);
  if (page == ScreenPage::Home) {
    for (int x = 0; x < 510; x += 5)
      blendRect({x, 0, 5, H}, 0x09151E, .85 * (1 - x / 540.));
    caption(70, 68, 14, "O R I G I N A L   T A C T I C A L   S H O O T E R",
            0xB7CBC8);
    caption(65, 105, 66, "BREACH", 0xF1EBDB);
    caption(70, 181, 22, "HARBOR / FIRST TO SEVEN", 0xCDAE73);
    caption(70, 218, 14, "Six-round halves. Larger routes. Harder opposition.",
            0xA6BABB);
    caption(70, 552, 13,
            "OFFLINE BOTS  /  ORIGINAL 3D ART  /  MOUSE-DRIVEN LOADOUT",
            0x7F9A9E);
  }
  if (page == ScreenPage::Pause) {
    caption(80, 76, 14, "MATCH PAUSED", 0xCDAE73);
    caption(77, 102, 48, "TAKE A BREATH");
    caption(80, 163, 16, "Your team will be here when you return.", 0xA2B8B9);
  }
  if (page == ScreenPage::Buy) {
    caption(55, 35, 32, "ARMORY");
    caption(260, 47, 19, "$" + std::to_string(game.money), 0xE3C27F);
    caption(55, 530, 15,
            game.freeze > 0 ? "FREEZE TIME / " +
                                  std::to_string(int(ceil(game.freeze))) + "s"
            : game.buyAllowed()
                ? "BUY ZONE / PURCHASES AVAILABLE"
                : "OUTSIDE BUY WINDOW / OWNED WEAPONS STILL AVAILABLE",
            0xAFBFBE);
    caption(55, 555, 13,
            "Click an item to purchase or equip. Scroll to change category. "
            "Combat continues.",
            0x88A3A7);
  }
  if (page == ScreenPage::Settings) {
    caption(70, 55, 36, "SETTINGS");
    caption(70, 108, 15, "Tune your view. Changes apply immediately.",
            0xA6BABB);
    std::array<std::string, 3> labels = {
        {"MOUSE SENSITIVITY", "MASTER VOLUME", "FIELD OF VIEW"}};
    std::array<double, 3> values = {{(ui.prefs.sensitivity - .25) / 2.75,
                                     ui.prefs.volume,
                                     (ui.prefs.fov - 75) / 35}};
    for (int i = 0; i < 3; i++) {
      int y = 185 + i * 68;
      caption(80, y + 6, 14, labels[i], 0xB6C9C8);
      rounded({340, y + 16, 450, 5}, 0x3C4F58, 2);
      rect(340, y + 16, int(values[i] * 450), 5, 0xC4A36A);
      ellipse(340 + int(values[i] * 450), y + 18, 8, 8, 0xE1C68F);
      std::string value =
          i == 0   ? std::to_string(int(ui.prefs.sensitivity * 100)) + "%"
          : i == 1 ? std::to_string(int(ui.prefs.volume * 100)) + "%"
                   : std::to_string(int(ui.prefs.fov)) + " deg";
      caption(820, y + 6, 15, value);
    }
  }
  if (page == ScreenPage::Utility) {
    caption(315, 52, 32, "TACTICAL UTILITY");
    caption(260, 99, 15, "Move the mouse and release middle button to select.",
            0xA6BABB);
    ellipse(W / 2, 300, 42, 42, 0x293D44);
    caption(W / 2 - 11, 288, 18, std::to_string(game.utilities[game.utility]));
    caption(325, 530, 14,
            "Scroll or hover to select. Left click throws. H quick-throws.",
            0x9BB5B7);
  }
  if (page == ScreenPage::Result) {
    caption(295, 130, 14, game.matchOver ? "MATCH COMPLETE" : "ROUND COMPLETE",
            0xCDAE73);
    caption(290, 162, 48,
            game.matchOver ? (game.matchWon ? "MATCH WON" : "MATCH LOST")
                           : (game.won ? "ROUND WON" : "ROUND LOST"),
            game.won ? 0xB7D6BC : 0xDEAB91);
    caption(295, 227, 17, game.result);
    caption(295, 262, 16,
            "FIRST TO 7 / " + std::to_string(game.wins) + " : " +
                std::to_string(game.losses) + "    KILLS " +
                std::to_string(game.kills) + "    $" +
                std::to_string(game.money),
            0xA6BABB);
  }
  if (page == ScreenPage::Roster) {
    caption(80, 58, 36, "TEAM ROSTER");
    caption(80, 125, 18, "COUNTER-TERRORISTS", 0x86BCE3);
    caption(510, 125, 18, "TERRORISTS", 0xEBC57B);
    for (size_t i = 0; i < game.bots.size(); i++) {
      auto &b = game.bots[i];
      int x = b.terrorist ? 510 : 80, y = 175 + b.role * 45;
      rounded({x - 10, y - 5, 365, 38}, 0x1E3039);
      caption(x, y, 15,
              game.botName(int(i)) + " / " +
                  (b.hp > 0 ? b.state : "ELIMINATED") + " / K " +
                  std::to_string(b.frags));
    }
    caption(80, 435, 16,
            "YOU / " + std::string(game.attacker ? "T" : "CT") + " / HP " +
                std::to_string(game.hp) + " / KILLS " +
                std::to_string(game.kills));
  }
  for (auto b : ui.buttons(game)) {
    bool hover = b.r.contains(ui.mx, ui.my);
    unsigned color = !b.enabled ? 0x202C32 : hover ? 0x3B5059 : 0x223640;
    if ((page == ScreenPage::Buy && b.action == 20 + ui.buyTab) ||
        (page == ScreenPage::Utility && b.action == 200 + game.utility))
      color = 0x415858;
    rounded(b.r, color);
    rect(b.r.x, b.r.y + 8, 3, b.r.h - 16,
         !b.enabled ? 0x526168
         : hover    ? 0xE4C38A
                    : 0xB79A65);
    int size = b.r.h > 70 ? 20 : 15;
    caption(b.r.x + 18, b.r.y + (b.r.h > 70 ? 17 : 13), size, b.title,
            b.enabled ? 0xE3ECEA : 0x75878D);
    if (!b.detail.empty())
      caption(b.r.x + 18, b.r.y + (b.r.h > 70 ? b.r.h - 31 : 38), 13, b.detail,
              b.enabled ? 0xA4BBBD : 0x647B82);
    if (page == ScreenPage::Buy && b.action >= 100 &&
        b.action < 100 + WEAPON_COUNT) {
      weaponIcon(b.r.x + b.r.w - 172, b.r.y + 54, b.action - 100,
                 b.enabled ? 0xB8C5C4 : 0x5F7075);
      double damage = weapons[b.action - 100].damage / 110.;
      rect(b.r.x + 18, b.r.y + 80, 60, 3, 0x4E5E61);
      rect(b.r.x + 18, b.r.y + 80, int(damage * 60), 3, 0xBFA36D);
    }
  }
  if (ui.feedbackTime > 0) {
    rounded({310, 545, 580, 35}, 0x2A4345);
    caption(325, 553, 14, ui.feedback, 0xD6DCC1);
  }
}
