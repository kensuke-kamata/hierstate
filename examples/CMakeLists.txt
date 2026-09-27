add_executable(hierstate_sequence sequence.cpp)
target_link_libraries(hierstate_sequence PRIVATE hierstate)

if(MSVC)
    target_compile_options(hierstate_sequence PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(hierstate_sequence PRIVATE -Wall -Wextra -Wpedantic)
endif()
