add_executable(cyber_shell host_shell.cpp)
target_link_libraries(cyber_shell PRIVATE cyberengine)
target_compile_options(cyber_shell PRIVATE -Wall -Wextra)
