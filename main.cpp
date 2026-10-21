#include "interface_draw.hpp"
#include "renderer.hpp"
#include <fstream>
#include <map>
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
          out = short(std::clamp(sum * ui.prefs.volume, -1., 1.) * 30000);
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
bool keys[256] = {}, mouse = false, captured = false, fireWasDown = false,
     pressPending = false, draggingSlider = false, mouseInteract = false;
int wheelRemainder = 0;
std::map<int, HFONT> fonts;
std::string prefsPath;
void capture(HWND win, bool on) {
  if (captured == on)
    return;
  captured = on;
  mouse = fireWasDown = pressPending = mouseInteract = false;
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
    for (bool &key : keys)
      key = false;
  }
}
void syncCapture(HWND win) { capture(win, ui.capturesMouse()); }
void savePreferences() {
  if (!prefsPath.empty() && !ui.prefs.save(prefsPath)) {
    ui.feedback = "Settings applied for this session; file could not be saved.";
    ui.feedbackTime = 4;
  }
}
void syncFullscreen(HWND win) {
  static bool applied = false;
  static WINDOWPLACEMENT placement = {
      sizeof(WINDOWPLACEMENT), 0, 0, {0, 0}, {0, 0}, {0, 0, 0, 0}};
  if (applied == ui.prefs.fullscreen)
    return;
  applied = ui.prefs.fullscreen;
  if (applied) {
    GetWindowPlacement(win, &placement);
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(MonitorFromWindow(win, MONITOR_DEFAULTTONEAREST), &mi);
    SetWindowLongPtr(win, GWL_STYLE,
                     GetWindowLongPtr(win, GWL_STYLE) & ~WS_OVERLAPPEDWINDOW);
    SetWindowPos(win, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                 mi.rcMonitor.right - mi.rcMonitor.left,
                 mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_FRAMECHANGED);
  } else {
    SetWindowLongPtr(win, GWL_STYLE,
                     GetWindowLongPtr(win, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(win, &placement);
    SetWindowPos(win, nullptr, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
  }
}
POINT cursorPosition(HWND win, LPARAM l) {
  RECT r;
  GetClientRect(win, &r);
  return {int(short(LOWORD(l))) * W / std::max(1, int(r.right)),
          int(short(HIWORD(l))) * H / std::max(1, int(r.bottom))};
}
void drawTextLayer(HDC dc) {
  SetBkMode(dc, TRANSPARENT);
  for (auto &text : interfaceText) {
    if (!fonts.count(text.size))
      fonts[text.size] = CreateFontA(
          text.size, 0, 0, 0, text.size >= 28 ? FW_BOLD : FW_NORMAL, FALSE,
          FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
          CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Segoe UI");
    auto old = SelectObject(dc, fonts[text.size]);
    SetTextColor(dc, RGB((text.color >> 16) & 255, (text.color >> 8) & 255,
                         text.color & 255));
    TextOutA(dc, text.x, text.y, text.text.c_str(), int(text.text.size()));
    SelectObject(dc, old);
  }
}
LRESULT CALLBACK proc(HWND win, UINT msg, WPARAM w, LPARAM l) {
  switch (msg) {
  case WM_DESTROY:
    capture(win, false);
    PostQuitMessage(0);
    return 0;
  case WM_KILLFOCUS:
    if (ui.simulates())
      ui.open(ScreenPage::Pause, game);
    capture(win, false);
    draggingSlider = false;
    return 0;
  case WM_SIZE:
    if (captured) {
      ui.open(ScreenPage::Pause, game);
      capture(win, false);
    }
    return 0;
  case WM_INPUT:
    if (captured && ui.page == ScreenPage::Play && game.hp > 0) {
      RAWINPUT raw = {};
      UINT size = sizeof(raw);
      if (GetRawInputData(reinterpret_cast<HRAWINPUT>(l), RID_INPUT, &raw,
                          &size, sizeof(RAWINPUTHEADER)) != UINT(-1) &&
          raw.header.dwType == RIM_TYPEMOUSE &&
          !(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
        double sensitivity =
            .0022 * ui.prefs.sensitivity * (aiming ? baseFocal() / focal() : 1);
        game.a = wrap(game.a + raw.data.mouse.lLastX * sensitivity);
        game.pitch =
            std::clamp(game.pitch - raw.data.mouse.lLastY * sensitivity *
                                        (ui.prefs.invertY ? -1 : 1),
                       -1.15, 1.15);
      }
    }
    return 0;
  case WM_MOUSEMOVE:
    if (!captured) {
      auto p = cursorPosition(win, l);
      ui.mx = p.x;
      ui.my = p.y;
      if (draggingSlider && (w & MK_LBUTTON))
        ui.slider(p.x, p.y);
      if (ui.page == ScreenPage::Utility)
        for (auto b : ui.buttons(game))
          if (b.action >= 200 && b.action <= 202 && b.enabled &&
              b.r.contains(p.x, p.y)) {
            game.utility = b.action - 200;
            ui.selectedUtility = game.utility;
          }
    }
    return 0;
  case WM_MOUSEWHEEL:
    wheelRemainder += short(HIWORD(w));
    while (std::abs(wheelRemainder) >= WHEEL_DELTA) {
      int direction = wheelRemainder > 0 ? 1 : -1;
      ui.wheel(direction, game);
      wheelRemainder -= direction * WHEEL_DELTA;
    }
    return 0;
  case WM_KEYDOWN:
    if (w < 256)
      keys[w] = true;
    if (l & (1LL << 30))
      return 0;
    if (w == VK_ESCAPE) {
      if (ui.page == ScreenPage::Settings)
        savePreferences();
      ui.escape(game);
      syncCapture(win);
      return 0;
    }
    if (w == VK_RETURN && ui.page == ScreenPage::Result) {
      ui.activate(10, game);
      syncCapture(win);
      return 0;
    }
    if (ui.page != ScreenPage::Play)
      return 0;
    if (w == 'R')
      game.startReload();
    if (w == VK_SPACE)
      game.jump();
    if (w == 'B') {
      ui.open(ScreenPage::Buy, game);
      syncCapture(win);
    }
    if (w == VK_TAB) {
      ui.open(ScreenPage::Roster, game);
      syncCapture(win);
    }
    if (w == '1' && game.primary >= 0) {
      ui.lastWeapon = game.weapon;
      game.switchWeapon(game.primary);
    }
    if (w == '2' && game.secondary >= 0) {
      ui.lastWeapon = game.weapon;
      game.switchWeapon(game.secondary);
    }
    if (w == 'E')
      game.pickup();
    if (w == 'G' && !game.utilityEquipped)
      game.dropGun(game.weapon);
    if (w == 'Q' && game.utilityEquipped) {
      game.utilityEquipped = false;
      return 0;
    }
    if (w == 'Q') {
      int previous = game.weapon;
      game.switchWeapon(ui.lastWeapon);
      ui.lastWeapon = previous;
    }
    if (w == 'H')
      game.throwUtility();
    if (w == 'V' && game.hp <= 0)
      game.spectator++;
    return 0;
  case WM_KEYUP:
    if (w < 256)
      keys[w] = false;
    return 0;
  case WM_LBUTTONDOWN:
    if (captured && ui.page == ScreenPage::Play)
      mouse = true;
    else {
      auto p = cursorPosition(win, l);
      auto before = ui.page;
      draggingSlider = ui.slider(p.x, p.y);
      if (!draggingSlider)
        ui.click(p.x, p.y, game);
      if (before == ScreenPage::Settings && ui.page != before)
        savePreferences();
      syncCapture(win);
    }
    return 0;
  case WM_LBUTTONUP:
    mouse = false;
    if (draggingSlider) {
      savePreferences();
      draggingSlider = false;
    }
    return 0;
  case WM_RBUTTONDOWN:
    if (captured && game.hp > 0)
      aiming = true;
    else if (ui.page == ScreenPage::Utility) {
      ui.open(ScreenPage::Play, game);
      syncCapture(win);
    }
    return 0;
  case WM_RBUTTONUP:
    aiming = false;
    return 0;
  case WM_MBUTTONDOWN:
    if (ui.page == ScreenPage::Play && game.hp > 0) {
      ui.selectedUtility = game.utility;
      ui.open(ScreenPage::Utility, game);
      syncCapture(win);
      POINT p = {W / 2, 300};
      RECT r;
      GetClientRect(win, &r);
      p.x = p.x * r.right / W;
      p.y = p.y * r.bottom / H;
      ClientToScreen(win, &p);
      SetCursorPos(p.x, p.y);
    }
    return 0;
  case WM_MBUTTONUP:
    if (ui.page == ScreenPage::Utility) {
      game.utilityEquipped = true;
      ui.open(ScreenPage::Play, game);
      syncCapture(win);
    }
    return 0;
  case WM_XBUTTONDOWN:
    if (ui.page == ScreenPage::Play) {
      if (HIWORD(w) == XBUTTON1)
        game.startReload();
      else
        mouseInteract = true;
    }
    return TRUE;
  case WM_XBUTTONUP:
    if (HIWORD(w) == XBUTTON2)
      mouseInteract = false;
    return TRUE;
  case WM_ENTERSIZEMOVE:
    if (ui.simulates())
      ui.open(ScreenPage::Pause, game);
    capture(win, false);
    return 0;
  case WM_ERASEBKGND:
    return 1;
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(win, &ps);
    bool home =
        ui.page == ScreenPage::Home || (ui.page == ScreenPage::Settings &&
                                        ui.settingsReturn == ScreenPage::Home);
    if (home) {
      double x = game.x, y = game.y, a = game.a, p = game.pitch;
      int gun = game.weapon;
      game.x = 24;
      game.y = 26;
      game.a = .25;
      game.pitch = -.03;
      game.weapon = 1;
      drawWorld();
      drawViewmodel();
      game.x = x;
      game.y = y;
      game.a = a;
      game.pitch = p;
      game.weapon = gun;
    } else
      render();
    drawInterface();
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
    drawTextLayer(back);
    RECT r;
    GetClientRect(win, &r);
    SetStretchBltMode(dc, HALFTONE);
    StretchBlt(dc, 0, 0, r.right, r.bottom, back, 0, 0, W, H, SRCCOPY);
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
  char module[MAX_PATH] = {};
  DWORD moduleSize = GetModuleFileNameA(nullptr, module, MAX_PATH);
  if (moduleSize > 0 && moduleSize < MAX_PATH) {
    std::string path(module);
    auto slash = path.find_last_of("\\/");
    if (slash != std::string::npos)
      loadArt(path.substr(0, slash) + "/assets");
  }
  char local[MAX_PATH] = {};
  DWORD n = GetEnvironmentVariableA("LOCALAPPDATA", local, MAX_PATH);
  if (n > 0 && n < MAX_PATH) {
    std::string directory = std::string(local) + "\\Breach";
    CreateDirectoryA(directory.c_str(), nullptr);
    prefsPath = directory + "\\settings.cfg";
    ui.prefs.load(prefsPath);
  }
  WNDCLASSA wc = {};
  wc.lpfnWndProc = proc;
  wc.hInstance = inst;
  wc.lpszClassName = "BreachArena";
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  RegisterClassA(&wc);
  RECT r = {0, 0, W, H};
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
  HWND win = CreateWindowA(wc.lpszClassName, "Breach / Harbor",
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
        for (auto f : fonts)
          DeleteObject(f.second);
        return 0;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    QueryPerformanceCounter(&now);
    double dt =
        std::min(.1, double(now.QuadPart - last.QuadPart) / freq.QuadPart);
    last = now;
    ui.update(dt, game);
    if (ui.quit) {
      DestroyWindow(win);
      continue;
    }
    syncFullscreen(win);
    syncCapture(win);
    aimBlend += (double(aiming) - aimBlend) * std::min(1., dt * 12);
    bool held = mouse;
    if (ui.simulates() && GetForegroundWindow() == win) {
      pressPending = pressPending || (held && !fireWasDown);
      fireWasDown = held;
      accumulator += dt;
      constexpr double fixed = 1. / 240.;
      while (accumulator >= fixed) {
        game.walking = moving = false;
        game.ads = aiming;
        if (ui.page == ScreenPage::Play && !game.ended && game.hp > 0) {
          game.usingObjective = keys['F'] || mouseInteract;
          game.locomotion(fixed, keys['W'] - keys['S'], keys['D'] - keys['A'],
                          keys[VK_SHIFT], keys[VK_CONTROL]);
          moving = game.walking;
        } else {
          game.usingObjective = false;
          game.vx = game.vy = 0;
          if (game.hp <= 0)
            aiming = false;
        }
        game.tick(fixed);
        if (ui.page == ScreenPage::Play)
          game.trigger(held, pressPending);
        pressPending = false;
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
#include "interface_draw.hpp"
#include "tests.hpp"
#endif
