include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable (bloomutil bloomutil.c)
target_link_libraries (bloomutil libsimplebloom)

install (TARGETS bloomutil DESTINATION bin)

IF(UNIX)
  TARGET_LINK_LIBRARIES(bloomutil m)
ENDIF(UNIX)


