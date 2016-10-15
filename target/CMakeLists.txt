include_directories("${WOODY_PROJECT_DIR}/include")

# Disable fopen is insecure warning
add_compile_options("/wd4996")

add_executable(woody woody.c ${WOODY_SOURCES})

set_property(TARGET woody PROPERTY FOLDER "executables")

install(TARGETS woody RUNTIME DESTINATION ${WOODY_PROJECT_DIR}/build)
