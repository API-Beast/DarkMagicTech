project(darkmagictech)

add_executable(darkmagictech TagRuleSet.cpp main.cpp Strings.cpp ConfigFile.cpp)
ADD_DEFINITIONS(-std=c++11)

install(TARGETS darkmagictech RUNTIME DESTINATION bin)
