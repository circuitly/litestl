include(CTest)

macro(test file extra_lib)
set(test_target "/usr/bin/g++
 ${file} -o ${file}_out")
#add_executable(${target} "${file}")
#target_link_libraries(${target} util ${extra_lib} math platform)
#target_include_directories(${target} PUBLIC util)
#target_include_directories(${target} PUBLIC math)
#target_include_directories(${target} PUBLIC mesh)
#target_include_directories(${target} PUBLIC platform)
#target_include_directories(${target} PUBLIC binding_generators)
#target_include_directories(${target} PUBLIC binding)

# testing with cross-compiling (i.e. we aren't using wasm) is annoying
# just do it manually
set(basedir "../../..")
set(testflags -I${basedir}/litestl -I${basedir}/source -I${basedir} 
              -I${basedir}/litestl/extern
              -std=c++20
              ${basedir}/litestl/platform/common.cc
              ${basedir}/litestl/platform/linux.cc
              ${basedir}/litestl/util/alloc.cc
              ${basedir}/litestl/util/string.cc
              ${basedir}/litestl/util/util.cc
              ${basedir}/litestl/util/task.cc
              ${basedir}/litestl/math/color.cc
              -lpthread
              -O0
              )

add_test(NAME ${file} COMMAND "g++" ${testflags} "${basedir}/litestl/tests/${file}" "-o" "${file}_out")
#add_test(NAME ${file} COMMAND pwd)
endmacro()

test(test_boolvector.cc "")
test(test_function.cc "")
test(test_map.cc "")
test(test_math_vec.cc "")
test(test_matrix.cc "")
test(test_set.cc "")
test(test_task.cc "")
test(test_vector.cc "")
test(test_shared_ptr.cc "")
