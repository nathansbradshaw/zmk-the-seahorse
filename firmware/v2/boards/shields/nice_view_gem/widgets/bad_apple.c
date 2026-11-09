#include <string.h>
#include <zephyr/kernel.h>
#include <zmk/usb.h>
#include "bad_apple.h"
#include "bad_apple_codec.h"
#include "util.h"
#include "../assets/bad_apple_data.inc"

static struct ba_decoder decoder;
static struct ba_session session;
static uint8_t frame[BAD_APPLE_WIDTH * BAD_APPLE_HEIGHT / 8];
static uint32_t next_frame;
static lv_obj_t *video_canvas;
static lv_color_t *video_buffer;
static lv_timer_t *timer;

static void clear_video(void) {
    fill_background(video_canvas);
    rotate_canvas(video_canvas, video_buffer);
    lv_obj_invalidate(video_canvas);
}

static void draw_frame(void) {
    fill_background(video_canvas);
    for (int y = 0; y < BAD_APPLE_HEIGHT; y++) {
        for (int x = 0; x < BAD_APPLE_WIDTH; x++) {
            unsigned int pixel = y * BAD_APPLE_WIDTH + x;
            bool white = frame[pixel / 8] & (0x80 >> (pixel % 8));
            /* Write the canvas buffer directly; invalidate only once per frame. */
            video_buffer[(y + (BUFFER_SIZE - BAD_APPLE_HEIGHT) / 2) * BUFFER_SIZE +
                         x + (BUFFER_SIZE - BAD_APPLE_WIDTH) / 2] =
                white ? lv_color_white() : lv_color_black();
        }
    }
    rotate_canvas(video_canvas, video_buffer);
    lv_obj_invalidate(video_canvas);
}

static void video_tick(lv_timer_t *unused) {
    ARG_UNUSED(unused);
    /* Recheck before touching pixels even if a USB display update is queued. */
    if (!session.powered || !zmk_usb_is_powered()) {
        ba_session_power(&session, false, session.connection, k_uptime_get_32());
        lv_timer_pause(timer);
        clear_video();
        return;
    }
    uint32_t due = ba_due_frame(&session, k_uptime_get_32(), BAD_APPLE_FPS);
    if (next_frame == BAD_APPLE_FRAMES && due >= BAD_APPLE_FRAMES) {
        session.finished = true;
        lv_timer_pause(timer);
        clear_video();
        return;
    }
    bool changed = false;
    /* Bound catch-up work so a delayed display never monopolizes its queue. */
    for (int budget = 0; budget < 3 && next_frame <= due && next_frame < BAD_APPLE_FRAMES;
         budget++) {
        if (!ba_decode(&decoder, bad_apple_data, sizeof(bad_apple_data), frame, sizeof(frame))) {
            session.finished = true;
            lv_timer_pause(timer);
            clear_video();
            return;
        }
        next_frame++;
        changed = true;
    }
    if (changed) { draw_frame(); }
}

void bad_apple_init(lv_obj_t *canvas, lv_color_t *buffer) {
    video_canvas = canvas;
    video_buffer = buffer;
    timer = lv_timer_create(video_tick, 20, NULL);
    lv_timer_pause(timer);
    clear_video();
}

void bad_apple_power(bool powered, uint32_t connection) {
    if (!timer) { return; }
    if (ba_session_power(&session, powered, connection, k_uptime_get_32())) {
        memset(&decoder, 0, sizeof(decoder));
        next_frame = 0;
        lv_timer_resume(timer);
        lv_timer_ready(timer);
    } else if (!powered) {
        lv_timer_pause(timer);
        clear_video();
    }
}
