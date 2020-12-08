cmake_minimum_required (VERSION 2.6)

project(pod_plugin_root)

include(common.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Threads REQUIRED)
# can use your own protobuf directory here
# SET(PROTOBUF_ROOT_DIR ${ROOT_DIR}/third_party/protobuf)
list(APPEND CMAKE_PREFIX_PATH /usr/local/protobuf)
find_package(Protobuf REQUIRED)

include_directories(${Protobuf_INCLUDE_DIR})

SET(PROTOBUF_DIR ${PROTOBUF_ROOT_DIR}/src/)
# SET(PROTOC_TOOL ${PROTOBUF_DIR}/protoc)
SET(PROTOBUF_LIB ${PROTOBUF_DIR}/.libs/)

SET(POD_PROTO ${ROOT_DIR}/proto)
SET(POD_LIB ${ROOT_DIR}/lib)

ADD_SUBDIRECTORY(proto)
ADD_SUBDIRECTORY(src)

ADD_SUBDIRECTORY(test)
