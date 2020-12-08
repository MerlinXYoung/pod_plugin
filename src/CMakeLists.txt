cmake_minimum_required (VERSION 2.6)

project(protoc-gen-pod)

file(GLOB_RECURSE SOURCE *.cpp *.cc)

include_directories(${PROJECT_SOURCE_DIR}
    ${PROTOBUF_DIR}
    ${POD_PROTO}
    ${CMAKE_CURRENT_BINARY_DIR}/../proto
    )

link_directories(${PROTOBUF_LIB})

add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME} 
    ${POD_LIB}/libpod_pb.a
    protobuf::libprotobuf
    protobuf::libprotoc
    Threads::Threads
    )

set(EXECUTABLE_OUTPUT_PATH ${POD_LIB})
add_dependencies(${PROJECT_NAME} pod_pb)
