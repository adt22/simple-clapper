# Simple Clapper
### A digital clapperboard for the Flipper Zero
**By Cinemateum (with Claude) · youtube.com/@cinemateum**

---

## What is it?

Simple Clapper turns your Flipper Zero into a minimal digital clapperboard. It displays a large Scene number and Take letter, and when you press OK it fires a simultaneous screen flash, LED flash, and beep — giving your camera and microphone a clear sync point to align audio and video in post.

It is designed for YouTubers and indie filmmakers shooting without a dedicated sound recordist or timecode system.

---

## Before you start

**Set your Flipper volume to maximum.**
Go to Flipper Settings → Sound → set to max before launching the app. The Flipper's piezo buzzer is physically small — you want every dB you can get for the boom mic to catch it cleanly.

**The LED flash is your friend.**
The white LED fires simultaneously with the beep. Even if the buzzer is hard to hear on set, the LED gives you a second visual sync point on camera. Point the Flipper toward the camera when slating.

---

## Navigation

### Settings page (opens on launch)
| Button | Action |
|--------|--------|
| Up / Down | Navigate options |
| OK | Toggle selected option |
| Back | Exit to main slate screen |

### Main slate screen
| Button | Action |
|--------|--------|
| Up | Scene + 1 (resets Take to A) |
| Down | Scene - 1 (resets Take to A) |
| Left | Take - 1 |
| Right | Take + 1 |
| OK | Slate (see below) |
| Back | Return to Settings |

---

## Settings explained

**INVERT** — Flips the display. White background with black characters. Useful if your lighting setup makes the standard black screen hard to read on camera, or if you simply prefer it.

**VOLUME** — Controls the beep loudness. Options: MUTE / LOW / MED / HIGH. Default is HIGH. Keep it at HIGH unless you have a specific reason to lower it.

**AUTO-INC** — Controls how the Take letter advances after each slate. See below.

---

## The slate workflow

### With AUTO-INC ON (recommended)

This mode prevents a confusing situation where the Flipper shows the *next* take while the camera is still rolling on the *current* one.

1. Screen shows **1A**
2. Press **OK** → flash + beep + LED fires. Screen still shows **1A**. A small dot appears in the bottom-left corner — this means an increment is ready.
3. Lower the Flipper out of frame. The camera still sees **1A** while you act your scene.
4. When ready for the next take, press **OK** again → screen silently advances to **1B** (no flash, no beep, no confusion).
5. Press **OK** again → flash + beep + LED fires on **1B**. Repeat.

Every audible slate = a real take being recorded. The silent press is invisible bookkeeping.

### With AUTO-INC OFF

OK always slates the current take with flash + beep. No automatic advancement. You manage the Take letter manually with Left/Right. Useful if you prefer full manual control or are logging takes differently.

---

## Scene and Take logic

- **Scenes**: 1 to 99. Scenes 1–9 display as a single large character. Scenes 10–99 display as two smaller characters, both still clearly readable on camera.
- **Takes**: A to Z. Wraps from Z back to A.
- **Changing the scene always resets the Take back to A.**

---

## Tips for YouTube shoots

- Slate at the start of each usable take, not every attempt — you will thank yourself in the edit.
- If shooting solo (camera locked off), hold the Flipper in front of the lens, press OK, then lower it and start your performance. The LED gives you the sync point even if the beep is off-screen.
- The 80ms flash duration is long enough to be caught at 24fps (one frame = 41.7ms), so the flash will always appear in at least one full frame.
- If you forget to slate, the LED flash alone is often enough to sync visually in post — look for the white flare in the footage.

---

## About

Simple Clapper v0.2
Made by **Cinemateum** (with Claude)
youtube.com/@cinemateum

Built for the Flipper Zero using the Flipper SDK and ufbt.
