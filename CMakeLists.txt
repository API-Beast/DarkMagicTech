project(darkmagictech)

cmake_minimum_required(VERSION 2.8)

add_executable(darkmagictech TagRuleSet.cpp main.cpp Strings.cpp Parsing/UTF8.cpp Parsing/StringParser.cpp Parsing/ConfigFile.cpp)
ADD_DEFINITIONS(-std=c++11)

install(TARGETS darkmagictech RUNTIME DESTINATION bin)
