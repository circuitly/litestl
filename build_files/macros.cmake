macro(lt_add_library target src lib lib_type)
  add_library(${target} ${lib_type})

  set(DEFAULT_SCOPE INTERFACE)
  set(SCOPE ${DEFAULT_SCOPE})

  set(LIB_LIST ${lib})  
  #Dependencies
  foreach(path IN LISTS LIB_LIST)
    if(path MATCHES "PRIVATE" OR path MATCHES "PUBLIC" OR path MATCHES "INTERFACE")
      set(SCOPE ${path})
    else()
      #message(STATUS, "-> ${target} ${SCOPE} ${file_type} ${path}")
      target_link_libraries(${target} ${path})
      target_include_directories(${target} PUBLIC ${path})

      if(MSVC)
        target_compile_options(${target} PUBLIC /std:c++20)
      endif()

      #Reset back to default scope
      set(SCOPE ${DEFAULT_SCOPE})
    endif()
  endforeach()

  #Make it so other libraries include [dir]/header,
  #so we don't have to use XXX_ prefixes to avoid
  #header file name collisions.
  if(${lib_type} MATCHES "INTERFACE")
      target_include_directories(${target} INTERFACE ..)
      target_include_directories(${target} INTERFACE ../..)
  else()
      target_include_directories(${target} PUBLIC ..)
      target_include_directories(${target} PUBLIC ../..)
  endif()

  set(SRC_LIST ${src})

  if(${lib_type} MATCHES "INTERFACE")
    set(DEFAULT_SCOPE INTERFACE)
  else()
    set(DEFAULT_SCOPE PRIVATE)
  endif()

  #Source files
  set(SCOPE ${DEFAULT_SCOPE})
  foreach(path IN LISTS SRC_LIST)
    if(path MATCHES "PRIVATE" OR path MATCHES "PUBLIC" OR path MATCHES "INTERFACE")
      set(SCOPE ${path})
    else()
      if (path MATCHES ".*\\.h")
        set(file_type "")
      else()
        set(file_type "") 
      endif()

      #message(STATUS, "-> ${target} ${SCOPE} ${file_type} ${path}")
      target_sources(${target} ${SCOPE} ${file_type} ${path})

      #Reset back to default scope
      set(SCOPE ${DEFAULT_SCOPE})
    endif()
  endforeach()
endmacro()
