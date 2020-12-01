file(GLOB SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.c *.h)

add_executable(batteryd  ${SOURCES})
install(TARGETS batteryd RUNTIME DESTINATION bin)
