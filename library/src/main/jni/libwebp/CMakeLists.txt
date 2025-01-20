cmake_minimum_required(VERSION 3.4.1)
project(libwebp C)


set(LIBWEBP_SOURCES
    libwebp/src/dec/alpha_dec.c
    libwebp/src/dec/buffer_dec.c
    libwebp/src/dec/frame_dec.c
    libwebp/src/dec/idec_dec.c
    libwebp/src/dec/io_dec.c
    libwebp/src/dec/quant_dec.c
    libwebp/src/dec/tree_dec.c
    libwebp/src/dec/vp8_dec.c
    libwebp/src/dec/vp8l_dec.c
    libwebp/src/dec/webp_dec.c
    libwebp/src/dsp/cpu.c
    libwebp/src/dsp/dec.c
    libwebp/src/dsp/dec_sse2.c
    libwebp/src/dsp/lossless.c
    libwebp/src/dsp/upsampling.c
    libwebp/src/dsp/upsampling_sse2.c
    libwebp/src/dsp/yuv.c
    libwebp/src/utils/bit_reader_utils.c
    libwebp/src/utils/bit_writer_utils.c
    libwebp/src/utils/color_cache_utils.c
    libwebp/src/utils/filters_utils.c
    libwebp/src/utils/huffman_utils.c
    libwebp/src/utils/huffman_encode_utils.c
    libwebp/src/utils/quant_levels_utils.c
    libwebp/src/utils/quant_levels_dec_utils.c
    libwebp/src/utils/rescaler_utils.c
    libwebp/src/utils/thread_utils.c
    libwebp/src/utils/utils.c
)

enable_language(ASM)

set(LIBWEBP_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/libwebp/src/

)


#message(STATUS "LIBWEBP_SOURCES: ${LIBWEBP_SOURCES}")


add_library(libwebp STATIC ${LIBWEBP_SOURCES})
target_include_directories(libwebp PUBLIC ${LIBWEBP_INCLUDES})