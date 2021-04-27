# Cmake file for Valkka library extensions
# (C) 2018 Sampsa Riikonen
#
# cmake .
# make
# (or make VERBOSE=1)
#
#
# - Compiles and links a cpp shared library, named "valkka_nv"
# - Generates with SWIG python bindings for the library
# - Creates a package that encapsulates the cpp library and its python bindings
# - The python part is installed as a namespace package, under namespace "valkka.hw"
#

cmake_minimum_required(VERSION 3.1)
find_package(PkgConfig REQUIRED)

project(valkka_nv)

# **** INPUT VARIABLES ****

# ** Where valkka, live555 and ffmpeg headers are installed **
# ** (by default, all in the same place **
SET(VALKKA_ROOT "/usr/include/valkka")
option(valkka_root "valkka_root" OFF)
if    (valkka_root)
  SET(VALKKA_ROOT ${valkka_root})
endif (valkka_root)

SET(LIVE555_ROOT "${VALKKA_ROOT}")
option(live555_root "live555_root" OFF)
if    (live555_root)
  SET(LIVE555_ROOT ${live555_root})
endif (live555_root)

SET(FFMPEG_ROOT "${VALKKA_ROOT}")
option(ffmpeg_root "ffmpeg_root" OFF)
if    (ffmpeg_root)
  SET(FFMPEG_ROOT ${ffmpeg_root})
endif (ffmpeg_root)

SET(CUDA_ROOT "/usr/local/cuda")
option(cuda_root "cuda_root" OFF)
if      (cuda_root)
  SET(CUDA_ROOT ${cuda_root})
endif   (cuda_root)

SET(NVCODEC_ROOT ${nvcodec_root})

message("CUDA ROOT           : " ${CUDA_ROOT})
message("NVCODEC ROOT        : " ${NVCODEC_ROOT})

## ** Where your custom-compiled library is located **
#SET(DEMO_ROOT "${CMAKE_SOURCE_DIR}/ext/demo")
#option(demo_root "demo_root" OFF)
#if    (demo_root)
#  SET(DEMO_ROOT ${demo_root})
#endif (demo_root)

# ****************************************************************
# WARNING: the following three lines are modified by the "setver.bash" script
SET(MAJOR_VERSION "1") 
SET(MINOR_VERSION "0") 
SET(PATCH_VERSION "0") 
set(VERSION_STRING ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})

# *** define build type: Debug or Release # now from the command line
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)

# ****************************************************************

# [some directory definitions]
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# [dependencies]
# exec_program(pkg-config ARGS --cflags x11 glew OUTPUT_VARIABLE GL_CFLAGS)
# exec_program(pkg-config ARGS --libs x11 glew OUTPUT_VARIABLE GL_LIBS)
# pkg_search_module(PYTHON REQUIRED python3)
# populates *_INCLUDE_DIRS and *_LIBRARIES variables

message("*** PYTHON INTERFACE ***")
# pkg_search_module(PYTHON REQUIRED python3) # don't use! https://bugs.python.org/issue36721
exec_program(python3-config ARGS --includes OUTPUT_VARIABLE PYTHON_INCLUDE_DIRS_TMP)
# exec_program(python3-config ARGS --libs --embed OUTPUT_VARIABLE PYTHON_LIBRARIES_TMP RETURN_VALUE RETVAL)
exec_program(python3-config ARGS --ldflags --embed OUTPUT_VARIABLE PYTHON_LIBRARIES RETURN_VALUE RETVAL)
#message("RETVAL: ${RETVAL}")
if(RETVAL)
    message("python3-config failed with --embed, so will re-run without it")
    # exec_program(python3-config ARGS --libs OUTPUT_VARIABLE PYTHON_LIBRARIES_TMP RETURN_VALUE RETVAL)
    exec_program(python3-config ARGS --ldflags OUTPUT_VARIABLE PYTHON_LIBRARIES RETURN_VALUE RETVAL)
else(RETVAL)
    # nada
endif(RETVAL)
#string(REPLACE " -l" " " PYTHON_LIBRARIES_TMP ${PYTHON_LIBRARIES_TMP})
#string(REPLACE " " ";" PYTHON_LIBRARIES ${PYTHON_LIBRARIES_TMP})
string(REPLACE "-I" " " PYTHON_INCLUDE_DIRS_TMP ${PYTHON_INCLUDE_DIRS_TMP})
string(REPLACE " " ";" PYTHON_INCLUDE_DIRS ${PYTHON_INCLUDE_DIRS_TMP})
message("PYTHON_LIBRARIES: ${PYTHON_LIBRARIES}")
message("PYTHON_INCLUDE_DIRS: ${PYTHON_INCLUDE_DIRS}")

# pkg_search not used for valkka
SET(VALKKA_LIBRARIES "-lValkka")
SET(VALKKA_INCLUDE_DIRS "${VALKKA_ROOT}/include")

