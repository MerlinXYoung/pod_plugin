find_package(Protobuf REQUIRED)

function (PROTOBUF_GENERATE_POD SRCS HDRS) 
    cmake_parse_arguments(protobuf_generate_pod "" "" "" ${ARGN})

  set(_proto_files "${protobuf_generate_pod_UNPARSED_ARGUMENTS}")
  if(NOT _proto_files)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_POD() called without any proto files")
    return()
  endif()
  
    foreach(proto_file ${proto_file_list})
        execute_process(COMMAND ${PROTOC_TOOL}
            --proto_path=${POD_PROTO}
            --proto_path=${ROOT_DIR}/proto
            --proto_path=${PROTOBUF_DIR}
            --cpp_out=${ROOT_DIR}
            --pod_out=${ROOT_DIR}
            --plugin=protoc-gen-pod=${POD_LIB}/pod_plugin
            ${proto_file} RESULT_VARIABLE retcode)
        if (NOT "${retcode}" STREQUAL "0")
            message(FATAL_ERROR "${BoldRed}Fatal Error${ColourReset}: protoc ${proto_file}")
        else()
            message(STATUS "${Green} ${proto_file} processed by protoc${ColourReset}")
        endif()
    endforeach()
endfunction()


function(PROTOBUF_GENERATE_POD_CPP SRCS HDRS)
  cmake_parse_arguments(protobuf_generate_pod_cpp "" "EXPORT_MACRO;DESCRIPTORS" "" ${ARGN})

  set(_proto_files "${protobuf_generate_pod_cpp_UNPARSED_ARGUMENTS}")
  if(NOT _proto_files)
    message(SEND_ERROR "Error: PROTOBUF_GENERATE_POD_CPP() called without any proto files")
    return()
  endif()

  if(PROTOBUF_GENERATE_POD_CPP_APPEND_PATH)
    set(_append_arg APPEND_PATH)
  endif()

  if(protobuf_generate_pod_cpp_DESCRIPTORS)
    set(_descriptors DESCRIPTORS)
  endif()

  if(DEFINED PROTOBUF_IMPORT_DIRS AND NOT DEFINED Protobuf_IMPORT_DIRS)
    set(Protobuf_IMPORT_DIRS "${PROTOBUF_IMPORT_DIRS}")
  endif()

  if(DEFINED Protobuf_IMPORT_DIRS)
    set(_import_arg IMPORT_DIRS ${Protobuf_IMPORT_DIRS})
  endif()

  set(_outvar)
  protobuf_generate(${_append_arg} ${_descriptors} LANGUAGE pod EXPORT_MACRO ${protobuf_generate_pod_cpp_EXPORT_MACRO} OUT_VAR _outvar ${_import_arg} PROTOS ${_proto_files})

  set(${SRCS})
  set(${HDRS})
  if(protobuf_generate_pod_cpp_DESCRIPTORS)
    set(${protobuf_generate_pod_cpp_DESCRIPTORS})
  endif()

  foreach(_file ${_outvar})
    if(_file MATCHES "cc$")
      list(APPEND ${SRCS} ${_file})
    elseif(_file MATCHES "desc$")
      list(APPEND ${protobuf_generate_pod_cpp_DESCRIPTORS} ${_file})
    else()
      list(APPEND ${HDRS} ${_file})
    endif()
  endforeach()
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
  if(protobuf_generate_pod_cpp_DESCRIPTORS)
    set(${protobuf_generate_pod_cpp_DESCRIPTORS} "${${protobuf_generate_pod_cpp_DESCRIPTORS}}" PARENT_SCOPE)
  endif()
endfunction()