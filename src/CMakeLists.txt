add_definitions(${LLVM_CXXFLAGS})
include_directories("${PROJECT_DIR}/${UB_INCLUDE}")

add_subdirectory("arithmetic-overflow")


# Insert your subdirectories here 


file(GLOB Sources "*.cpp")

# Add your target here

add_library(GET_TOGETHER OBJECT ${Sources})


set_target_properties(GET_TOGETHER PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(GET_TOGETHER PUBLIC "-fPIC")


