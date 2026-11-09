#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zmk/activity.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/usb.h>
#include "display_power.h"

/* Adapter to the pinned ZMK v0.3 display scheduler (src/display/main.c).
 * Keep its display queue and hardware driver; only control periodic polling.
 */
extern struct k_timer display_timer;
extern uint32_t seahorse_display_period_ms(bool usb_powered, bool active, uint32_t period_ms);

struct display_power_state { bool usb_powered; bool active; };

static void display_refresh_cb(struct k_work *work) {
    ARG_UNUSED(work);
    lv_task_handler();
    /* A lone update must not wait for LVGL's next refresh deadline. */
    lv_refr_now(NULL);
}

K_WORK_DEFINE(seahorse_display_refresh_work, display_refresh_cb);

static struct display_power_state display_power_get_state(const zmk_event_t *eh) {
    return (struct display_power_state){
        .usb_powered = zmk_usb_is_powered(),
        .active = zmk_activity_get_state() == ZMK_ACTIVITY_ACTIVE,
    };
}

static void display_power_update_cb(struct display_power_state state) {
    uint32_t period = seahorse_display_period_ms(state.usb_powered, state.active,
                                                CONFIG_ZMK_DISPLAY_TICK_PERIOD_MS);
    if (period) {
        k_timer_start(&display_timer, K_MSEC(period), K_MSEC(period));
    } else {
        k_timer_stop(&display_timer);
    }
    /* Flush once on state changes, including the last frame on USB removal. */
    k_work_submit_to_queue(zmk_display_work_q(), &seahorse_display_refresh_work);
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_display_power, struct display_power_state,
                           display_power_update_cb, display_power_get_state);
ZMK_SUBSCRIPTION(widget_display_power, zmk_activity_state_changed);
ZMK_SUBSCRIPTION(widget_display_power, zmk_usb_conn_state_changed);

void seahorse_display_power_init(void) {
    widget_display_power_init();
    /* Run after ZMK's initialize_display has started its default timer. */
    k_work_submit_to_queue(zmk_display_work_q(), &widget_display_power_work);
}

void seahorse_display_request_refresh(void) {
    if (zmk_display_is_initialized() && !zmk_usb_is_powered() &&
        zmk_activity_get_state() != ZMK_ACTIVITY_ACTIVE) {
        /* Canvas updates already run on the display queue; flush after them.
         * This preserves current status without a polling timer while idle.
         */
        k_work_submit_to_queue(zmk_display_work_q(), &seahorse_display_refresh_work);
    }
}
