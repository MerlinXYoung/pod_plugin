cmake_minimum_required (VERSION 2.6)
project(TEST CXX)
include(../common.cmake)
list(APPEND CMAKE_PREFIX_PATH /usr/local/protobuf)
# include(../protobuf.pod.cmake)

list(APPEND CMAKE_PREFIX_PATH /usr/local/googletest)
find_package(GTest REQUIRED)

# can use your own protobuf directory here
# SET(PROTOBUF_ROOT_DIR ${ROOT_DIR}/../third_party/protobuf)
# SET(PROTOBUF_DIR ${PROTOBUF_ROOT_DIR}/src)
# SET(PROTOC_TOOL ${PROTOBUF_DIR}/protoc)
SET(PROTOC_TOOL ${Protobuf_PROTOC_EXECUTABLE})
SET(PROTOBUF_LIB ${PROTOBUF_DIR}/.libs)

# can use your own googletest directory here

SET(GTEST_INCLUDE ${GTEST_ROOT}/googletest/include)
SET(GTEST_LIB ${GTEST_ROOT}/build/lib)

SET(POD_PROTO ${ROOT_DIR}/../proto)
SET(POD_LIB ${ROOT_DIR}/../lib)
set(POD_INC ${ROOT_DIR}/../build/proto)


file(GLOB proto_file_list proto/*.proto)

# PROTOBUF_GENERATE_POD_CPP(PROTO_SRCS PROTO_HDRS ${proto_file_list})
foreach(proto_file ${proto_file_list})
    string(REGEX REPLACE ".*/" "" proto_name ${proto_file})
    string(REGEX REPLACE ".proto" "" base_name ${proto_name})
    message(STATUS "proto:${proto_name} base_name:${base_name}")
    add_custom_command(OUTPUT ${base_name}.pod.cc ${base_name}.pod.h ${base_name}.pb.cc ${base_name}.pb.h
        COMMAND ${PROTOC_TOOL} --proto_path=${POD_PROTO} --proto_path=${ROOT_DIR}/proto --proto_path=${PROTOBUF_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} --pod_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=${POD_LIB}/protoc-gen-pod ${proto_name}
        MAIN_DEPENDENCY ${proto_file}
        DEPENDS protoc-gen-pod
        COMMENT "${base_name}"
    )
    LIST(APPEND PROTO_SRCS ${base_name}.pod.cc)
    LIST(APPEND PROTO_SRCS ${base_name}.pb.cc)
endforeach()
# foreach(proto_file ${proto_file_list})
#     execute_process(COMMAND ${PROTOC_TOOL}
#         --proto_path=${POD_PROTO}
#         --proto_path=${ROOT_DIR}/proto
#         --proto_path=${PROTOBUF_DIR}
#         --cpp_out=${ROOT_DIR}
#         --pod_out=${ROOT_DIR}
#         --plugin=protoc-gen-pod=${POD_LIB}/pod_plugin
#         ${proto_file} RESULT_VARIABLE retcode)
#     if (NOT "${retcode}" STREQUAL "0")
#         message(FATAL_ERROR "${BoldRed}Fatal Error${ColourReset}: protoc ${proto_file}")
#     else()
#         message(STATUS "${Green} ${proto_file} processed by protoc${ColourReset}")
#     endif()
# endforeach()

include_directories(${PROTOBUF_DIR}
    ${GTEST_INCLUDE}
    ${POD_PROTO}
    ${POD_INC}
    )

link_directories(${PROTOBUF_LIB} ${GTEST_LIB} ${POD_LIB})

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${PROTO_SRCS})

target_link_libraries(${PROJECT_NAME}
GTest::GTest
    libpod_pb.a
    protobuf::libprotobuf
    Threads::Threads
    )
