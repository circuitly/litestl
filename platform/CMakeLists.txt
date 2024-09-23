set(LIB
)

set(SRC
  PUBLIC export.h
  PUBLIC time.h
  PUBLIC cpu.h
  common.cc
)

if (NOT BUILD_WASM)
set(SRC 
    ${SRC}
    win32.cc
)
endif()

lt_add_library(platform "${SRC}" "${LIB}" OBJECT)
