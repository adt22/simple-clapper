#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "glyphs.h"

// ─── Layout ──────────────────────────────────────────────────────────────────
#define SCREEN_W        128
#define SCREEN_H         64
#define DIVIDER_X        63
#define LG_SCENE_X        3
#define LG_TAKE_X        68
#define LG_Y              2
#define SM_Y              7
#define SM_X_START        2
#define SM_GAP            2
#define FLASH_MS         80

// Settings layout
// Header bar: y=0..11 (12px)
// 5 rows visible, each 10px tall, starting y=13
// Row text baseline = row_top + 8
// Selection highlight box = row_top-1 to row_top+9 (11px tall)
#define MENU_HEADER_H    12
#define MENU_ROW_H       10
#define MENU_ROW0_Y      13
#define MENU_ROWS_VIS     5
#define MENU_ICON_X       2
#define MENU_LABEL_X     14
#define MENU_VAL_X       88
#define MENU_ITEMS        6  // START, INVERT, VOLUME, AUTO-INC, HOW-TO, ABOUT

// Instructions layout
#define INSTR_ROW_H      10
#define INSTR_ROW0_Y     14
#define INSTR_ROWS_VIS    5
#define INSTR_TOTAL      35

// ─── State ───────────────────────────────────────────────────────────────────

typedef enum { VOL_MUTE, VOL_QUIET, VOL_NORMAL, VOL_LOUD } VolumeLevel;
typedef enum { PAGE_SETTINGS, PAGE_MAIN, PAGE_ABOUT, PAGE_INSTRUCTIONS } AppPage;

typedef struct {
    uint8_t      scene;
    uint8_t      take;
    bool         inverted;
    VolumeLevel  volume;
    bool         auto_increment;
    bool         pending_inc;
    bool         flash;
    AppPage      page;
    uint8_t      settings_sel;
    uint8_t      menu_scroll;          // top visible row index
    uint8_t      instructions_scroll;
} ClapperState;

// ─── Notification sequences ──────────────────────────────────────────────────

static const NotificationSequence seq_led = {
    &message_red_255, &message_green_255, &message_blue_255,
    &message_delay_50,
    &message_red_0,   &message_green_0,   &message_blue_0,
    NULL,
};
static const NotificationSequence seq_beep_quiet  = { &message_note_a5,  &message_delay_50,  &message_sound_off, NULL };
static const NotificationSequence seq_beep_normal = { &message_note_c6,  &message_delay_50,  &message_sound_off, NULL };
static const NotificationSequence seq_beep_loud   = { &message_note_c6,  &message_delay_100, &message_sound_off, NULL };

// ─── Small icon primitives (9x9 each) ────────────────────────────────────────

static void icon_clapper(Canvas* c, int x, int y) {
    canvas_draw_box(c,  x+1, y,   7, 2);
    canvas_draw_box(c,  x,   y+3, 9, 5);
    canvas_draw_line(c, x,   y,   x, y+3);
    // stripe gaps on arm
    canvas_draw_dot(c, x+3, y);   canvas_draw_dot(c, x+3, y+1);
    canvas_draw_dot(c, x+6, y);   canvas_draw_dot(c, x+6, y+1);
}

static void icon_film(Canvas* c, int x, int y) {
    canvas_draw_frame(c, x, y, 9, 9);
    canvas_draw_box(c, x+1, y+1, 2, 2);
    canvas_draw_box(c, x+6, y+1, 2, 2);
    canvas_draw_box(c, x+1, y+6, 2, 2);
    canvas_draw_box(c, x+6, y+6, 2, 2);
    canvas_draw_line(c, x+3, y+4, x+5, y+4);
}

