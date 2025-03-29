//
// Created by neuifo on 2025/1/20.
//
#include "config.h"

#ifdef IMAGE_SUPPORT_WEBP

#include <stdlib.h>

#include "../log.h"
#include "../utils.h"
#include "image_webp.h"
#include "image_utils.h"
#include "java_wrapper.h"


static void free_frame_info_array(WEBP_FRAME_INFO* frame_info_array, unsigned int count) {
    int i;

    if (frame_info_array == NULL) {
        return;
    }

    for (i = 0; i < count; i++) {
        WEBP_FRAME_INFO* frame_info = frame_info_array + i;
        free(frame_info->buffer);
        frame_info->buffer = NULL;
    }
    free(frame_info_array);
}

void* WEBP_decode(JNIEnv* env, PatchHeadInputStream* patch_head_input_stream, bool partially) {
    WEBP* webp = NULL;
    WebPDemuxer* demux = NULL;
    WebPData webp_data;
    bool is_animated;
    unsigned int width;
    unsigned int height;
    unsigned int frame_count = 0;
    WEBP_FRAME_INFO* frame_info_array = NULL;
    int i;

    // Read WebP data from the input stream
    // Read WebP data from the input stream
    size_t buffer_size = 1024 * 1024; // 1MB 缓冲区
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    if (buffer == NULL) {
        WTF_OM;
        free(buffer);
        return NULL;
    }

    size_t bytes_read = read_patch_head_input_stream(env, patch_head_input_stream, buffer, 0, buffer_size);
    if (bytes_read == 0) {
        LOGE(MSG("Failed to read WebP data"));
        free(buffer);
        return NULL;
    }

    webp_data.bytes = buffer;
    webp_data.size = bytes_read;

    if (webp_data.bytes == NULL || webp_data.size == 0) {
        LOGE(MSG("Failed to read WebP data"));
        return NULL;
    }

    // Create WebP demuxer
    demux = WebPDemux(&webp_data);
    if (demux == NULL) {
        LOGE(MSG("Failed to create WebP demuxer"));
        free(webp_data.bytes);
        return NULL;
    }

    // Check if WebP is animated
    uint32_t flags = WebPDemuxGetI(demux, WEBP_FF_FORMAT_FLAGS);
    is_animated = (flags & WEBP_FF_ANIMATION_FLAG) != 0;
//    is_animated = false;

    // Get WebP dimensions
    width = WebPDemuxGetI(demux, WEBP_FF_CANVAS_WIDTH);
    height = WebPDemuxGetI(demux, WEBP_FF_CANVAS_HEIGHT);

    // Create WEBP structure
    webp = (WEBP*) malloc(sizeof(WEBP));
    if (webp == NULL) {
        WTF_OM;
        WebPDemuxDelete(demux);
        free(webp_data.bytes);
        return NULL;
    }

    // Initialize WEBP structure
    webp->width = width;
    webp->height = height;
    webp->is_opaque = true; // Assume opaque for simplicity
    webp->buffer = NULL;
    webp->is_animated = is_animated;
    webp->buffer_index = -1;
    webp->frame_info_array = NULL;
    webp->frame_count = 0;
    webp->backup = NULL;
    webp->partially = partially;
    webp->demux = demux;
    webp->webp_data = webp_data;
    webp->patch_head_input_stream = patch_head_input_stream;

    if (is_animated) {
        // Get frame count
        frame_count = WebPDemuxGetI(demux, WEBP_FF_FRAME_COUNT);

        // Create frame info array
        frame_info_array = (WEBP_FRAME_INFO*) calloc(frame_count, sizeof(WEBP_FRAME_INFO));
        if (frame_info_array == NULL) {
            WTF_OM;
            WebPDemuxDelete(demux);
            free(webp_data.bytes);
            free(webp);
            return NULL;
        }

        // Read frames
        WebPIterator iter;
        if (WebPDemuxGetFrame(demux, 1, &iter)) {
            for (i = 0; i < frame_count; i++) {
                WEBP_FRAME_INFO* frame_info = frame_info_array + i;
                frame_info->width = iter.width;
                frame_info->height = iter.height;
                frame_info->offset_x = iter.x_offset;
                frame_info->offset_y = iter.y_offset;
                frame_info->delay = iter.duration;
                frame_info->dop = iter.dispose_method;
                frame_info->bop = iter.blend_method;
                frame_info->pop = 0; // Not used in WebP

                // Decode frame
                frame_info->buffer = WebPDecodeRGBA(iter.fragment.bytes, iter.fragment.size, &frame_info->width, &frame_info->height);
                if (frame_info->buffer == NULL) {
                    LOGE(MSG("Failed to decode WebP frame"));
                    free_frame_info_array(frame_info_array, frame_count);
                    WebPDemuxDelete(demux);
                    free(webp_data.bytes);
                    free(webp);
                    return NULL;
                }

                WebPBitstreamFeatures features;
                VP8StatusCode status = WebPGetFeatures(webp_data.bytes, webp_data.size, &features);
                if (status != VP8_STATUS_OK) {
                    LOGE(MSG("Failed to get WebP features"));
                    free_frame_info_array(frame_info_array, frame_count);
                    WebPDemuxDelete(demux);
                    free(webp_data.bytes);
                    free(webp);
                    return NULL;
                }

                // 现在可以安全地使用 features.has_alpha
                frame_info->has_alpha = features.has_alpha;

                WebPDemuxNextFrame(&iter);
            }
        }

        webp->frame_info_array = frame_info_array;
        webp->frame_count = frame_count;
    } else {
        // Decode single frame
        webp->buffer = WebPDecodeRGBA(webp_data.bytes, webp_data.size, &width, &height);
        if (webp->buffer == NULL) {
            LOGE(MSG("Failed to decode WebP image"));
            WebPDemuxDelete(demux);
            free(webp_data.bytes);
            free(webp);
            return NULL;
        }

        webp->buffer_index = 0;
    }

    return webp;
}

