//! Tests of the same Rust decoder/session code that runs on-device.
//! See ../boards/shields/nice_view_gem/BAD_APPLE.md for the build commands.
use super::{ba_decode, ba_due_frame, ba_session_power, Decoder, Session};
fn decoder() -> Decoder {
    unsafe { std::mem::zeroed() }
}
fn decode(d: &mut Decoder, data: &[u8], output: &mut [u8]) -> bool {
    unsafe {
        ba_decode(
            d,
            data.as_ptr(),
            data.len(),
            output.as_mut_ptr(),
            output.len(),
        )
    }
}

#[test]
fn once_per_power_connection_and_no_restart_from_usb_state_updates() {
    let mut s = Session::default();
    unsafe {
        assert!(!ba_session_power(&mut s, false, 0, 1));
        assert!(ba_session_power(&mut s, true, 1, 2));
        assert!(!ba_session_power(&mut s, true, 1, 3));
        s.finished = true;
        assert!(!ba_session_power(&mut s, true, 1, 4));
        assert!(s.finished);
        assert!(!ba_session_power(&mut s, false, 1, 5));
        assert!(ba_session_power(&mut s, true, 2, 6));
        assert!(!s.finished);
        assert_eq!(s.started_ms, 6);
        // A disconnect/reconnect coalesced into one display callback is fresh too.
        assert!(ba_session_power(&mut s, true, 3, 7));
    }
}

#[test]
fn clock_wrap_and_exact_video_rate() {
    let mut s = Session::default();
    unsafe {
        ba_session_power(&mut s, true, 1, u32::MAX - 99);
        assert_eq!(ba_due_frame(&s, 0, 12), 1);
        assert_eq!(ba_due_frame(&s, 900, 12), 12);
        assert_eq!(ba_due_frame(&s, 218983, 12), 2628);
    }
}

#[test]
fn invalid_and_truncated_streams_fail_closed() {
    assert!(!decode(&mut decoder(), &[], &mut [0; 1]));
    assert!(!decode(&mut decoder(), &[0, 0], &mut [0; 1]));
    assert!(!decode(&mut decoder(), &[0, 0, 0], &mut [0; 1]));
    assert!(!decode(&mut decoder(), &[1], &mut [0; 1]));
}

#[test]
fn overlapping_match_can_cross_frame_boundary() {
    // One literal followed by an 18-byte, distance-one overlapping copy.
    let stream = [1, 0x96, 15, 0, 0];
    let mut d = decoder();
    let mut output = [0; 7];
    assert!(decode(&mut d, &stream, &mut output));
    assert_eq!(output, [0x96; 7]);
    let mut rest = [0; 12];
    assert!(decode(&mut d, &stream, &mut rest));
    assert_eq!(rest, [0x96; 12]);
}

#[test]
fn maximum_length_match_and_truncated_extension() {
    let mut output = [0; 274];
    assert!(decode(&mut decoder(), &[1, 0x96, 15, 0, 255], &mut output));
    assert_eq!(output, [0x96; 274]);
    assert!(!decode(&mut decoder(), &[1, 0x96, 15, 0], &mut [0; 2]));
}

#[test]
fn entire_embedded_video_decodes_through_production_codec() {
    let source = include_str!("../boards/shields/nice_view_gem/assets/bad_apple_data.inc");
    let body = source.split_once("= {").unwrap().1;
    let data: Vec<u8> = body
        .split(',')
        .filter_map(|s| {
            s.trim()
                .strip_prefix("0x")
                .and_then(|h| u8::from_str_radix(h, 16).ok())
        })
        .collect();
    assert_eq!(data.len(), 191264);
    let mut d = decoder();
    let mut hash = 0xcbf29ce484222325u64;
    for _ in 0..2629 {
        let mut frame = [0; 56 * 42 / 8];
        assert!(decode(&mut d, &data, &mut frame));
        hash = frame
            .iter()
            .fold(hash, |h, b| (h ^ *b as u64).wrapping_mul(0x100000001b3));
    }
    assert_eq!(d.offset, data.len());
    assert_eq!(d.remaining, 0);
    assert_eq!(hash, 0xa7596e010312d4ee);
}
