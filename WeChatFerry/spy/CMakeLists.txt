# Spy project - dynamic library
project(Spy LANGUAGES C CXX)

find_package(spdlog REQUIRED)
find_package(magic_enum REQUIRED)
find_package(minhook CONFIG REQUIRED)
find_package(nng REQUIRED)

add_library(spy SHARED
  chatroom_manager.cpp
  chatroom_manager.h
  misc_manager.cpp
  misc_manager.h
  database_executor.cpp
  database_executor.h
  contact_manager.cpp
  contact_manager.h
  message_handler.cpp
  message_handler.h
  rpc_server.cpp
  rpc_server.h
  message_sender.cpp
  message_sender.h
  spy.cpp
  spy.h
  spy_types.h
  account_manager.cpp
  account_manager.h
  resource.h
  rpc_helper.h
  sqlite3.h
  dllmain.cpp
  spy.def

  # Common files
  ${CMAKE_SOURCE_DIR}/com/util.cpp
  ${CMAKE_SOURCE_DIR}/com/util.h
  ${CMAKE_SOURCE_DIR}/com/log.hpp

  # RPC files
  ${CMAKE_SOURCE_DIR}/rpc/pb_util.cpp
  ${CMAKE_SOURCE_DIR}/rpc/pb_util.h
  ${CMAKE_SOURCE_DIR}/rpc/pb_types.h
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb.h
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_common.h
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_decode.h
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_encode.h
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_common.c
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_decode.c
  ${CMAKE_SOURCE_DIR}/rpc/nanopb/pb_encode.c
  ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.c
  ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.h
)

# link directories
target_link_directories(spy PRIVATE
  ${CMAKE_SOURCE_DIR}/smc
)

# Link dependencies
target_link_libraries(spy PRIVATE 
  Codec
  mp3lame
  version
  shlwapi
  iphlpapi
  wsock32
  ws2_32
  crypt32
  magic_enum::magic_enum
  nng::nng
  spdlog::spdlog
  minhook::minhook
  c++
)

add_custom_command(
  OUTPUT
    ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.c
    ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.h
  COMMAND ${CMAKE_SOURCE_DIR}/rpc/tool/protoc --nanopb_out=${CMAKE_SOURCE_DIR}/rpc/proto wcf.proto
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/rpc/proto
  DEPENDS ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.proto
  COMMENT "Generated protobuf files"
)

# Add generated files as dependencies
add_custom_target(protobuf_generation
  DEPENDS
    ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.c
    ${CMAKE_SOURCE_DIR}/rpc/proto/wcf.pb.h
)
add_dependencies(spy protobuf_generation)

# Set output name for debug builds
set_target_properties(spy PROPERTIES
  DEBUG_POSTFIX "d"
)

# Post-build copy commands
# add_custom_command(TARGET spy POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:spy>
#     ${CMAKE_SOURCE_DIR}/Out
#   COMMAND ${CMAKE_COMMAND} -E copy
#     $<TARGET_FILE:spy>
#     ${CMAKE_SOURCE_DIR}/../clients/python/wcferry
# )
