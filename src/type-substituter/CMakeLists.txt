file(GLOB Sources "*.cpp")

set(NAME TYPE_SUBSTITUTER)

add_library(${NAME} OBJECT ${Sources})
set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(${NAME} PUBLIC "-fPIC")

ADD_SOURCE($<TARGET_OBJECTS:${NAME}>)