static void icon_speaker(Canvas* c, int x, int y) {
    canvas_draw_box(c,  x,   y+3, 3, 3);
    canvas_draw_line(c, x+2, y+3, x+5, y+1);
    canvas_draw_line(c, x+2, y+5, x+5, y+7);
    canvas_draw_line(c, x+5, y+1, x+5, y+7);
    canvas_draw_line(c, x+6, y+2, x+6, y+6);
    canvas_draw_line(c, x+8, y+1, x+8, y+7);
}

static void icon_auto(Canvas* c, int x, int y) {
    canvas_draw_line(c, x+1, y+4, x+7, y+4);
    canvas_draw_line(c, x+5, y+2, x+7, y+4);
    canvas_draw_line(c, x+5, y+6, x+7, y+4);
    canvas_draw_line(c, x+1, y+4, x+1, y+7);
    canvas_draw_line(c, x+1, y+7, x+4, y+7);
}

static void icon_info(Canvas* c, int x, int y) {
    canvas_draw_circle(c, x+4, y+4, 4);
    canvas_draw_dot(c,   x+4, y+2);
    canvas_draw_line(c,  x+4, y+4, x+4, y+6);
}

// ─── Glyph draw helpers ──────────────────────────────────────────────────────

static void draw_large_glyph(Canvas* canvas, const uint64_t glyph[LGLYPH_H], int ox, int oy) {
    for(int row = 0; row < LGLYPH_H; row++) {
        uint64_t bits = glyph[row];
        if(!bits) continue;
        for(int col = 0; col < LGLYPH_W; col++)
            if(bits & ((uint64_t)1 << (LGLYPH_W - 1 - col)))
                canvas_draw_dot(canvas, ox + col, oy + row);
    }
}

static void draw_small_glyph(Canvas* canvas, const uint32_t glyph[SGLYPH_H], int ox, int oy) {
    for(int row = 0; row < SGLYPH_H; row++) {
        uint32_t bits = glyph[row];
        if(!bits) continue;
        for(int col = 0; col < SGLYPH_W; col++)
            if(bits & ((uint32_t)1 << (SGLYPH_W - 1 - col)))
                canvas_draw_dot(canvas, ox + col, oy + row);
    }
}

// ─── Pages ───────────────────────────────────────────────────────────────────

static void draw_main(Canvas* canvas, ClapperState* s) {
    canvas_clear(canvas);
    if(s->inverted) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, 0, 0, SCREEN_W, SCREEN_H);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_color(canvas, ColorBlack);
    }
    if(s->scene < 10) {
        draw_large_glyph(canvas, GLYPH_DIGIT_LG[s->scene], LG_SCENE_X, LG_Y);
    } else {
        draw_small_glyph(canvas, GLYPH_DIGIT_SM[s->scene / 10], SM_X_START,                   SM_Y);
        draw_small_glyph(canvas, GLYPH_DIGIT_SM[s->scene % 10], SM_X_START + SGLYPH_W + SM_GAP, SM_Y);
    }
    draw_large_glyph(canvas, GLYPH_LETTER[s->take], LG_TAKE_X, LG_Y);
    canvas_draw_line(canvas, DIVIDER_X, 2, DIVIDER_X, SCREEN_H - 3);
    if(s->pending_inc && s->auto_increment) {
        canvas_set_color(canvas, s->inverted ? ColorWhite : ColorBlack);
        canvas_draw_box(canvas, 1, SCREEN_H - 4, 3, 3);
    }
}

// Menu item definitions
typedef struct {
    const char* label;
    void (*icon_fn)(Canvas*, int, int);
} MenuItem;

static const MenuItem MENU_ITEMS_DEF[MENU_ITEMS] = {
    { "START",    icon_clapper },
    { "INVERT",   icon_film    },
    { "VOLUME",   icon_speaker },
    { "AUTO-INC", icon_auto    },
    { "HOW-TO",   icon_info    },
    { "ABOUT",    icon_info    },
};

