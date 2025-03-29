//
// Created by neuifo on 2025/1/20.
//

#ifndef IMAGE_IMAGE_WEBP_H
#define IMAGE_IMAGE_WEBP_H

#include "config.h"
#ifdef IMAGE_SUPPORT_WEBP

#include <stdio.h>
#include <stdbool.h>

#include "../utils.h"
#include "patch_head_input_stream.h"
#include "src/dec/webpi_dec.h"
#include "src/webp/demux.h"

#define IMAGE_WEBP_DECODER_DESCRIPTION ("libwebp " "521")

//'R', 'I', 'F', 'F'. 文件大小 'W', 'E', 'B', 'P'

#define IMAGE_WEBP_MAGIC_NUMBER_0 0x52
#define IMAGE_WEBP_MAGIC_NUMBER_1 0x49

#define WEBP_FF_ANIMATION_FLAG 0x00000002  // 动画标志位掩码

#define IMAGE_WEBP_PREPARE_UNKNOWN 0x00
#define IMAGE_WEBP_PREPARE_NONE 0x01
#define IMAGE_WEBP_PREPARE_BACKGROUND 0x02
#define IMAGE_WEBP_PREPARE_USE_BACKUP 0x03



typedef struct {
    unsigned char* buffer;
    unsigned int width;
    unsigned int height;
    unsigned int offset_x;
    unsigned int offset_y;
    unsigned int delay; // ms
    unsigned char dop;
    unsigned char bop;
    unsigned char pop;
    bool has_alpha; // 修改为布尔类型
} WEBP_FRAME_INFO;

typedef struct {
    unsigned int width;
    unsigned int height;
    bool is_opaque;
    unsigned char* buffer;
    bool is_animated;
    int buffer_index;
    WEBP_FRAME_INFO* frame_info_array;
    unsigned int frame_count;
    unsigned char* backup;
    bool partially;
    WebPDemuxer* demux;
    WebPData webp_data;
    PatchHeadInputStream* patch_head_input_stream;
} WEBP;


void* WEBP_decode(JNIEnv* env, PatchHeadInputStream* patch_head_input_stream, bool partially);
bool WEBP_complete(JNIEnv* env, WEBP* webp);
bool WEBP_is_completed(WEBP* webp);
int WEBP_get_width(WEBP* webp);
int WEBP_get_height(WEBP* webp);
int WEBP_get_byte_count(WEBP* webp);
void WEBP_render(WEBP* png, int src_x, int src_y,
                 void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
                 int width, int height, bool fill_blank, int default_color);
void WEBP_advance(WEBP* webp);
int WEBP_get_delay(WEBP* webp);
int WEBP_get_frame_count(WEBP* webp);
bool WEBP_is_opaque(WEBP* webp);
void WEBP_recycle(JNIEnv* env, WEBP* webp);



#endif // IMAGE_SUPPORT_webp
#endif //IMAGE_IMAGE_WEBP_H
