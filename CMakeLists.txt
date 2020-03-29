cmake_minimum_required(VERSION 3.2)
project("ub-tester-tool")

if ( NOT DEFINED ENV{LLVM_HOME})
    message(FATAL_ERROR "$LLVM_HOME is not defined")
else()
    message(STATUS "$LLVM_HOME found: $ENV{LLVM_HOME}")
    set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    set(LLVM_HOME $ENV{LLVM_HOME} CACHE PATH "Root of LLVM installation")
    set(LLVM_INC ${LLVM_HOME}/include)
    set(LLVM_LIB ${LLVM_HOME}/lib)
    set(LLVM_BIN ${LLVM_HOME}/bin)
    set(LLVM_CMAKE ${LLVM_LIB}/cmake/llvm)

    find_package(LLVM REQUIRED CONFIG)
    find_package(Clang REQUIRED CONFIG)

    set(CMAKE_C_COMPILER ${LLVM_BIN}/clang)
    set(CMAKE_CXX_COMPILER ${LLVM_BIN}/clang++)

    # Generate compile_commands.json for our tool (need to run the tool with -p compile_commands.json)
    set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE) 

    add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)

    # Get the Clang version within the current LLVM installation
    EXECUTE_PROCESS( COMMAND ${LLVM_BIN}/clang++ --version OUTPUT_VARIABLE clang_full_version_string )
    # Extract the numeric value to a string e.g 10.0.0
    string (REGEX REPLACE ".*clang version ([0-9]+\\.[0-9]+\\.[0-9]+).*" "\\1" CLANG_VERSION_STRING 
      "${clang_full_version_string}")

    # Include clang builtins they are added in compile_commands.json and thus included by our tool
    include_directories(${LLVM_LIB}/clang/${CLANG_VERSION_STRING}/include) 
    include_directories(${LLVM_INCLUDE_DIRS})

    link_directories(${LLVM_LIBRARY_DIRS})

    set(UB_EXE ub-tester)
    set(UB_INCLUDE include)
    set(UB_SRC src)

    add_subdirectory(${UB_SRC})
    set(uninit-variables $<TARGET_OBJECTS:UNINIT_VARIABLES>)

    add_executable(${UB_EXE} $<TARGET_OBJECTS:GET_TOGETHER> ${uninit-variables})

    target_link_libraries(${UB_EXE}
        LLVMSupport
        clangTooling
        clangBasic
        )

    target_link_libraries(${UB_EXE} pthread z dl)
    
endif()
