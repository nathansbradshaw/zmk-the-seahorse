#include "bad_apple_codec.h"

bool ba_decode(struct ba_decoder *d, const uint8_t *data, size_t length,
               uint8_t *output, size_t count) {
    for (size_t i = 0; i < count; i++) {
        uint8_t value;
        if (!d->remaining) {
            if (!d->bits) {
                if (d->offset >= length) { return false; }
                d->flags = data[d->offset++];
                d->bits = 8;
            }
            bool literal = d->flags & 1;
            d->flags >>= 1;
            d->bits--;
            if (literal) {
                if (d->offset >= length) { return false; }
                value = data[d->offset++];
            } else {
                if (d->offset > length || length - d->offset < 2) { return false; }
                uint16_t token = data[d->offset] | ((uint16_t)data[d->offset + 1] << 8);
                d->offset += 2;
                d->distance = (token >> 4) + 1;
                d->remaining = (token & 15) + 3;
                if (d->distance > d->produced) { return false; }
            }
            if (literal) {
                output[i] = value;
                d->history[d->produced++ & 4095] = value;
                continue;
            }
        }
        value = d->history[(d->produced - d->distance) & 4095];
        d->remaining--;
        output[i] = value;
        d->history[d->produced++ & 4095] = value;
    }
    return true;
}

bool ba_session_power(struct ba_session *s, bool powered, uint32_t connection, uint32_t now) {
    bool restart = powered && (!s->powered || s->connection != connection);
    s->powered = powered;
    s->connection = connection;
    if (restart) {
        s->started_ms = now;
        s->finished = false;
    }
    return restart;
}

uint32_t ba_due_frame(const struct ba_session *s, uint32_t now, uint32_t fps) {
    /* Unsigned subtraction also handles the 32-bit uptime wrap. */
    return ((uint64_t)(uint32_t)(now - s->started_ms) * fps) / 1000;
}
