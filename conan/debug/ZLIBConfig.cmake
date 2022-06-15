########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()


include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ZLIBTargets.cmake)
include(CMakeFindDependencyMacro)

foreach(_DEPENDENCY ${zlib_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(ZLIB_VERSION_STRING "1.2.12")
set(ZLIB_INCLUDE_DIRS ${zlib_INCLUDE_DIRS_DEBUG} )
set(ZLIB_INCLUDE_DIR ${zlib_INCLUDE_DIRS_DEBUG} )
set(ZLIB_LIBRARIES ${zlib_LIBRARIES_DEBUG} )
set(ZLIB_DEFINITIONS ${zlib_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${zlib_BUILD_MODULES_PATHS_DEBUG} )
    conan_message(STATUS "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()

