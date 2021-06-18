file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-tiny32-systolic-dnn
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-tiny32-systolic-dnn PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-systolic-dnn
        main.cpp)

target_link_libraries(riscv-vp-systolic-dnn rv32 platform-tiny32-systolic-dnn platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
