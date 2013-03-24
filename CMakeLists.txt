project(Convolver)
cmake_minimum_required(VERSION 2.6)

# set the output destination
set(CONVOLVER_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Linux/build/Convolver.so)

# create a custom target that will run each time you type make 
add_custom_target(build_Convolver ALL 
                  COMMAND ${CMAKE_MAKE_PROGRAM}
                  WORKING_DIRECTORY
                  ${CMAKE_CURRENT_SOURCE_DIR}/Builds/Linux
                  COMMENT "Building Introjucer generated Makefile")

# now create an imported static target
add_library(Convolver SHARED IMPORTED)
set_target_properties(Convolver PROPERTIES PREFIX "")
set_property(TARGET Convolver APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(Convolver PROPERTIES IMPORTED_LOCATION_NOCONFIG "${CONVOLVER_LIBRARY}")

add_dependencies(Convolver build_Convolver)
