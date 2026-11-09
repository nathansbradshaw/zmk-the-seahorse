#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* 4 KiB streaming history, not one RAM buffer per video frame. */
struct ba_decoder {
    uint8_t history[4096];
    size_t offset;
    uint32_t produced;
    uint16_t distance;
    uint16_t remaining;
    uint8_t flags;
    uint8_t bits;
};

struct ba_session {
    uint32_t connection;
    uint32_t started_ms;
    bool powered;
    bool finished;
};

bool ba_decode(struct ba_decoder *decoder, const uint8_t *data, size_t length,
               uint8_t *output, size_t count);
/* True only when a fresh powered connection needs a new playback. */
bool ba_session_power(struct ba_session *session, bool powered, uint32_t connection,
                      uint32_t now);
uint32_t ba_due_frame(const struct ba_session *session, uint32_t now, uint32_t fps);
