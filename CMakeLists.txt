cmake_minimum_required(VERSION 3.14)
project(make)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED COMPONENTS regex filesystem)

add_executable(make main.cpp input.cpp types/variable.hpp types/include.hpp types/rule.hpp types/makefile.hpp input.hpp dependencies.cpp variables.cpp variables.hpp dependencies.hpp argparse/argparse.hpp build/build.hpp build/build.cpp error.hpp)
target_link_libraries(make Boost::regex Boost::filesystem)
