//! Keep USB animation and active input responsive; idle battery status is event-driven.
#[no_mangle]
pub extern "C" fn seahorse_display_period_ms(
    usb_powered: bool,
    active: bool,
    period_ms: u32,
) -> u32 {
    if usb_powered || active {
        period_ms.max(1)
    } else {
        0
    }
}
