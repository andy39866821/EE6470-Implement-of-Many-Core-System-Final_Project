file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(platform-tiny32-dnn-2-cores
ethernet.cpp
display.cpp
${HEADERS})

target_include_directories(platform-tiny32-dnn-2-cores PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(riscv-vp-dnn-2-cores
        main.cpp)

target_link_libraries(riscv-vp-dnn-2-cores rv32 platform-tiny32-dnn-2-cores platform-common gdb-mc ${Boost_LIBRARIES} ${SystemC_LIBRARIES} pthread)

INSTALL(TARGETS riscv-vp RUNTIME DESTINATION bin)
