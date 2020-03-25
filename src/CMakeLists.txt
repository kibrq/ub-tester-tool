
add_definitions(${LLVM_CXXFLAGS})
include_directories("${PROJECT_DIR}/${UB_INCLUDE}")

#add_subdirectory("index-out-of-bounds")
#set(indexOutOfBounds $<TARGET_OBJECTS:INDEX_OUT_OF_BOUNDS>)


# Insert your subdirectories here 


file(GLOB Sources "*.cpp")
add_executable(${UB_EXE} ${Sources}
  )

set_target_properties(${UB_EXE} PROPERTIES COMPILE_FLAGS "-fno-rtti -std=c++17")
target_compile_options(${UB_EXE} PUBLIC "-fPIC")

target_link_libraries(${UB_EXE}
  LLVMSupport
  clangTooling
  clangBasic
  clangASTMatchers
  )

target_link_libraries(${UB_EXE} pthread z dl)
