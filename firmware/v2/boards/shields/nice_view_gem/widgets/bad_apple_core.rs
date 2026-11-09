//! Allocation-free playback state and streaming LZSS decoder for the C/LVGL widget.
#![cfg_attr(not(test), no_std)]

mod display_power;
pub use display_power::seahorse_display_period_ms;

#[repr(C)]
pub struct Decoder {
    history: [u8; 4096],
    offset: usize,
    produced: u32,
    distance: u16,
    remaining: u16,
    flags: u8,
    bits: u8,
}

#[repr(C)]
#[derive(Default)]
pub struct Session {
    connection: u32,
    started_ms: u32,
    powered: bool,
    finished: bool,
}

/// Decode exactly `count` bytes, carrying matches across video-frame boundaries.
///
/// # Safety
/// `decoder` points to initialized, exclusively borrowed Decoder storage. The
/// input/output ranges are valid for their lengths and do not alias the decoder.
#[no_mangle]
pub unsafe extern "C" fn ba_decode(
    decoder: *mut Decoder,
    data: *const u8,
    length: usize,
    output: *mut u8,
    count: usize,
) -> bool {
    if decoder.is_null() || data.is_null() || output.is_null() {
        return false;
    }
    let d = &mut *decoder;
    let input = core::slice::from_raw_parts(data, length);
    let output = core::slice::from_raw_parts_mut(output, count);
    for byte in output {
        if d.remaining == 0 {
            if d.bits == 0 {
                let Some(&flags) = input.get(d.offset) else {
                    return false;
                };
                d.flags = flags;
                d.offset += 1;
                d.bits = 8;
            }
            let literal = d.flags & 1 != 0;
            d.flags >>= 1;
            d.bits -= 1;
            if literal {
                let Some(&value) = input.get(d.offset) else {
                    return false;
                };
                d.offset += 1;
                *byte = value;
                d.history[d.produced as usize & 4095] = value;
                d.produced = d.produced.wrapping_add(1);
                continue;
            }
            let Some(&low) = input.get(d.offset) else {
                return false;
            };
            let Some(&high) = input.get(d.offset + 1) else {
                return false;
            };
            let token = u16::from_le_bytes([low, high]);
            d.offset += 2;
            d.distance = (token >> 4) + 1;
            d.remaining = (token & 15) + 3;
            if d.remaining == 18 {
                let Some(&extension) = input.get(d.offset) else {
                    return false;
                };
                d.offset += 1;
                d.remaining += extension as u16;
            }
            if d.distance as u32 > d.produced {
                return false;
            }
        }
        let value = d.history[d.produced.wrapping_sub(d.distance as u32) as usize & 4095];
        d.remaining -= 1;
        *byte = value;
        d.history[d.produced as usize & 4095] = value;
        d.produced = d.produced.wrapping_add(1);
    }
    true
}

/// Returns true only when a fresh powered connection needs a new playback.
///
/// # Safety
/// `session` points to initialized, exclusively borrowed Session storage.
#[no_mangle]
pub unsafe extern "C" fn ba_session_power(
    session: *mut Session,
    powered: bool,
    connection: u32,
    now: u32,
) -> bool {
    let Some(s) = session.as_mut() else {
        return false;
    };
    let restart = powered && (!s.powered || s.connection != connection);
    s.powered = powered;
    s.connection = connection;
    if restart {
        s.started_ms = now;
        s.finished = false;
    }
    restart
}

/// Uptime-wrap-safe deadline calculation; no floating-point operations.
///
/// # Safety
/// `session` points to initialized Session storage with no concurrent mutation.
#[no_mangle]
pub unsafe extern "C" fn ba_due_frame(session: *const Session, now: u32, fps: u32) -> u32 {
    let Some(s) = session.as_ref() else {
        return 0;
    };
    ((now.wrapping_sub(s.started_ms) as u64 * fps as u64) / 1000) as u32
}

#[cfg(not(test))]
#[panic_handler]
fn panic(_: &core::panic::PanicInfo<'_>) -> ! {
    loop {
        core::hint::spin_loop();
    }
}

#[cfg(test)]
#[path = "../../../../tools/test_bad_apple.rs"]
mod tests;