static void draw_settings(Canvas* canvas, ClapperState* s) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    // Header
    canvas_draw_box(canvas, 0, 0, SCREEN_W, MENU_HEADER_H);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 14, 9, "[ SIMPLE CLAPPER ]");
    canvas_set_color(canvas, ColorBlack);

    const char* vol_labels[] = { "MUTE", "LOW ", "MED ", "HIGH" };

    // Draw MENU_ROWS_VIS rows starting from menu_scroll
    for(int i = 0; i < MENU_ROWS_VIS; i++) {
        int item_idx = s->menu_scroll + i;
        if(item_idx >= MENU_ITEMS) break;

        int row_y = MENU_ROW0_Y + i * MENU_ROW_H;
        bool active = (item_idx == (int)s->settings_sel);

        // Highlight: draw box from row_y to row_y+MENU_ROW_H-1
        if(active) {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, row_y, SCREEN_W, MENU_ROW_H);
            canvas_set_color(canvas, ColorWhite);
        } else {
            canvas_set_color(canvas, ColorBlack);
        }

        // Icon (top-left of row, vertically centred in row)
        if(MENU_ITEMS_DEF[item_idx].icon_fn)
            MENU_ITEMS_DEF[item_idx].icon_fn(canvas, MENU_ICON_X, row_y);

        // Label
        canvas_set_font(canvas, FontKeyboard);
        canvas_draw_str(canvas, MENU_LABEL_X, row_y + 8, MENU_ITEMS_DEF[item_idx].label);

        // Value (right side, only for toggle items)
        const char* val = NULL;
        if(item_idx == 1) val = s->inverted       ? "ON " : "OFF";
        if(item_idx == 2) val = vol_labels[s->volume];
        if(item_idx == 3) val = s->auto_increment ? "ON " : "OFF";
        if(val) canvas_draw_str(canvas, MENU_VAL_X, row_y + 8, val);
    }

    // Scroll indicators
    canvas_set_color(canvas, ColorBlack);
    if(s->menu_scroll > 0)
        canvas_draw_str(canvas, 121, MENU_ROW0_Y + 6, "^");
    if(s->menu_scroll + MENU_ROWS_VIS < MENU_ITEMS)
        canvas_draw_str(canvas, 121, MENU_ROW0_Y + (MENU_ROWS_VIS - 1) * MENU_ROW_H + 6, "v");
}

// Instructions text
static const char* INSTR_LINES[] = {
    "SIMPLE CLAPPER v0.2",
    "By Cinemateum",
    "(with Claude)",
    "",
    "-- SETUP --",
    "Set Flipper volume",
    "to MAX in Settings.",
    "Point at camera",
    "when slating.",
    "",
    "-- MAIN SCREEN --",
    "UP/DOWN: scene",
    "(resets take to A)",
    "L/R: change take",
    "OK: slate",
    "BACK: menu",
    "",
    "-- AUTO-INC ON --",
    "Recommended mode.",
    "1st OK: slates.",
    "Dot = next ready.",
    "2nd OK: advances",
    "silently.",
    "3rd OK: slates.",
    "Sound = real take.",
    "",
    "-- AUTO-INC OFF --",
    "OK always slates.",
    "L/R to change take.",
    "",
    "-- FLASH --",
    "80ms = 2 frames",
    "at 24fps. Always",
    "caught on camera.",
    "LED fires in sync.",
    NULL,
};

static void draw_instructions(Canvas* canvas, uint8_t scroll) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_box(canvas, 0, 0, SCREEN_W, MENU_HEADER_H);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 22, 9, "[ HOW-TO ]");
    canvas_set_color(canvas, ColorBlack);

    canvas_set_font(canvas, FontKeyboard);
    for(uint8_t i = 0; i < INSTR_ROWS_VIS; i++) {
        uint8_t idx = scroll + i;
        if(idx >= INSTR_TOTAL) break;
        int y = INSTR_ROW0_Y + i * INSTR_ROW_H;
        canvas_draw_str(canvas, 2, y + 8, INSTR_LINES[idx]);
    }

    // Scroll arrows — right edge, not overlapping text
    if(scroll > 0)
        canvas_draw_str(canvas, 121, INSTR_ROW0_Y + 6, "^");
    if(scroll + INSTR_ROWS_VIS < INSTR_TOTAL)
        canvas_draw_str(canvas, 121, INSTR_ROW0_Y + (INSTR_ROWS_VIS - 1) * INSTR_ROW_H + 6, "v");
}

