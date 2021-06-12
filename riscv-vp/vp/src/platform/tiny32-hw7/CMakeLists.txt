file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-tiny32-hw7
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-tiny32-hw7 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-hw7
        main.cpp)

target_link_libraries(riscv-vp-hw7 rv32 platform-tiny32-hw7 platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
