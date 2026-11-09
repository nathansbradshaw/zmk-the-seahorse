# Keep the PS/2 transport, settings, and behavior module pinned in west.yml,
# replacing only its input driver with the source maintained in this profile.
# module.yml orders this module after that dependency.
if(CONFIG_ZMK_INPUT_MOUSE_PS2)
  set(SEAHORSE_UPSTREAM_PS2_INPUT
    "${ZEPHYR_KB_ZMK_PS2_MOUSE_TRACKPOINT_DRIVER_MODULE_DIR}/src/drivers/input/input_mouse_ps2.c")
  get_target_property(SEAHORSE_APP_SOURCES app SOURCES)
  list(FIND SEAHORSE_APP_SOURCES "${SEAHORSE_UPSTREAM_PS2_INPUT}" SEAHORSE_PS2_SOURCE_INDEX)
  if(SEAHORSE_PS2_SOURCE_INDEX EQUAL -1)
    message(FATAL_ERROR "Pinned PS/2 input source was not registered; review module ordering/source changes")
  endif()
  list(REMOVE_AT SEAHORSE_APP_SOURCES ${SEAHORSE_PS2_SOURCE_INDEX})
  set_property(TARGET app PROPERTY SOURCES "${SEAHORSE_APP_SOURCES}")
  target_sources(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/drivers/input/input_mouse_ps2.c)
  message(STATUS "Seahorse: using local PS/2 input driver with configured reset GPIO support")
endif()
