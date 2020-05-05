file(GLOB Sources "*.cpp")

add_library(UB_UTILITY OBJECT ${Sources})
set_target_properties(UB_UTILITY PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(UB_UTILITY PUBLIC "-fPIC")

ADD_SOURCE($<TARGET_OBJECTS:UB_UTILITY>)

