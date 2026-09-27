add_executable(hierstate_group group.cpp)
target_link_libraries(hierstate_group PRIVATE hierstate::hierstate)

if(MSVC)
    target_compile_options(hierstate_group PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(hierstate_group PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME hierstate_group COMMAND hierstate_group)

add_executable(hierstate_hierarchy hierarchy.cpp)
target_link_libraries(hierstate_hierarchy PRIVATE hierstate::hierstate)

if(MSVC)
    target_compile_options(hierstate_hierarchy PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(hierstate_hierarchy PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME hierstate_hierarchy COMMAND hierstate_hierarchy)
