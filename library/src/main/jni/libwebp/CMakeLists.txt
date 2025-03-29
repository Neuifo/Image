cmake_minimum_required(VERSION 3.4.1)
project(libwebp C)


function(parse_Makefile_am FOLDER VAR SRC_REGEX)
    file(READ ${FOLDER}/Makefile.am MAKEFILE_AM)
    string(REGEX MATCHALL "${SRC_REGEX}_SOURCES[ ]*\\+?=[ ]+[0-9a-z\\._ ]*"
            FILES_PER_LINE ${MAKEFILE_AM})
    set(SRCS ${${VAR}})
    foreach(FILES ${FILES_PER_LINE})
        string(FIND ${FILES} "=" OFFSET)
        math(EXPR OFFSET "${OFFSET} + 2")
        string(SUBSTRING ${FILES} ${OFFSET} -1 FILES)
        if(FILES)
            string(REGEX MATCHALL "[0-9a-z\\._]+" FILES ${FILES})
            foreach(FILE ${FILES})
                list(APPEND SRCS ${FOLDER}/${FILE})
            endforeach()
        endif()
    endforeach()
    set(${VAR} ${SRCS} PARENT_SCOPE)
endfunction()

set(WEBP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src)
parse_makefile_am(${WEBP_SRC_DIR}/dec "WEBP_DEC_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/demux "WEBP_DEMUX_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_COMMON_SRCS" "COMMON")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_ENC_SRCS" "ENC")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_ENC_SRCS" "dsp_[^ ]*")
parse_makefile_am(${WEBP_SRC_DIR}/dsp "WEBP_DSP_DEC_SRCS" "decode_[^ ]*")
parse_makefile_am(${WEBP_SRC_DIR}/enc "WEBP_ENC_SRCS" "")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_COMMON_SRCS" "COMMON")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_ENC_SRCS" "ENC")
parse_makefile_am(${WEBP_SRC_DIR}/utils "WEBP_UTILS_DEC_SRCS" "decode_[^ ]*")


set(LIBWEBP_SOURCES
        ${WEBP_DEC_SRCS}
        ${WEBP_DEMUX_SRCS}
        ${WEBP_DSP_COMMON_SRCS}
        ${WEBP_DSP_ENC_SRCS}
        ${WEBP_DSP_DEC_SRCS}
        ${WEBP_ENC_SRCS}
        ${WEBP_UTILS_COMMON_SRCS}
        ${WEBP_UTILS_ENC_SRCS}
        ${WEBP_UTILS_DEC_SRCS}
)

enable_language(ASM)

set(LIBWEBP_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/libwebp
        ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src  # 添加源码目录
        ${ANDROID_NDK}/sources/android/cpufeatures
)

#arm64-v8a
add_definitions(-DWEBP_USE_NEON=1)

# Android set
include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
add_library(cpufeatures-webp STATIC
        ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
list(APPEND INSTALLED_LIBRARIES cpufeatures-webp)
target_link_libraries(cpufeatures-webp dl)
set(SHARPYUV_DEP_LIBRARIES ${SHARPYUV_DEP_LIBRARIES} cpufeatures-webp)
set(WEBP_DEP_LIBRARIES ${WEBP_DEP_LIBRARIES} cpufeatures-webp)
set(cpufeatures_include_dir ${ANDROID_NDK}/sources/android/cpufeatures)
set(SHARPYUV_DEP_INCLUDE_DIRS ${SHARPYUV_DEP_INCLUDE_DIRS}
        ${cpufeatures_include_dir})
set(WEBP_DEP_INCLUDE_DIRS ${WEBP_DEP_INCLUDE_DIRS} ${cpufeatures_include_dir})
add_definitions(-DHAVE_CPU_FEATURES_H=1)
set(HAVE_CPU_FEATURES_H 1)


add_library(libwebp STATIC ${LIBWEBP_SOURCES})
target_link_libraries(libwebp PRIVATE ${WEBP_DEP_LIBRARIES} z)
target_include_directories(libwebp PUBLIC ${LIBWEBP_INCLUDES})