include_directories("${WOODY_PROJECT_DIR}/include")

# add_definitions(-DWOODY_WINDOWS)

# Disable fopen is insecure warning
add_compile_options("/wd4996")

add_executable(woody woody.c ${WOODY_SOURCES})

target_link_libraries(woody kernel32.lib)

install(TARGETS woody RUNTIME DESTINATION ${WOODY_PROJECT_DIR}/build)
