#pragma once
// Both teams run the same perception, navigation and objective logic. Shared
// memory contains sightings/heard shots, never hidden target positions.
void Game::tickBots(double dt) {
  int planter = -1, defuser = -1;
  double closest = 1e9;
  for (size_t i = 0; i < bots.size(); i++) {
    auto &b = bots[i];
    if (b.hp <= 0)
      continue;
    if (b.terrorist && planter < 0)
      planter = int(i);
    if (!b.terrorist && planted) {
      double d = dist(b.x, b.y, bombX, bombY);
      if (d < closest) {
        closest = d;
        defuser = int(i);
      }
    }
  }
  for (size_t i = 0; i < bots.size(); i++) {
    auto &b = bots[i];
    if (b.hp <= 0)
      continue;
    b.flash = std::max(0., b.flash - dt);
    b.blind = std::max(0., b.blind - dt);
    b.suppression = std::max(0., b.suppression - dt);
    b.cooldown -= dt;
    b.alert = std::max(0., b.alert - dt);
    b.repath -= dt;
    b.communicate -= dt;
    b.coverTime -= dt;
    const auto &gun = weapons[b.gun];
    if (b.reload > 0) {
      b.reload = std::max(0., b.reload - dt);
      if (b.reload == 0)
        b.ammo = gun.capacity;
    }
    b.senseTimer -= dt;
    double tx = 0, ty = 0, best = 50;
    int target = -2;
    auto consider = [&](double xx, double yy, int id) {
      double d = dist(b.x, b.y, xx, yy);
      bool fov = std::abs(wrap(atan2(yy - b.y, xx - b.x) - b.facing)) < 2.1 ||
                 b.alert > 0 || d < 3;
      if (b.blind <= 0 && fov && d < std::min(best, gun.range) &&
          visible(b.x, b.y, xx, yy)) {
        best = d;
        tx = xx;
        ty = yy;
        target = id;
      }
    };
    if (b.senseTimer <= 0) {
      if (hp > 0 && b.terrorist != attacker)
        consider(x, y, -1);
      for (size_t j = 0; j < bots.size(); j++)
        if (bots[j].hp > 0 && bots[j].terrorist != b.terrorist)
          consider(bots[j].x, bots[j].y, int(j));
      b.senseTimer = .08 + b.role * .008;
      b.senseX = tx;
      b.senseY = ty;
    } else if (b.blind <= 0 && b.target != -2) {
      bool aliveTarget = b.target == -1 ? hp > 0 : bots[b.target].hp > 0;
      if (aliveTarget) {
        target = b.target;
        tx = b.senseX;
        ty = b.senseY;
        best = dist(b.x, b.y, tx, ty);
      } else
        b.senseTimer = 0;
    }
    bool sees = target != -2;
    if (target != b.target)
      b.reaction = .18 + rng() * .22;
    b.target = target;
    b.reaction = std::max(0., b.reaction - dt);
    if (sees) {
      b.lastX = tx;
      b.lastY = ty;
      b.alert = 6;
      b.knows = true;
      b.facing = atan2(ty - b.y, tx - b.x);
      if (b.communicate <= 0) {
        b.communicate = .5;
        for (auto &friendBot : bots)
          if (friendBot.hp > 0 && friendBot.terrorist == b.terrorist &&
              dist(friendBot.x, friendBot.y, b.x, b.y) < 12) {
            friendBot.knows = true;
            friendBot.lastX = tx;
            friendBot.lastY = ty;
            friendBot.alert = 5;
          }
      }
    }
    // A support throws one smoke when taking fire; an entry bot flashes a long
    // lane before pushing. Utility observes the same physics as the player's.
    if (sees && !b.usedUtility && time > 4 && best > 5 && best < 16 &&
        (b.role == 1 || b.role == 2)) {
      int kind = b.role == 1 ? 2 : 1;
      Vec dir = direction(b.facing);
      grenades.push_back(
          {{b.x, b.y, 1.4}, {dir.x * 6, dir.y * 6, 3}, kind, 1.25, int(i)});
      b.usedUtility = true;
    }
    double gx = b.lastX, gy = b.lastY;
    bool doingObjective = false;
    int chosenSite = (round % 2) ? 5 : 26;
    if (planted) {
      if (!b.terrorist && int(i) == defuser) {
        gx = bombX;
        gy = bombY;
        doingObjective = true;
        b.state = "DEFUSE";
      } else {
        double angle = b.role * 1.25;
        double radius = b.terrorist ? 3.5 : 2.5;
        gx = bombX + cos(angle) * radius;
        gy = bombY + sin(angle) * radius;
        b.state = b.terrorist ? "HOLD BOMB" : "COVER DEFUSER";
      }
    } else if (b.terrorist) {
      gx = 27;
      gy = chosenSite;
      b.state = int(i) == planter ? "PLANT" : "ESCORT";
      doingObjective = true;
      // Flankers use the other connector before converging on the bomb site.
      if (b.role == 3 && b.x < 18) {
        gx = 16;
        gy = chosenSite == 5 ? 19 : 14;
        b.state = "FLANK";
      }
      if (sees && best < 12 && int(i) != planter) {
        gx = tx;
        gy = ty;
        doingObjective = false;
        b.state = "ENGAGE";
      }
    } else {
      gx = b.role == 2 ? 16 : 27;
      gy = b.role % 2 ? 26 : 5;
      b.state = "GUARD";
      if (b.knows && b.alert > 0) {
        gx = b.lastX;
        gy = b.lastY;
        b.state = "INVESTIGATE";
      }
    }
    bool seekingCover = (b.hp < 35 || b.reload > 0) && b.knows && b.alert > 0;
    if (seekingCover) {
      if (b.coverTime <= 0) {
        double score = 1e9;
        b.coverX = b.x;
        b.coverY = b.y;
        for (int dy = -4; dy <= 4; dy++)
          for (int dx = -4; dx <= 4; dx++) {
            double cx = int(b.x) + dx + .5, cy = int(b.y) + dy + .5;
            if (arena.solid(cx, cy, .25) || clearLine(cx, cy, b.lastX, b.lastY))
              continue;
            double cost = dist(cx, cy, b.x, b.y);
            if (cost < score) {
              score = cost;
              b.coverX = cx;
              b.coverY = cy;
            }
          }
        b.coverTime = 1;
      }
      gx = b.coverX;
      gy = b.coverY;
      doingObjective = false;
      b.state = "TAKE COVER";
    }
    bool canPlant = b.terrorist && !planted && int(i) == planter &&
                    site(b.x, b.y) && (!sees || best > 8);
    bool canDefuse = !b.terrorist && planted && int(i) == defuser &&
                     dist(b.x, b.y, bombX, bombY) < 1.6 && (!sees || best > 7);
    bool hold = sees && !seekingCover && !doingObjective && best > 3 &&
                best < (b.gun == 3 ? 30 : 13);
    bool objectiveHold = canPlant || canDefuse;
    if (b.blind <= 0 && !objectiveHold) {
      if (!hold && dist(b.x, b.y, gx, gy) > .4) {
        if (b.repath <= 0) {
          b.path = route(b.x, b.y, gx, gy);
          b.repath = .4 + b.role * .045;
        }
        if (!b.path.empty()) {
          int p = b.path.front();
          double nx = p % MAP + .5, ny = p / MAP + .5,
                 d = dist(b.x, b.y, nx, ny);
          if (d < .13)
            b.path.erase(b.path.begin());
          else {
            double speed = (seekingCover ? 2. : b.gun == 3 ? 1.1 : 1.7) * dt;
            double xx = b.x + (nx - b.x) / d * std::min(speed, d),
                   yy = b.y + (ny - b.y) / d * std::min(speed, d);
            if (!arena.solid(xx, b.y, .22))
              b.x = xx;
            if (!arena.solid(b.x, yy, .22))
              b.y = yy;
            if (!sees)
              b.facing = atan2(ny - b.y, nx - b.x);
          }
        }
      } else if (hold && b.gun != 3) {
        double strafe = sin(time * 1.7 + i * 2) * dt * .8;
        double xx = b.x - (ty - b.y) / std::max(1., best) * strafe,
               yy = b.y + (tx - b.x) / std::max(1., best) * strafe;
        if (!arena.solid(xx, yy, .23)) {
          b.x = xx;
          b.y = yy;
        }
      }
      // Soft separation avoids team stacks without making narrow lanes
      // impassable.
      for (size_t j = 0; j < bots.size(); j++) {
        auto &other = bots[j];
        if (i == j || other.hp <= 0)
          continue;
        double d = dist(b.x, b.y, other.x, other.y);
        if (d < .58 && d > .001) {
          double push = dt * (.58 - d) * 2,
                 xx = b.x + (b.x - other.x) / d * push,
                 yy = b.y + (b.y - other.y) / d * push;
          if (!arena.solid(xx, yy, .23)) {
            b.x = xx;
            b.y = yy;
          }
        }
      }
    }
    if (b.blind > 0) {
      b.plant = 0;
      b.state = "BLINDED";
      continue;
    }
    if (objectiveHold) {
      b.plant += dt;
      b.state = canPlant ? "PLANTING" : "DEFUSING";
      if (canPlant && b.plant >= 3 && !planted) {
        planted = true;
        bombX = b.x;
        bombY = b.y;
        bombClock = 40;
        plantProgress = 0;
        sounds.push_back(BombBeep);
        log(botName(int(i)) + " PLANTED THE BOMB");
        for (auto &other : bots) {
          other.plant = 0;
          other.repath = 0;
        }
      } else if (canDefuse && b.plant >= 5) {
        finishTeam(false, "CT WIN - BOMB DEFUSED");
        return;
      }
    } else
      b.plant = 0;
    if (b.ammo <= 0 && b.reload <= 0) {
      b.reload = gun.reload;
      b.state = "RELOAD";
    }
    if (!sees || !visible(b.x, b.y, tx, ty) || b.cooldown > 0 ||
        b.reaction > 0 || b.reload > 0 || objectiveHold)
      continue;
    // Don't fire through a teammate, even though friendly damage is disabled.
    double dx = tx - b.x, dy = ty - b.y, den = dx * dx + dy * dy;
    bool blocked = false;
    for (auto &other : bots)
      if (&other != &b && other.hp > 0 && other.terrorist == b.terrorist) {
        double t =
            den ? ((other.x - b.x) * dx + (other.y - b.y) * dy) / den : 0;
        if (t > .02 && t < .98 &&
            dist(other.x, other.y, b.x + t * dx, b.y + t * dy) < .32)
          blocked = true;
      }
    if (hp > 0 && attacker == b.terrorist) {
      double t = den ? ((x - b.x) * dx + (y - b.y) * dy) / den : 0;
      if (t > .02 && t < .98 && dist(x, y, b.x + t * dx, b.y + t * dy) < .32)
        blocked = true;
    }
    if (blocked) {
      b.state = "WAIT FIRE LANE";
      continue;
    }
    b.ammo--;
    b.flash = .055;
    b.cooldown = b.gun == 3 ? 1.65 : .11 + rng() * .13;
    if (b.gun != 3 && b.ammo % 4 == 0)
      b.cooldown += .35; // controlled bursts
    tracers.push_back(
        {{b.x, b.y, 1.4}, {tx, ty, 1.35}, .09, b.terrorist != attacker});
    sounds.push_back(EnemyShot);
    for (auto &other : bots)
      if (other.hp > 0 && other.terrorist != b.terrorist &&
          dist(other.x, other.y, b.x, b.y) < 22) {
        other.knows = true;
        other.lastX = b.x;
        other.lastY = b.y;
        other.alert = 5;
      }
    double accuracy =
        (b.gun == 3 ? .65 : .52) - best * .013 - b.suppression * .18;
    bool hitTarget = rng() < std::clamp(accuracy, .08, .65);
    if (hitTarget) {
      int damage = b.gun == 3 ? 65 : b.gun == 2 ? 11 : 17;
      if (target == -1 && hp > 0) {
        if (armor > 0) {
          damage = int(damage * .65);
          armor = std::max(0, armor - 5);
        }
        hp = std::max(0, hp - damage);
        hurt = .25;
        if (hp == 0)
          log(botName(int(i)) + " > YOU");
      } else if (target >= 0)
        damageBot(target, damage, int(i));
    }
    if (checkElimination())
      return;
  }
}
