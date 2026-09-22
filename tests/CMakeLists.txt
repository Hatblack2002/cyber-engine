add_executable(cyber_tests unit/test_main.cpp)
target_link_libraries(cyber_tests PRIVATE cyberengine)
target_compile_options(cyber_tests PRIVATE -Wall -Wextra)
add_test(NAME cyber COMMAND cyber_tests)
