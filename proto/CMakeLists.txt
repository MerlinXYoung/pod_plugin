cmake_minimum_required (VERSION 2.6)

project(pod_pb)

file(GLOB proto_file_list ${POD_PROTO}/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto_file_list})


add_library(${PROJECT_NAME} ${PROTO_SRCS})
set(LIBRARY_OUTPUT_PATH ${POD_LIB})