//
// Created by neuifo on 2025/1/20.
//

#ifndef IMAGE_IMAGE_WEBP_H
#define IMAGE_IMAGE_WEBP_H

#include "config.h"
#ifdef IMAGE_SUPPORT_WEBP

#include <stdbool.h>

#include "../utils.h"

#define IMAGE_WEBP_DECODER_DESCRIPTION ("libwebp " "521")

//'R', 'I', 'F', 'F'. 文件大小 'W', 'E', 'B', 'P'

#define IMAGE_WEBP_MAGIC_NUMBER_0 0x52
#define IMAGE_WEBP_MAGIC_NUMBER_1 0x49



#define IMAGE_WEBP_PREPARE_UNKNOWN 0x00
#define IMAGE_WEBP_PREPARE_NONE 0x01
#define IMAGE_WEBP_PREPARE_BACKGROUND 0x02
#define IMAGE_WEBP_PREPARE_USE_BACKUP 0x03


#endif // IMAGE_SUPPORT_webp
#endif //IMAGE_IMAGE_WEBP_H
