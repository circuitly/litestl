set(LIB
    PUBLIC eigen
    util
)

set(SRC
  PUBLIC vector.h
  PUBLIC color.h
  PUBLIC lut.h
  PUBLIC geom.h 
  PUBLIC quat.h
  PUBLIC matrix.h
  PUBLIC bspline.h
  PUBLIC mix.h
  color.cc
)

lt_add_library(math "${SRC}" "${LIB}" STATIC)

