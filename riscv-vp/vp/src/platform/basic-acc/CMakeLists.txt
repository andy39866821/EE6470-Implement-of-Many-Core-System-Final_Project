file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-basic-acc
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-basic-acc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-acc
        main.cpp)

target_link_libraries(riscv-vp-acc rv32 platform-basic-acc platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
