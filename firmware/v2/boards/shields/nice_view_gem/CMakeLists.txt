if(CONFIG_ZMK_DISPLAY AND CONFIG_NICE_VIEW_WIDGET_STATUS)
  zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
  zephyr_library_sources(custom_status_screen.c)
  zephyr_library_sources(assets/images.c)
  zephyr_library_sources(widgets/battery.c)
  zephyr_library_sources(widgets/output.c)
  zephyr_library_sources(widgets/util.c)
  
  if(NOT CONFIG_ZMK_SPLIT OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  zephyr_library_sources(widgets/layer.c)
  zephyr_library_sources(widgets/profile.c)
  zephyr_library_sources(widgets/screen.c)
  zephyr_library_sources(widgets/bad_apple.c)
  zephyr_library_sources(widgets/display_power.c)
  # The LVGL widget stays C; allocation-free playback/decoding is Rust.
  find_program(SEAHORSE_RUSTC rustc REQUIRED)
  if(CONFIG_FP_HARDABI)
    set(SEAHORSE_RUST_TARGET thumbv7em-none-eabihf)
  else()
    set(SEAHORSE_RUST_TARGET thumbv7em-none-eabi)
  endif()
  set(SEAHORSE_VIDEO_LIB ${CMAKE_CURRENT_BINARY_DIR}/libseahorse_video.a)
  add_custom_command(
    OUTPUT ${SEAHORSE_VIDEO_LIB}
    COMMAND ${SEAHORSE_RUSTC} --edition=2021 --crate-name seahorse_video
      --crate-type staticlib --target ${SEAHORSE_RUST_TARGET}
      -C panic=abort -C opt-level=z -C embed-bitcode=no
      ${CMAKE_CURRENT_LIST_DIR}/widgets/bad_apple_core.rs -o ${SEAHORSE_VIDEO_LIB}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/widgets/bad_apple_core.rs
      ${CMAKE_CURRENT_LIST_DIR}/widgets/display_power.rs
    VERBATIM)
  add_custom_target(seahorse_video_rust_build DEPENDS ${SEAHORSE_VIDEO_LIB})
  # Link as an external archive, outside Zephyr's whole-archive list. Pulling
  # every Rust/compiler-builtins object would conflict with the C runtime.
  add_library(seahorse_video_rust STATIC IMPORTED GLOBAL)
  set_target_properties(seahorse_video_rust PROPERTIES IMPORTED_LOCATION ${SEAHORSE_VIDEO_LIB})
  add_dependencies(app seahorse_video_rust_build)
  zephyr_link_libraries(seahorse_video_rust)
  else()
    zephyr_library_sources(assets/crystal.c)
    zephyr_library_sources(widgets/animation.c)
    zephyr_library_sources(widgets/screen_peripheral.c)
  endif()
endif()