message("*** PYTHON INTERFACE ***")
# execute_process(COMMAND python3 -c "from distutils import sysconfig; print(sysconfig.get_python_lib(),end='')" OUTPUT_VARIABLE PYTHON_DIR)
set(PYTHON_DIR "lib/python3/dist-packages") # just hard-code it
message("PYTHON INSTALL DIR           : " ${PYTHON_DIR})

# execute_process(COMMAND python3 -c "import numpy; print(numpy.get_include())" OUTPUT_VARIABLE NUMPY_INCLUDE_DIR)
# message("PYTHON NUMPY HEADER FILES IN : " ${NUMPY_INCLUDE_DIR})

execute_process(
  COMMAND ./makeswig.bash ${VALKKA_INCLUDE_DIRS}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/include
)

execute_process(
  COMMAND swig -python -c++ -o ${CMAKE_SOURCE_DIR}/python/valkka/nv/valkka_nv_wrap.cpp -outdir ${CMAKE_SOURCE_DIR}/python/valkka/nv ${CMAKE_SOURCE_DIR}/include/module.i
)

# compiler flags
# add_compile_options("-std=c++14" "-pthread") #  ${GL_CFLAGS})
add_compile_options("-std=c++14") # .. -pthread required only at link time

# [define library source files]
file(GLOB SOURCES src/*.cpp)
# file(GLOB SWIGBASE include/module.i.base)

# [we're compiling a library here..]
add_library(${PROJECT_NAME} SHARED ${SOURCES})
# => now the target_* commands work

# target_link_libraries(${PROJECT_NAME} "-Wl,--no-as-needed") # NOTE: add this one if libValkka dependency is not for some reason included into your shared library

option(valkka_lib "valkka_lib" OFF)
if    (valkka_lib)
  target_link_libraries(${PROJECT_NAME} "-L${valkka_lib}")
endif (valkka_lib)

# [define library header files]
# include_directories(include)
target_include_directories(${PROJECT_NAME} PUBLIC include)

# Nvidia header files
target_include_directories(${PROJECT_NAME} PUBLIC "${CUDA_ROOT}/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${NVCODEC_ROOT}")
target_include_directories(${PROJECT_NAME} PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvDecoder")
target_include_directories(${PROJECT_NAME} PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvEncoder")
target_include_directories(${PROJECT_NAME} PUBLIC "${NVCODEC_ROOT}/Samples/Utils")

# [set shared library version]
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${MAJOR_VERSION})

# [include directories]
target_include_directories(${PROJECT_NAME} PUBLIC ${VALKKA_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${VALKKA_LIBRARIES} ${PYTHON_LIBRARIES})
target_link_libraries(${PROJECT_NAME} "pthread")

# Nvidia libs
target_link_libraries(${PROJECT_NAME} "-L${CUDA_ROOT}/lib64/stubs")
# nvidia "firmware": ..?
target_link_libraries(${PROJECT_NAME} "-L${NVCODEC_ROOT}/Samples/NvCodec/Lib/linux/stubs/x86_64")
target_link_libraries(${PROJECT_NAME} "dl;cuda;nvcuvid")

message("VALKKA  ROOT : ${VALKKA_ROOT}")
message("LIVE555 ROOT : ${LIVE555_ROOT}")
message("FFMPEG  ROOT : ${FFMPEG_ROOT}")
# message("DEMO    ROOT : ${DEMO_ROOT}")

target_include_directories(${PROJECT_NAME} PUBLIC "${LIVE555_ROOT}/liveMedia/include" "${LIVE555_ROOT}/groupsock/include" "${LIVE555_ROOT}/BasicUsageEnvironment/include" "${LIVE555_ROOT}/UsageEnvironment/include")
target_include_directories(${PROJECT_NAME} PUBLIC "${FFMPEG_ROOT}") # ffmpeg header files are referred in the code with the directory name
target_include_directories(${PROJECT_NAME} PUBLIC "${VALKKA_INCLUDE_DIRS}")

## ********************************************************************************
## The statically compiled library we're encapsulating here into our library, might have some dependencies on some elemental libraries
## In the case the static library would depend on, say, x11, glew and alsa, you would use:
# pkg_search_module(X11 REQUIRED x11)
# pkg_search_module(GLEW REQUIRED glew)
# pkg_search_module(ALSA REQUIRED alsa)
## header file location
# target_include_directories(${PROJECT_NAME} PUBLIC ${X11_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
## link time dependencies
# target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} ${GLEW_LIBRARIES} ${ALSA_LIBRARIES})
## *******************************************************************************

## *************************************************************************************************
## define header file location (you have installed this by yourself)
#target_include_directories(${PROJECT_NAME} PUBLIC "${DEMO_ROOT}")
## define where the .a files are to be found (you have compiled this by yourself)
#target_link_libraries(${PROJECT_NAME} "-L${DEMO_ROOT}")
## link time dependencies
#target_link_libraries(${PROJECT_NAME} "rt")
#target_link_libraries(${PROJECT_NAME} "-Wl,--allow-multiple-definition" "-Wl,-Bsymbolic" "-Wl,--start-group" "-Wl,--whole-archive")
#target_link_libraries(${PROJECT_NAME} ":demo.a") # you could have a whole list of .a files here
#target_link_libraries(${PROJECT_NAME} "-Wl,--no-whole-archive" "-Wl,--end-group")
##
## **************************************************************************************************
 
## **************************************************************************************************
## *** An example how to use a system-wide installed shared library ***
## *** Your library will depend on this one dynamically ***
#message("USING SYSTEM PROVIDED SHARED LIBRARY FOR SQLITE3")
## In this demo, our library will depend dynamically on the sqlite3 library.  Install that into your system with: sudo apt-get install libsqlite3-dev
#pkg_search_module(SQLITE REQUIRED sqlite3)
## define header file location
#target_include_directories(${PROJECT_NAME} PUBLIC ${SQLITE_INCLUDE_DIRS})
## link time dependencies
#target_link_libraries(${PROJECT_NAME} ${SQLITE_LIBRARIES})
# **************************************************************************************************

# *** (SWIG, 3) Compile the cpp-wrapped python code ***
add_library(swig_module SHARED ${CMAKE_SOURCE_DIR}/python/valkka/nv/valkka_nv_wrap.cpp)
target_include_directories(swig_module PUBLIC include)
target_include_directories(swig_module PUBLIC ${VALKKA_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
target_include_directories(swig_module PUBLIC "${LIVE555_ROOT}/liveMedia/include" "${LIVE555_ROOT}/groupsock/include" "${LIVE555_ROOT}/BasicUsageEnvironment/include" "${LIVE555_ROOT}/UsageEnvironment/include")
target_include_directories(swig_module PUBLIC "${FFMPEG_ROOT}") # ffmpeg header files are referred in the code with the directory name
target_include_directories(swig_module PUBLIC "${VALKKA_INCLUDE_DIRS}") 

# Nvidia header files
#target_include_directories(swig_module PUBLIC "${CUDA_ROOT}/include")
#target_include_directories(swig_module PUBLIC "${NVCODEC_ROOT}")
#target_include_directories(swig_module PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvDecoder")
#target_include_directories(swig_module PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvEncoder")
#target_include_directories(swig_module PUBLIC "${NVCODEC_ROOT}/Samples/Utils")

# target_include_directories(swig_module PUBLIC "${NUMPY_INCLUDE_DIR}") # ffmpeg header files are referred in the code with the directory name

target_link_libraries(swig_module "valkka_nv.so")
target_link_libraries(swig_module "-L${CMAKE_CURRENT_BINARY_DIR}/lib")

set_target_properties(swig_module PROPERTIES VERSION ${VERSION_STRING} SOVERSION ${MAJOR_VERSION})
set_target_properties(swig_module PROPERTIES PREFIX "")
set_target_properties(swig_module PROPERTIES OUTPUT_NAME "_valkka_nv")
set_target_properties(swig_module PROPERTIES SUFFIX ".so")
set_target_properties(swig_module PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/python/valkka/nv)

add_dependencies(swig_module ${PROJECT_NAME}) # swig .so depends on the main shared library

set(TESTNAMES "mytest" "dectest") # add here the names of your test binaries like this: "mytest1" "mytest2" ..
add_custom_target(tests) # Note: without 'ALL'
foreach( testname ${TESTNAMES} )
  add_executable(${testname} "test/${testname}.cpp") # Note: without 'ALL'
  target_include_directories(${testname} PUBLIC "include")
  target_include_directories(${testname} PUBLIC "${LIVE555_ROOT}/liveMedia/include" "${LIVE555_ROOT}/groupsock/include" "${LIVE555_ROOT}/BasicUsageEnvironment/include" "${LIVE555_ROOT}/UsageEnvironment/include")
  target_include_directories(${testname} PUBLIC "${FFMPEG_ROOT}")
  target_include_directories(${testname} PUBLIC "${VALKKA_INCLUDE_DIRS}")
  target_include_directories(${testname} PUBLIC "${PYTHON_INCLUDE_DIRS}")

  target_include_directories(${testname} PUBLIC "${CUDA_ROOT}/include")
  target_include_directories(${testname} PUBLIC "${NVCODEC_ROOT}")
  target_include_directories(${testname} PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvDecoder")
  target_include_directories(${testname} PUBLIC "${NVCODEC_ROOT}/Samples/NvCodec/NvEncoder")
  target_include_directories(${testname} PUBLIC "${NVCODEC_ROOT}/Samples/Utils")


  target_link_libraries(${testname} "Valkka.so")
  target_link_libraries(${testname} "${PROJECT_NAME}.so")
  target_link_libraries(${testname} "-L${CMAKE_CURRENT_BINARY_DIR}/lib")
  target_link_libraries(${testname} ${PYTHON_LIBRARIES})

  if    (valkka_lib)
    target_link_libraries(${testname} "-L${valkka_lib}")
  endif (valkka_lib)
  
  add_dependencies(tests ${testname}) # tests depends on the executable
  add_dependencies(${testname} ${PROJECT_NAME}) # test depends on libValkka
endforeach( testname ${TESTNAMES} )


# *** packaging ***
# SET(CPACK_SET_DESTDIR "on") # don't use
# SET(CPACK_PACKAGING_INSTALL_PREFIX "/tmp") # don't use

exec_program(dpkg ARGS --print-architecture OUTPUT_VARIABLE MY_ARCH)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "${PROJECT_NAME}")

SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGE_VERSION "${VERSION_STRING}")

SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${MY_ARCH})

# The dependencies: keep these consistent with debian/control
# SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libpython3.5(>= 3.5.2), libgcc1(>= 1:6.0.1), libc6(>= 2.23), libgl1-mesa-glx(>= 12.0.6), libx11-6(>= 2:1.6.3), libstdc++6(>= 5.4.0), libc6(>= 2.23), libglew1.13(>= 1.13.0), python3-numpy") # ubuntu 16
# NEW: Avoid version numbers, use "utils" packages to imply a dependency, for example, to libglew (without hardcoding the version number into the package name)
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "valkka(>=0.6.0)")

#
# objdump -p libValkka.so
# dpkg -S libc.so.6
# => libc6:amd64: /lib/x86_64-linux-gnu/libc.so.6
# apt-cache show libc6 | grep "Version"
# => Version: 2.23-0ubuntu9
# a typical dependency seems to be: libc6 (>= 2.17)

SET(CPACK_PACKAGE_CONTACT "xiaoxoxin@gmail.com")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Valkka NVidia cuda extensions")
SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "")
SET(CPACK_DEBIAN_PACKAGE_SUGGESTS "")
# SET(CPACK_PACKAGE_INSTALL_DIRECTORY "dir") # don't use

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION lib) # install the shared library
#install(DIRECTORY "${CMAKE_SOURCE_DIR}/include" DESTINATION include/valkka FILES_MATCHING PATTERN "*.h") # install header files
# The install command: https://cmake.org/cmake/help/v3.0/command/install.html

# Cmake's INSTALL command is totally cryptic
# what the INSTALL command (maybe) does ..
# .. it takes the last bit of DIRECTORY and puts matched files into DESTINATION/last_bit

# Cmake manual:
# "The last component of each directory name is appended to the destination directory but a trailing slash may be used to avoid this because it leaves the last component empty."
# fair enough! :)

## include header files ..
# install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/" DESTINATION include/valkka_nv FILES_MATCHING PATTERN "*.h")

## an example from Valkka's CMake file, how to include header files of used libraries:
#install(DIRECTORY "${LIVE555_ROOT}/liveMedia/include"               DESTINATION include/valkka/liveMedia             FILES_MATCHING PATTERN "*.h*")
#install(DIRECTORY "${FFMPEG_ROOT}/libavfilter"   DESTINATION include/valkka   FILES_MATCHING PATTERN "*.h")

# TODO: change from absolute to relative dir
install(DIRECTORY "${CMAKE_SOURCE_DIR}/python/valkka"  DESTINATION ${PYTHON_DIR}  
    FILES_MATCHING PATTERN "*.py" 
    PATTERN "nv/__pycache__" EXCLUDE 
)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/python/valkka"  DESTINATION ${PYTHON_DIR}  
    FILES_MATCHING PATTERN "*.so*"
    PATTERN "nv/__pycache__" EXCLUDE
)

# install(DIRECTORY "${FFMPEG_ROOT}" DESTINATION include/valkka   FILES_MATCHING PATTERN "*.h")

# when compiling on my linux box, I use:
# -I/home/sampsa/live555/live/UsageEnvironment/include
# -I/home/sampsa/ffmpeg/ffmpeg_git_lgpl 
#
# with cpp api, should use
# -I/usr/valkka/include/
# -I/usr/valkka/BasicUsageEnvironment/include/
# .. etc. for live555
# -I/usr/valkka/ffmpeg/
#
# however, if combined with "-I/usr/include", there might be other versions of the same header files in "/usr/include/" TODO: think about this..
#

# TODO: how to configure pkg-config when installing this .deb package?
# something like this..?
# https://gitlab.kitware.com/third-party/zlib/commit/ca6e7a0d552e3b54c0833658409e34f9de3bead6

# This must always be last!
INCLUDE(CPack)
