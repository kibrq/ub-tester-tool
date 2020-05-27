file(GLOB Sources "*.cpp")

add_library(CODE_INJECTOR OBJECT ${Sources})
set_target_properties(CODE_INJECTOR PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(CODE_INJECTOR PUBLIC "-fPIC")

ADD_SOURCE($<TARGET_OBJECTS:CODE_INJECTOR>)

