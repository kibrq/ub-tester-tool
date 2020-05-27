add_definitions(${LLVM_CXXFLAGS})
include_directories("${PROJECT_SOURCE_DIR}/${UB_INCLUDE}")

add_subdirectory("code-injector")
add_subdirectory("arithmetic-overflow")
add_subdirectory("uninit-variables")
add_subdirectory("index-out-of-bounds")
add_subdirectory("type-substituter")
add_subdirectory("pointers")

# Insert your subdirectories here 


file(GLOB Sources "*.cpp")
# Add your target here

add_library(UTILITY OBJECT ${Sources})

ADD_SOURCE($<TARGET_OBJECTS:UTILITY>)




set_target_properties(UTILITY PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(UTILITY PUBLIC "-fPIC")

