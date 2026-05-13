/* licensed under GPL v3.0 see https://github.com/ZDev22/ZEngine/blob/main/LICENSE for current license

This file is designed to take the weight of compiling all dependencies away from main.c
To use along-side zengine.h, use #define ZENGINE_DEPS_DEFINED */

/* RGFW */
#define RGFW_IMPLEMENTATION
#define RGFW_VULKAN
#define RGFW_EXPORT
#define RGFW_ASSERT(x) (void)(x)

/* STB_IMAGE */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT

/* STB_IMAGE_WRITE */
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_ASSERT

/* STB_TRUETYPE */
#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_ASSERT

/* DEPENDENCIES */
#include "deps/RGFW.h"
#include "deps/stb_image.h"
#include "deps/stb_image_write.h"
#include "deps/stb_truetype.h"

#undef RGFW_IMPLEMENTATION
#undef ZCOLLIDE_IMPLEMENTATION
#undef ZTEXT_IMPLEMENTATION
#undef STB_IMAGE_IMPLEMENTATION
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STB_TRUETYPE_IMPLEMENTATION

/* ZDEPS */
#define ZCOLLIDE_IMPLEMENTATION
#define ZTEXT_IMPLEMENTATION
#include "zcollide.h"
#include "ztext.h"

