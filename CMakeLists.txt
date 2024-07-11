cmake_minimum_required(VERSION 3.28)
project(c_client)
project(cpp_client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

#set (CMAKE_EXE_LINKER_FLAGS "-static")
#set (CMAKE_C_FLAGS "-fvisibility=hidden") # -pedantic
#set (CMAKE_CXX_FLAGS "-fvisibility=hidden") # -pedantic

add_compile_definitions(_REENTRANT)
add_compile_definitions(_GNU_SOURCE)
add_compile_definitions(LINUX)

include_directories(/media/disk/setup/oracle/instantclient_23_4/sdk/include/)
link_directories(/media/disk/setup/oracle/instantclient_23_4/)

add_executable(c_client main.c)
target_link_libraries(c_client clntsh)

add_executable(cpp_client main.cpp)
target_link_libraries(cpp_client occi clntsh)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(c_client pthread dl m)
    target_link_libraries(cpp_client pthread dl m)
else()

endif ()
