project(native_lib C CXX)
cmake_minimum_required(VERSION 3.3.2)

message("\n-------Building android application--------\n")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(SOURCES  "src/util.cpp" "src/main.cpp" "src/lib.cpp")

SET(NDK_VERSION 23.1.7779620)

set(ANDROID_NDK /home/alex/Android/Sdk/ndk/${NDK_VERSION})

# build & link
include_directories (${CMAKE_BINARY_DIR})
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${ANDROID_NDK}/sources/android)

string(CONCAT COMPILER_FLAGS " -O3 ")
set (CMAKE_CXX_FLAGS ${COMPILER_FLAGS})

add_library(${CMAKE_PROJECT_NAME} SHARED ${SOURCES}) 

set(DAWN_BUILD_EXAMPLES OFF)
add_subdirectory(dawn)
set(DAWN_LIBRARIES  
    dawn_internal_config
    dawncpp
    dawn_proc
    dawn_common
    dawn_utils
    dawn_native
)

find_library(log-lib log)

add_library(app-glue
             STATIC
             ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)


# https://github.com/gongminmin/android_native_app_glue/commit/bd129b034fe6f8cc09f90c971dfe33df1593c55e
target_link_libraries(${PROJECT_NAME} app-glue "-u ANativeActivity_onCreate")
target_link_libraries(${PROJECT_NAME} android ${log-lib})
target_link_libraries(${PROJECT_NAME} ${DAWN_LIBRARIES})

set_target_properties(${CMAKE_PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)