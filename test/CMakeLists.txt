include_directories ("${PROJECT_SOURCE_DIR}/src")
add_executable (basictest basictest.c)
target_link_libraries (basictest libsimplebloom)

# does the application run
add_test (doesitrun basictest)

IF(UNIX)
  TARGET_LINK_LIBRARIES(basictest m)
ENDIF(UNIX)