void WEBP_render(WEBP* webp, int src_x, int src_y,
                 void* dst, int dst_w, int dst_h, int dst_x, int dst_y,
                 int width, int height, bool fill_blank, int default_color) {
    copy_pixels(webp->buffer, webp->width, webp->height, src_x, src_y,
                dst, dst_w, dst_h, dst_x, dst_y,
                width, height, fill_blank, default_color);
}



int WEBP_get_frame_count(WEBP* webp) {
    if (webp->is_animated) {
        return webp->frame_count;
    }
    return 1;
}

int WEBP_get_byte_count(WEBP* webp) {
    if (webp == NULL) {
        return 0;
    }

    if (!webp->is_animated) {
        // 单帧 WebP，直接返回缓冲区大小
        return webp->buffer ? (webp->width * webp->height * 4) : 0;
    } else {
        // 动画 WebP，累加所有帧的缓冲区大小
        int total = 0;
        for (unsigned int i = 0; i < webp->frame_count; i++) {
            WEBP_FRAME_INFO* frame = &webp->frame_info_array[i];
            total += frame->width * frame->height * 4; // 假设每个像素 4 个字节（RGBA）
        }
        return total;
    }
}

void WEBP_advance(WEBP* webp) {
    if (webp == NULL || !webp->is_animated) {
        return;
    }

    // 释放当前帧备份缓冲区
    if (webp->backup) {
        free(webp->backup);
        webp->backup = NULL;
    }

    // 切换到下一帧
    webp->buffer_index++;
    if (webp->buffer_index >= webp->frame_count) {
        // 到达末尾，可选择停止或从头开始
        webp->buffer_index = 0; // 从头开始
    }

    // 备份当前帧数据以便恢复
    if (webp->buffer) {
        size_t bufferSize = webp->width * webp->height * 4;
        webp->backup = (unsigned char*)malloc(bufferSize);
        if (webp->backup) {
            memcpy(webp->backup, webp->buffer, bufferSize);
        }
    }
}

int WEBP_get_delay(WEBP* webp) {
    if (webp->is_animated) {
        int current_frame = webp->buffer_index;
        if (current_frame >= 0 && current_frame < webp->frame_count) {
            return webp->frame_info_array[current_frame].delay;
        }
    }
    return 0;
}

bool WEBP_is_opaque(WEBP* webp) {
    if (webp->is_animated) {
        for (unsigned int i = 0; i < webp->frame_count; i++) {
            if (webp->frame_info_array[i].has_alpha) {
                return false;
            }
        }
        return true;
    } else {
        return webp->is_opaque;
    }
}

void WEBP_recycle(JNIEnv* env, WEBP* webp) {
    if (webp == NULL) {
        return;
    }

    free(webp->buffer);
    webp->buffer = NULL;

    free_frame_info_array(webp->frame_info_array, webp->frame_count);
    webp->frame_info_array = NULL;

    free(webp->backup);
    webp->backup = NULL;

    if (webp->demux != NULL) {
        WebPDemuxDelete(webp->demux);
    }
    webp->demux = NULL;

    if (webp->patch_head_input_stream != NULL) {
        close_patch_head_input_stream(env, webp->patch_head_input_stream);
        destroy_patch_head_input_stream(env, &webp->patch_head_input_stream);
        webp->patch_head_input_stream = NULL;
    }
}

bool WEBP_complete(JNIEnv* env, WEBP* webp) {
    if (webp == NULL || !webp->partially) {
        return true; // 不需要处理或已完整
    }

    // 如果已完整，直接返回成功
    if (WEBP_is_completed(webp)) {
        return true;
    }

    // 重新读取完整数据
    free(webp->webp_data.bytes);
    size_t buffer_size = 1024 * 1024; // 1MB 缓冲区
    unsigned char* buffer = (unsigned char*)malloc(buffer_size);
    if (buffer == NULL) {
        WTF_OM;
        return false;
    }

    size_t bytes_read = read_patch_head_input_stream(env, webp->patch_head_input_stream, buffer, 0, buffer_size);
    if (bytes_read == 0) {
        LOGE(MSG("Failed to read complete WebP data"));
        free(buffer);
        return false;
    }

    webp->webp_data.bytes = buffer;
    webp->webp_data.size = bytes_read;
    webp->partially = false; // 标记为已完成

    // 如果是动画，需要重新解析帧数据
    if (webp->is_animated) {
        WebPDemuxDelete(webp->demux);
        webp->demux = WebPDemux(&webp->webp_data);
        if (webp->demux == NULL) {
            LOGE(MSG("Failed to re-create WebP demuxer"));
            free(webp->webp_data.bytes);
            return false;
        }
    }

    return true;
}

bool WEBP_is_completed(WEBP* webp) {
    return (webp != NULL) && !webp->partially;
}

int WEBP_get_width(WEBP* webp) {
    return (webp != NULL) ? (int)webp->width : 0;
}

int WEBP_get_height(WEBP* webp) {
    return (webp != NULL) ? (int)webp->height : 0;
}

#endif // IMAGE_SUPPORT_WEBP