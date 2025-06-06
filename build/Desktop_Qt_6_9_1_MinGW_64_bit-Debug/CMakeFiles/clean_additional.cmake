# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CyberScanner_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CyberScanner_autogen.dir\\ParseCache.txt"
  "CyberScanner_autogen"
  )
endif()