static void draw_about(Canvas* canvas) {
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);

    canvas_draw_box(canvas, 0, 0, SCREEN_W, MENU_HEADER_H);
    canvas_set_color(canvas, ColorWhite);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas, 22, 9, "[ ABOUT ]");

    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontKeyboard);
    canvas_draw_str(canvas,  2, 24, "SIMPLE CLAPPER v0.2");
    canvas_draw_str(canvas,  2, 35, "By: CINEMATEUM");
    canvas_draw_str(canvas,  2, 45, "    (with Claude)");
    canvas_draw_str(canvas,  2, 55, "YT: @cinemateum");
}

// ─── Callbacks ───────────────────────────────────────────────────────────────

static void clapper_draw_cb(Canvas* canvas, void* ctx) {
    ClapperState* s = (ClapperState*)ctx;
    if(s->flash) {
        if(s->inverted) {
            canvas_clear(canvas);
        } else {
            canvas_set_color(canvas, ColorBlack);
            canvas_draw_box(canvas, 0, 0, SCREEN_W, SCREEN_H);
        }
        return;
    }
    switch(s->page) {
        case PAGE_SETTINGS:     draw_settings(canvas, s);                          break;
        case PAGE_MAIN:         draw_main(canvas, s);                              break;
        case PAGE_ABOUT:        draw_about(canvas);                                break;
        case PAGE_INSTRUCTIONS: draw_instructions(canvas, s->instructions_scroll); break;
    }
}

static void clapper_input_cb(InputEvent* event, void* ctx) {
    furi_message_queue_put((FuriMessageQueue*)ctx, event, 0);
}

// ─── Slate ───────────────────────────────────────────────────────────────────

static void do_beep(ClapperState* s, NotificationApp* notif) {
    if(s->volume == VOL_MUTE) return;
    const NotificationSequence* seq =
        s->volume == VOL_QUIET ? &seq_beep_quiet :
        s->volume == VOL_LOUD  ? &seq_beep_loud  :
                                 &seq_beep_normal;
    notification_message(notif, seq);
}

static void do_flash(ClapperState* s, ViewPort* vp, NotificationApp* notif) {
    s->flash = true;
    view_port_update(vp);
    notification_message(notif, &seq_led);
    do_beep(s, notif);
    furi_delay_ms(FLASH_MS);
    s->flash = false;
}

static void handle_ok(ClapperState* s, ViewPort* vp, NotificationApp* notif) {
    if(s->auto_increment && s->pending_inc) {
        // Silent advance — no flash, no beep, no LED
        s->take = (s->take + 1) % 26;
        s->pending_inc = false;
    } else {
        do_flash(s, vp, notif);
        if(s->auto_increment) s->pending_inc = true;
    }
}

// ─── Menu navigation helpers ─────────────────────────────────────────────────

static void menu_scroll_to(ClapperState* s, int new_sel) {
    if(new_sel < 0) new_sel = 0;
    if(new_sel >= MENU_ITEMS) new_sel = MENU_ITEMS - 1;
    s->settings_sel = (uint8_t)new_sel;
    // Keep selected item visible
    if(s->settings_sel < s->menu_scroll)
        s->menu_scroll = s->settings_sel;
    if(s->settings_sel >= s->menu_scroll + MENU_ROWS_VIS)
        s->menu_scroll = s->settings_sel - MENU_ROWS_VIS + 1;
}

// ─── Main ────────────────────────────────────────────────────────────────────

