file(GLOB Sources "*.cpp")

add_library(ARITHMETIC_OVERFLOW OBJECT ${Sources})
set_target_properties(ARITHMETIC_OVERFLOW PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(ARITHMETIC_OVERFLOW PUBLIC "-fPIC")
