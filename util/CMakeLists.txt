set(LIB
    eigen
)

set(SRC
  PUBLIC assert.h
  PUBLIC alloc.h
  PUBLIC boolvector.h
  PUBLIC callback_list.h
  PUBLIC compiler_util.h
  PUBLIC map.h
  PUBLIC rand.h
  PUBLIC set.h
  PUBLIC string.h
  PUBLIC time.h
  PUBLIC task.h
  PUBLIC ordered_set.h
  PUBLIC vector.h
  PUBLIC type_tags.h
  PUBLIC memory.h
  alloc.cc
  util.cc
  string.cc
)

lt_add_library(util "${SRC}" "${LIB}" STATIC)