int32_t clapper_app(void* p) {
    UNUSED(p);

    ClapperState state = {
        .scene                = 1,
        .take                 = 0,
        .inverted             = false,
        .volume               = VOL_LOUD,
        .auto_increment       = true,
        .pending_inc          = false,
        .flash                = false,
        .page                 = PAGE_SETTINGS,
        .settings_sel         = 0,
        .menu_scroll          = 0,
        .instructions_scroll  = 0,
    };

    FuriMessageQueue* queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    ViewPort* vp = view_port_alloc();
    view_port_draw_callback_set(vp, clapper_draw_cb, &state);
    view_port_input_callback_set(vp, clapper_input_cb, queue);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, vp, GuiLayerFullscreen);
    NotificationApp* notif = furi_record_open(RECORD_NOTIFICATION);

    InputEvent event;
    bool running = true;

    while(running) {
        if(furi_message_queue_get(queue, &event, 100) != FuriStatusOk) continue;

        // Long press Back on settings = exit app
        if(event.type == InputTypeLong && event.key == InputKeyBack) {
            if(state.page == PAGE_SETTINGS) {
                running = false;
                continue;
            }
        }

        if(event.type == InputTypeShort) {
            switch(state.page) {

            case PAGE_SETTINGS:
                switch(event.key) {
                case InputKeyUp:
                    menu_scroll_to(&state, (int)state.settings_sel - 1);
                    break;
                case InputKeyDown:
                    menu_scroll_to(&state, (int)state.settings_sel + 1);
                    break;
                case InputKeyOk:
                    switch(state.settings_sel) {
                    case 0: state.page = PAGE_MAIN; break;
                    case 1: state.inverted = !state.inverted; break;
                    case 2: state.volume = (state.volume + 1) % 4; break;
                    case 3:
                        state.auto_increment = !state.auto_increment;
                        state.pending_inc = false;
                        break;
                    case 4:
                        state.page = PAGE_INSTRUCTIONS;
                        state.instructions_scroll = 0;
                        break;
                    case 5: state.page = PAGE_ABOUT; break;
                    default: break;
                    }
                    break;
                case InputKeyBack:
                    state.page = PAGE_MAIN;
                    break;
                default: break;
                }
                break;

            case PAGE_MAIN:
                switch(event.key) {
                case InputKeyUp:
                    state.scene = (state.scene >= 99) ? 1 : state.scene + 1;
                    state.take = 0; state.pending_inc = false;
                    break;
                case InputKeyDown:
                    state.scene = (state.scene <= 1) ? 99 : state.scene - 1;
                    state.take = 0; state.pending_inc = false;
                    break;
                case InputKeyRight:
                    state.take = (state.take + 1) % 26;
                    state.pending_inc = false;
                    break;
                case InputKeyLeft:
                    state.take = (state.take + 25) % 26;
                    state.pending_inc = false;
                    break;
                case InputKeyOk:
                    handle_ok(&state, vp, notif);
                    break;
                case InputKeyBack:
                    state.page = PAGE_SETTINGS;
                    state.pending_inc = false;
                    break;
                default: break;
                }
                break;

            case PAGE_ABOUT:
                if(event.key == InputKeyBack) state.page = PAGE_SETTINGS;
                break;

            case PAGE_INSTRUCTIONS:
                if(event.key == InputKeyUp && state.instructions_scroll > 0)
                    state.instructions_scroll--;
                else if(event.key == InputKeyDown &&
                        state.instructions_scroll + INSTR_ROWS_VIS < INSTR_TOTAL)
                    state.instructions_scroll++;
                else if(event.key == InputKeyBack)
                    state.page = PAGE_SETTINGS;
                break;

            }
        }

        view_port_update(vp);
    }

    gui_remove_view_port(gui, vp);
    view_port_free(vp);
    furi_message_queue_free(queue);
    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    return 0;
}
