# SDK project - dynamic library
project(SDK LANGUAGES C CXX)

find_package(spdlog REQUIRED)

add_library(sdk SHARED
  dllmain.cpp
  injector.cpp
  injector.h
  sdk.cpp
  sdk.h
  sdk.def
  
  # Common files
  ${CMAKE_SOURCE_DIR}/com/util.cpp
  ${CMAKE_SOURCE_DIR}/com/util.h
)

target_link_libraries(sdk PRIVATE
  version
  shlwapi
  spdlog::spdlog
  c++
)

# Set compiler definitions
target_compile_definitions(sdk PRIVATE
  SDK_EXPORTS
  _WINDOWS
  _USRDLL
)

# add_compile_options(
#   # -Wall
#   -fPIC
#   # -fms-extensions
# )
# Set output name for debug builds
set_target_properties(sdk PROPERTIES
  DEBUG_POSTFIX "d"
)

# # Post-build copy commands
# add_custom_command(TARGET sdk POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:sdk>
#     ${CMAKE_SOURCE_DIR}/Out
#   COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:sdk>
#     ${CMAKE_SOURCE_DIR}/../clients/python/wcferry
#   COMMAND ${CMAKE_COMMAND} -E copy
#     ${CMAKE_SOURCE_DIR}/DISCLAIMER.md
#     ${CMAKE_SOURCE_DIR}/Out
#   COMMAND ${CMAKE_COMMAND} -E copy
#     ${CMAKE_SOURCE_DIR}/DISCLAIMER.md
#     ${CMAKE_SOURCE_DIR}/../clients/python/wcferry
# )
