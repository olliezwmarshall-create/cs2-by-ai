#include "renderer.hpp"
#include <fstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <mmsystem.h>
// Procedural PCM sounds mixed in real time: no downloaded assets or runtime
// files.
struct Audio {
  struct Voice {
    Sound kind;
    int pos = 0;
  };
  HWAVEOUT device = nullptr;
  static constexpr int N = 1024;
  std::array<std::array<short, N>, 3> buffers{};
  std::array<WAVEHDR, 3> headers{};
  std::vector<Voice> voices;
  uint32_t seed = 193;
  Audio() {
    WAVEFORMATEX f = {};
    f.wFormatTag = WAVE_FORMAT_PCM;
    f.nChannels = 1;
    f.nSamplesPerSec = 22050;
    f.wBitsPerSample = 16;
    f.nBlockAlign = 2;
    f.nAvgBytesPerSec = 44100;
    if (waveOutOpen(&device, WAVE_MAPPER, &f, 0, 0, CALLBACK_NULL) !=
        MMSYSERR_NOERROR) {
      device = nullptr;
      return;
    }
    for (int i = 0; i < 3; i++) {
      headers[i].lpData = reinterpret_cast<char *>(buffers[i].data());
      headers[i].dwBufferLength = N * 2;
      waveOutPrepareHeader(device, &headers[i], sizeof(WAVEHDR));
      headers[i].dwFlags |= WHDR_DONE;
    }
  }
  double duration(Sound s) {
    switch (s) {
    case Explosion:
      return .8;
    case BombBeep:
      return .1;
    case WinSound:
      return .6;
    case Rifle:
      return .32;
    case EnemyShot:
      return .22;
    case MagOut:
      return .23;
    case MagIn:
      return .16;
    case Bolt:
      return .23;
    case Step:
      return .13;
    case Hit:
      return .12;
    default:
      return .07;
    }
  }
  double sample(Voice v) {
    double t = v.pos / 22050.;
    seed = 1664525 * seed + 1013904223;
    double noise = double(seed >> 8) / 8388608. - 1.;
    switch (v.kind) {
    case Explosion:
      return (noise * .7 + sin(2 * PI * 48 * t) * .5) * exp(-t * 6);
    case BombBeep:
      return sin(2 * PI * 1800 * t) * .25 * exp(-t * 12);
    case WinSound:
      return sin(2 * PI * (440 + int(t * 6) * 110) * t) * .2 * exp(-t * 3);
    case Rifle:
      return (noise * .7 + sin(2 * PI *
                               ((game.weapon == 0   ? 145
                                 : game.weapon == 2 ? 190
                                 : game.weapon == 3 ? 60
                                                    : 85) *
                                    t -
                                70 * t * t)) *
                               .45) *
             exp(-t * 23);
    case EnemyShot:
      return (noise * .5 + sin(2 * PI * 95 * t) * .25) * exp(-t * 28);
    case Step:
      return (noise * .25 + sin(2 * PI * 75 * t) * .4) * exp(-t * 30);
    case MagOut:
      return noise * .3 * exp(-t * 18) +
             sin(2 * PI * 430 * t) * exp(-t * 60) * .3;
    case MagIn:
      return noise * .55 * exp(-t * 55) +
             sin(2 * PI * 160 * t) * .4 * exp(-t * 45);
    case Bolt:
      return noise * .4 * (exp(-t * 55) + .8 * exp(-std::abs(t - .1) * 100));
    case Hit:
      return sin(2 * PI * 1400 * t) * exp(-t * 40) * .15;
    default:
      return noise * .2 * exp(-t * 90);
    }
  }
  void pump() {
    if (!device) {
      game.sounds.clear();
      return;
    }
    for (auto s : game.sounds)
      if (voices.size() < 32)
        voices.push_back({s, 0});
    game.sounds.clear();
    for (int i = 0; i < 3; i++)
      if (headers[i].dwFlags & WHDR_DONE) {
        for (auto &out : buffers[i]) {
          double sum = 0;
          for (auto &v : voices) {
            if (v.pos < duration(v.kind) * 22050)
              sum += sample(v);
            v.pos++;
          }
          out = short(std::clamp(sum * .55, -1., 1.) * 30000);
        }
        voices.erase(std::remove_if(voices.begin(), voices.end(),
                                    [&](Voice v) {
                                      return v.pos >= duration(v.kind) * 22050;
                                    }),
                     voices.end());
        waveOutWrite(device, &headers[i], sizeof(WAVEHDR));
      }
  }
  ~Audio() {
    if (device) {
      waveOutReset(device);
      for (auto &h : headers)
        waveOutUnprepareHeader(device, &h, sizeof(h));
      waveOutClose(device);
    }
  }
};
bool keys[256] = {}, mouse = false, captured = false;
bool fireWasDown = false, pressPending = false;
HFONT font = nullptr;
void capture(HWND win, bool on) {
  captured = on;
  fireWasDown = pressPending = false;
  mouse = false;
  aiming = false;
  game.usingObjective = false;
  if (on) {
    RECT r;
    GetClientRect(win, &r);
    POINT p = {r.left, r.top}, q = {r.right, r.bottom};
    ClientToScreen(win, &p);
    ClientToScreen(win, &q);
    RECT clip = {p.x, p.y, q.x, q.y};
    ClipCursor(&clip);
    SetCapture(win);
    ShowCursor(FALSE);
  } else {
    ClipCursor(nullptr);
    ReleaseCapture();
    ShowCursor(TRUE);
    for (bool &k : keys)
      k = false;
  }
}
void label(HDC dc, int x, int y, const std::string &s) {
  TextOutA(dc, x, y, s.c_str(), int(s.size()));
}
LRESULT CALLBACK proc(HWND win, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
  case WM_DESTROY:
    if (captured)
      capture(win, false);
    PostQuitMessage(0);
    return 0;
  case WM_KILLFOCUS:
    if (captured)
      capture(win, false);
    return 0;
  case WM_INPUT:
    if (captured) {
      RAWINPUT raw = {};
      UINT size = sizeof(raw);
      if (GetRawInputData(reinterpret_cast<HRAWINPUT>(l), RID_INPUT, &raw,
                          &size, sizeof(RAWINPUTHEADER)) != UINT(-1) &&
          raw.header.dwType == RIM_TYPEMOUSE &&
          !(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
        game.a = wrap(game.a + raw.data.mouse.lLastX * .0022 *
                                   (aiming ? 630 / focal() : 1));
        game.pitch = std::clamp(game.pitch - raw.data.mouse.lLastY * .0022 *
                                                 (aiming ? 630 / focal() : 1),
                                -1.15, 1.15);
      }
    }
    return 0;
  case WM_KEYDOWN:
    if (w < 256)
      keys[w] = true;
    if (w == VK_ESCAPE && captured)
      capture(win, false);
    else if (w == VK_ESCAPE)
      DestroyWindow(win);
    if (!(l & (1LL << 30))) {
      if (w == 'R')
        game.startReload();
      if (w == 'B' && game.buyAllowed())
        game.buyOpen = !game.buyOpen;
      if (w >= '1' && w <= '6') {
        if (game.buyOpen)
          game.buy(int(w - '0'));
        else if (w <= '5')
          game.switchWeapon(int(w - '1'));
      }
      if (w == 'V' && game.hp <= 0)
        game.spectator++;
      if (w == 'Q')
        game.utility = (game.utility + 1) % 3;
      if (w == 'G')
        game.throwUtility();
      if (w == VK_RETURN && game.ended) {
        game.reset();
        mouse = false;
      }
    }
    return 0;
  case WM_KEYUP:
    if (w < 256)
      keys[w] = false;
    return 0;
  case WM_LBUTTONDOWN:
    if (!captured)
      capture(win, true);
    else
      mouse = true;
    return 0;
  case WM_LBUTTONUP:
    mouse = false;
    return 0;
  case WM_RBUTTONDOWN:
    if (captured)
      aiming = true;
    return 0;
  case WM_RBUTTONUP:
    aiming = false;
    return 0;
  case WM_ENTERSIZEMOVE:
    if (captured)
      capture(win, false);
    return 0;
  case WM_ERASEBKGND:
    return 1;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(win, &ps);
    render();
    HDC back = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, W, H);
    auto old = SelectObject(back, bitmap);
    BITMAPINFO bi = {};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = W;
    bi.bmiHeader.biHeight = -H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    SetDIBitsToDevice(back, 0, 0, W, H, 0, 0, 0, H, pixels.data(), &bi,
                      DIB_RGB_COLORS);
    auto oldfont = SelectObject(back, font);
    SetBkMode(back, TRANSPARENT);
    SetTextColor(back, RGB(210, 226, 220));
    label(back, 126, 30, "A");
    label(back, 126, 114, "B");
    label(back, 175, 20,
          "BREACH / DOCKYARD   " +
              std::string(game.attacker ? "ATTACK" : "DEFEND"));
    label(back, 175, 43,
          "ROUND " + std::to_string(game.round) + "   TIME " +
              (game.planted ? std::string("FROZEN")
                            : std::to_string(int(std::max(0., game.clock)))) +
              "   T " + std::to_string(game.alive(true)) + " / CT " +
              std::to_string(game.alive(false)) + "   $" +
              std::to_string(game.money));
    label(back, 175, 67, game.objectiveText());
    label(back, 175, 91,
          "SCORE " + std::to_string(game.wins) + " : " +
              std::to_string(game.losses) +
              "   B: buy   1-5: weapons   Q: utility   G: throw");
    label(back, 28, H - 58,
          "HP " + std::to_string(game.hp) + "  ARMOR " +
              std::to_string(game.armor));
    label(back, W - 265, H - 60, game.gun().name);
    label(back, W - 265, H - 39,
          std::to_string(game.ammo()) + " / " +
              std::to_string(game.reserve[game.weapon]) + "    " +
              std::array<std::string, 3>{
                  {"FRAG", "SMOKE", "FLASH"}}[game.utility] +
              " x" + std::to_string(game.utilities[game.utility]));
    if (game.reload > 0)
      label(back, W / 2 - 65, H - 65, "RELOADING...");
    if (game.plantProgress > 0)
      label(back, W / 2 - 65, H - 125, "PLANTING...");
    if (game.defuseProgress > 0)
      label(back, W / 2 - 65, H - 125, "DEFUSING...");
    if (game.buyOpen) {
      label(back, 268, 165, "SUPPLY / BUY ZONE - B TO CLOSE");
      label(back, 268, 202, "1 CARBINE $2400   2 SMG $1200");
      label(back, 268, 232, "3 BOLT SNIPER $3500  4 ARMOR $650");
      label(back, 268, 262, "5 UTILITY $300    6 SCOUT DMR $2800");
      label(back, 268, 300, "Purchase in spawn during first 22 seconds.");
      label(back, 268, 330,
            "Pistol always available. Equipment persists on wins.");
      label(back, 268, 360,
            "Weapons: 1 Pistol / 2 Carbine / 3 SMG / 4 Bolt / 5 DMR");
    }
    if (!captured) {
      label(back, 270, 440, "CLICK TO CAPTURE MOUSE / RESUME");
      label(back, 165, 465,
            "WASD move | Mouse aim | Click fire | Right click zoom | R reload");
      label(back, 180, 488,
            "F hold plant / defuse | Shift sprint | Esc release mouse / quit");
    }
    if (game.hp <= 0 && !game.ended) {
      int id = game.spectating();
      label(back, 280, 180,
            id >= 0 ? "SPECTATING " + game.botName(id) + " | V: NEXT TEAMMATE"
                    : "ELIMINATED - ROUND CONTINUES");
    }
    for (size_t i = 0; i < game.feed.size(); i++)
      label(back, 630, 125 + int(i) * 20, game.feed[i]);
    if (game.boltCycle > 0)
      label(back, W / 2 - 70, H - 65, "CYCLING BOLT...");
    if (game.ended)
      label(back, 245, 210, game.result + " - ENTER: NEXT ROUND");
    if (keys[VK_TAB]) {
      label(back, 200, 170, "TEAM ROSTER / STATUS / KILLS");
      for (size_t i = 0; i < game.bots.size(); i++) {
        auto &b = game.bots[i];
        label(back, 180 + int(b.terrorist) * 370, 200 + b.role * 30,
              game.botName(int(i)) + " " + (b.hp > 0 ? b.state : "DEAD") +
                  "  " + std::to_string(b.frags));
      }
      label(back, 200, 375,
            "YOU " + std::string(game.attacker ? "T" : "CT") + "  HP " +
                std::to_string(game.hp) + "  KILLS " +
                std::to_string(game.kills));
    }
    RECT r;
    GetClientRect(win, &r);
    StretchBlt(dc, 0, 0, r.right, r.bottom, back, 0, 0, W, H, SRCCOPY);
    SelectObject(back, oldfont);
    SelectObject(back, old);
    DeleteObject(bitmap);
    DeleteDC(back);
    EndPaint(win, &ps);
    return 0;
  }
  }
  return DefWindowProcA(win, msg, w, l);
}
int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int) {
  WNDCLASSA wc = {};
  wc.lpfnWndProc = proc;
  wc.hInstance = inst;
  wc.lpszClassName = "BreachArena";
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&wc);
  RECT r = {0, 0, W, H};
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  font = CreateFontA(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                     ANTIALIASED_QUALITY, FIXED_PITCH, "Consolas");
  HWND win = CreateWindowA(wc.lpszClassName, "Breach - Dockyard 3D",
                           WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                           CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                           nullptr, nullptr, inst, nullptr);
  if (!win)
    return 1;
  RAWINPUTDEVICE rid = {0x01, 0x02, 0, win};
  RegisterRawInputDevices(&rid, 1, sizeof(rid));
  Audio audio;
  LARGE_INTEGER freq, last, now;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&last);
  MSG msg = {};
  double accumulator = 0;
  while (true) {
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        DeleteObject(font);
        return 0;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    QueryPerformanceCounter(&now);
    double dt =
        std::min(.05, double(now.QuadPart - last.QuadPart) / freq.QuadPart);
    last = now;
    bool held = mouse || keys[VK_SPACE];
    if (captured && GetForegroundWindow() == win) {
      pressPending = pressPending || (held && !fireWasDown);
      fireWasDown = held;
      accumulator += dt;
      constexpr double fixed = 1. / 240.;
      while (accumulator >= fixed) {
        game.walking = false;
        moving = false;
        game.ads = aiming;
        if (!game.ended && !game.buyOpen && game.hp > 0) {
          game.a = wrap(game.a + (keys[VK_RIGHT] - keys[VK_LEFT]) * fixed * 2);
          game.usingObjective = keys['F'];
          double forward = keys['W'] - keys['S'], side = keys['D'] - keys['A'],
                 len = std::max(1., hypot(forward, side));
          double speed = keys[VK_SHIFT] && !aiming ? 3.7 : aiming ? 1.45 : 2.6;
          forward *= fixed * speed / len;
          side *= fixed * speed / len;
          game.move(cos(game.a) * forward - sin(game.a) * side,
                    sin(game.a) * forward + cos(game.a) * side);
          moving = game.walking;
        } else {
          game.usingObjective = false;
          if (game.hp <= 0)
            aiming = false;
        }
        game.tick(fixed);
        game.trigger(held, pressPending);
        pressPending = false;
        if (!game.buyAllowed())
          game.buyOpen = false;
        accumulator -= fixed;
      }
    } else {
      accumulator = 0;
      fireWasDown = pressPending = false;
    }
    audio.pump();
    InvalidateRect(win, nullptr, FALSE);
    UpdateWindow(win);
    Sleep(1);
  }
}
#else
#include "tests.hpp"
#endif
