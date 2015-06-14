include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/murmur2")
add_subdirectory (murmur2) 

add_library(libsimplebloom bloom.c)
target_link_libraries (libsimplebloom murmur2)

install (TARGETS libsimplebloom DESTINATION lib)
install (FILES bloom.h DESTINATION include)


