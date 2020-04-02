file(GLOB Sources "*.cpp")

add_library(UNINIT_VARIABLES OBJECT ${Sources})
set_target_properties(UNINIT_VARIABLES PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(UNINIT_VARIABLES PUBLIC "-fPIC")