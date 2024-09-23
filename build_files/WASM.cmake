#note: we have debugging info enabled for sourcemaps
set(CMAKE_CXX_COMPILER python "${CMAKE_SOURCE_DIR}/build_files/emsdk_env.py" "${EMSDK_PATH}" emcc "-g -gsource-map -DWASM" )
set(CMAKE_CXX_FLAGS CACHE STRING "flags")
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "flags")
set(CMAKE_C_COMPILER python "${CMAKE_SOURCE_DIR}/build_files/emsdk_env.py" "${EMSDK_PATH}" emcc "-g -gsource-map -DWASM" )
set(CMAKE_C_FLAGS CACHE STRING "flags")
set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "flags")

set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "linker flags")
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "linker flags")
set(CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "linker flags")
set(CMAKE_LINKER python "${CMAKE_SOURCE_DIR}/build_files/emsdk_env.py" "${EMSDK_PATH}" emcc)

set(CMAKE_SYSTEM_NAME WASM)
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_CXX_LINKER_DEPFILE_SUPPORTED false)
set(CMAKE_C_LINKER_DEPFILE_SUPPORTED false)