# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_brainswarm_robotic_arm_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED brainswarm_robotic_arm_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(brainswarm_robotic_arm_FOUND FALSE)
  elseif(NOT brainswarm_robotic_arm_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(brainswarm_robotic_arm_FOUND FALSE)
  endif()
  return()
endif()
set(_brainswarm_robotic_arm_CONFIG_INCLUDED TRUE)

# output package information
if(NOT brainswarm_robotic_arm_FIND_QUIETLY)
  message(STATUS "Found brainswarm_robotic_arm: 0.0.0 (${brainswarm_robotic_arm_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'brainswarm_robotic_arm' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${brainswarm_robotic_arm_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(brainswarm_robotic_arm_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${brainswarm_robotic_arm_DIR}/${_extra}")
endforeach()
