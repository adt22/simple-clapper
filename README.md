# Simple Clapper

A minimal digital clapperboard app for the **Flipper Zero**.

Made by [Cinemateum](https://www.youtube.com/@cinemateum) (with Claude).

---

## What it does

Displays a large **Scene number** and **Take letter** on the Flipper screen. Press OK to slate — screen flash, beep, and LED fire simultaneously, giving your camera and microphone a clean sync point for audio/video alignment in post.

Designed for YouTubers and indie filmmakers shooting without a dedicated sound recordist or timecode system.

![Simple Clapper icon](clapper.png)

---

## Features

- Scenes **1–99** (single digits display large, double digits display smaller but still readable)
- Takes **A–Z**
- Simultaneous **screen flash + piezo beep + RGB LED** on slate
- **Smart auto-increment** — take advances silently between takes so the camera never sees the wrong take number while rolling
- **Inverted display** mode for different lighting setups
- **Volume control** — Mute / Low / Med / High
- Settings menu with icons
- Scrollable how-to guide built in
- Exits cleanly via long-press Back

---

## Controls

### Settings screen (opens on launch)
| Button | Action |
|--------|--------|
| Up / Down | Navigate menu |
| OK | Select / toggle option |
| Back | Go to slate screen |
| Long Back | Exit app |

### Slate screen
| Button | Action |
|--------|--------|
| Up / Down | Scene +/- (resets take to A) |
| Left / Right | Take letter +/- |
| OK | Slate |
| Back | Return to settings |

---

## The auto-increment workflow

With **AUTO-INC ON** (recommended):

1. Screen shows **1A**
2. **Press OK** → flash + beep + LED. Screen stays on **1A**. Small dot appears = increment is armed.
3. Lower the Flipper. Camera still sees **1A** while you perform.
4. **Press OK again** → silently advances to **1B**. No sound, no flash.
5. **Press OK again** → flash + beep + LED on **1B**. Repeat.

Every audible slate = a real take. The silent press is bookkeeping only.

With **AUTO-INC OFF**: OK always slates. Manage takes manually with Left/Right.

---

## Before you use it

**Set your Flipper volume to maximum** in Settings → Sound before launching the app. The piezo buzzer is physically small — you want every dB available for the boom mic to catch the sync.

---

## Installation

### Requirements
- [Python 3](https://python.org)
- [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) — `pip install ufbt`

### Build and deploy
```bash
git clone https://github.com/YOUR_USERNAME/simple-clapper
cd simple-clapper
ufbt update
ufbt launch
```

Or copy the pre-built `simple_clapper.fap` to `SD Card/apps/Tools/` via [qFlipper](https://flipperzero.one/update).

---

## Technical notes

- Flash duration: **80ms** = ~2 frames at 24fps. Always caught on camera.
- LED fires a white burst in sync with the beep via the Flipper notification system.
- Custom bitmap font rendered at **56×60px** (large) and **28×50px** (double-digit scenes) — the Flipper's built-in fonts are too small to be readable on camera.
- Stack size: 4KB.

---

## License

MIT — do whatever you want with it.

---

## Credits

Built by [Cinemateum](https://www.youtube.com/@cinemateum) with [Claude](https://claude.ai) (Anthropic).
