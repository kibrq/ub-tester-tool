file(GLOB Sources "*.cpp")

add_library(INDEX_OUT_OF_BOUNDS OBJECT ${Sources})
set_target_properties(INDEX_OUT_OF_BOUNDS PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(INDEX_OUT_OF_BOUNDS PUBLIC "-fPIC")


