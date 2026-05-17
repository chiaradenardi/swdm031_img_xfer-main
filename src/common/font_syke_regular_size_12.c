/**
 * @file      font_syke_regular_size_12.c
 *
 * @brief     The size 12 of font Syke Regular
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2026. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************************
 * Size: 12 px
 * Bpp: 1
 * Opts: --bpp 1 --size 12 --no-compress --stride 1 --align 1 --font Syke-Regular-2.otf --range 32-127 --format lvgl -o
 *font_syke_regular_size_12.c
 ******************************************************************************/

#ifdef __has_include
#if __has_include( "lvgl.h" )
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef FONT_SYKE_REGULAR_SIZE_12
#define FONT_SYKE_REGULAR_SIZE_12 1
#endif

#if FONT_SYKE_REGULAR_SIZE_12

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0xfd,

    /* U+0022 "\"" */
    0xb6, 0x80,

    /* U+0023 "#" */
    0x12, 0x12, 0x7f, 0x24, 0x24, 0x7e, 0x24, 0x2c,

    /* U+0024 "$" */
    0x21, 0xe8, 0x20, 0xc1, 0xe0, 0xc1, 0x7, 0xe2, 0x0,

    /* U+0025 "%" */
    0x63, 0x12, 0x42, 0x50, 0x4a, 0xe6, 0xa2, 0x14, 0x44, 0x88, 0x8e,

    /* U+0026 "&" */
    0x70, 0x91, 0x23, 0x87, 0x13, 0x63, 0xbf,

    /* U+0027 "'" */
    0xe0,

    /* U+0028 "(" */
    0x29, 0x49, 0x24, 0xc9, 0x80,

    /* U+0029 ")" */
    0x89, 0x12, 0x49, 0x6a, 0x0,

    /* U+002A "*" */
    0x25, 0x5c, 0xa0,

    /* U+002B "+" */
    0x10, 0x23, 0xf8, 0x81, 0x2, 0x0,

    /* U+002C "," */
    0xe0,

    /* U+002D "-" */
    0xf0,

    /* U+002E "." */
    0x80,

    /* U+002F "/" */
    0x10, 0x84, 0x42, 0x21, 0x18,

    /* U+0030 "0" */
    0x7b, 0x38, 0x61, 0x86, 0x1c, 0xde,

    /* U+0031 "1" */
    0x7e, 0x92, 0x49,

    /* U+0032 "2" */
    0xf0, 0x42, 0x37, 0x42, 0x1f,

    /* U+0033 "3" */
    0xf0, 0x42, 0xf0, 0x84, 0x3e,

    /* U+0034 "4" */
    0x8, 0x30, 0xa3, 0x4c, 0x9f, 0xc2, 0x4,

    /* U+0035 "5" */
    0x7e, 0x31, 0xe0, 0x84, 0x3e,

    /* U+0036 "6" */
    0x7b, 0x8, 0x3e, 0x86, 0x18, 0x5e,

    /* U+0037 "7" */
    0xfc, 0x30, 0x86, 0x10, 0xc2, 0x18,

    /* U+0038 "8" */
    0x7a, 0x18, 0x5f, 0x86, 0x18, 0x5e,

    /* U+0039 "9" */
    0x7a, 0x18, 0x61, 0x7c, 0x10, 0xde,

    /* U+003A ":" */
    0x84,

    /* U+003B ";" */
    0x87,

    /* U+003C "<" */
    0x2, 0x1d, 0xc4, 0x7, 0x1, 0x80, 0x80,

    /* U+003D "=" */
    0xfe, 0x0, 0x7, 0xf0,

    /* U+003E ">" */
    0x1, 0xc0, 0x60, 0x31, 0x9c, 0x0, 0x0,

    /* U+003F "?" */
    0xf0, 0x42, 0x36, 0x20, 0x8,

    /* U+0040 "@" */
    0x1e, 0x18, 0x64, 0xea, 0x49, 0x92, 0x64, 0x99, 0x27, 0x3e, 0x60, 0xf, 0x80,

    /* U+0041 "A" */
    0x18, 0x70, 0xa1, 0x26, 0x4f, 0x90, 0xe1,

    /* U+0042 "B" */
    0xf2, 0x28, 0xbc, 0x8a, 0x18, 0x7e,

    /* U+0043 "C" */
    0x7f, 0x8, 0x20, 0x82, 0xc, 0x1f,

    /* U+0044 "D" */
    0xfa, 0x38, 0x61, 0x86, 0x18, 0xfe,

    /* U+0045 "E" */
    0xfc, 0x21, 0xe8, 0x42, 0x1f,

    /* U+0046 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x10,

    /* U+0047 "G" */
    0x7f, 0x8, 0x20, 0x9e, 0x1c, 0x5f,

    /* U+0048 "H" */
    0x86, 0x18, 0x7f, 0x86, 0x18, 0x61,

    /* U+0049 "I" */
    0xff,

    /* U+004A "J" */
    0x11, 0x11, 0x11, 0x1e,

    /* U+004B "K" */
    0x8e, 0x6b, 0x38, 0xe2, 0x49, 0xa3,

    /* U+004C "L" */
    0x84, 0x21, 0x8, 0x42, 0x1f,

    /* U+004D "M" */
    0xc7, 0x8f, 0xbd, 0x5a, 0xb2, 0x60, 0xc1,

    /* U+004E "N" */
    0xc7, 0x1a, 0x69, 0x96, 0x58, 0xe3,

    /* U+004F "O" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x71, 0xbe,

    /* U+0050 "P" */
    0xfa, 0x18, 0x61, 0xfa, 0x8, 0x20,

    /* U+0051 "Q" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x71, 0xbe, 0x10, 0x1c,

    /* U+0052 "R" */
    0xfa, 0x18, 0x61, 0xfa, 0x68, 0xe1,

    /* U+0053 "S" */
    0x7e, 0x8, 0x18, 0x1c, 0x10, 0x7e,

    /* U+0054 "T" */
    0xfe, 0x20, 0x40, 0x81, 0x2, 0x4, 0x8,

    /* U+0055 "U" */
    0x86, 0x18, 0x61, 0x86, 0x18, 0x5e,

    /* U+0056 "V" */
    0xc2, 0x89, 0x12, 0x22, 0xc5, 0xa, 0xc,

    /* U+0057 "W" */
    0xcc, 0x53, 0x14, 0xcd, 0x2a, 0x7a, 0x8c, 0xa3, 0x38, 0xc4,

    /* U+0058 "X" */
    0x46, 0xc8, 0xa0, 0xc3, 0x85, 0x11, 0x63,

    /* U+0059 "Y" */
    0x44, 0x88, 0xa1, 0x41, 0x2, 0x4, 0x8,

    /* U+005A "Z" */
    0xfc, 0x31, 0x84, 0x21, 0x8c, 0x3f,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x93, 0x80,

    /* U+005C "\\" */
    0xc2, 0x10, 0x42, 0x8, 0x42,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x27, 0x80,

    /* U+005E "^" */
    0x20, 0xc5, 0x12, 0x8c,

    /* U+005F "_" */
    0xfc,

    /* U+0060 "`" */
    0x44,

    /* U+0061 "a" */
    0xf8, 0x47, 0xd8, 0xfc,

    /* U+0062 "b" */
    0x84, 0x3d, 0x18, 0xc6, 0x3e,

    /* U+0063 "c" */
    0x7c, 0x21, 0x8, 0x3c,

    /* U+0064 "d" */
    0x8, 0x5f, 0x18, 0xc6, 0x2f,

    /* U+0065 "e" */
    0x74, 0x7f, 0x8, 0x3c,

    /* U+0066 "f" */
    0x74, 0xf4, 0x44, 0x44,

    /* U+0067 "g" */
    0x7e, 0x28, 0x9c, 0x83, 0xf8, 0x7e,

    /* U+0068 "h" */
    0x84, 0x2f, 0x98, 0xc6, 0x31,

    /* U+0069 "i" */
    0xbf,

    /* U+006A "j" */
    0x20, 0x92, 0x49, 0x3c,

    /* U+006B "k" */
    0x84, 0x25, 0x4c, 0x72, 0x53,

    /* U+006C "l" */
    0xff,

    /* U+006D "m" */
    0xbb, 0xe6, 0x62, 0x31, 0x18, 0x8c, 0x44,

    /* U+006E "n" */
    0xbe, 0x63, 0x18, 0xc4,

    /* U+006F "o" */
    0x74, 0x63, 0x18, 0xb8,

    /* U+0070 "p" */
    0xf4, 0x63, 0x18, 0xfa, 0x10,

    /* U+0071 "q" */
    0x7c, 0x63, 0x18, 0xbc, 0x21,

    /* U+0072 "r" */
    0xac, 0x88, 0x88,

    /* U+0073 "s" */
    0xf8, 0xc3, 0x1e,

    /* U+0074 "t" */
    0x44, 0xf4, 0x44, 0x47,

    /* U+0075 "u" */
    0x8c, 0x63, 0x19, 0xf4,

    /* U+0076 "v" */
    0xca, 0x52, 0xa3, 0x18,

    /* U+0077 "w" */
    0xc9, 0x59, 0x55, 0x55, 0x66, 0x22,

    /* U+0078 "x" */
    0x49, 0xa3, 0xc, 0x69, 0x20,

    /* U+0079 "y" */
    0x45, 0x22, 0x8e, 0x30, 0x42, 0x38,

    /* U+007A "z" */
    0xf8, 0x88, 0x84, 0x7c,

    /* U+007B "{" */
    0x74, 0x44, 0x4c, 0x44, 0x44, 0x70,

    /* U+007C "|" */
    0xff, 0xe0,

    /* U+007D "}" */
    0xe2, 0x22, 0x23, 0x22, 0x22, 0xe0,

    /* U+007E "~" */
    0x66, 0x60
};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    { .bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0 } /* id = 0 reserved */,
    { .bitmap_index = 0, .adv_w = 48, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 1, .adv_w = 53, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 2, .adv_w = 81, .box_w = 3, .box_h = 3, .ofs_x = 1, .ofs_y = 5 },
    { .bitmap_index = 4, .adv_w = 137, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 12, .adv_w = 115, .box_w = 6, .box_h = 11, .ofs_x = 1, .ofs_y = -1 },
    { .bitmap_index = 21, .adv_w = 188, .box_w = 11, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 32, .adv_w = 137, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 39, .adv_w = 45, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = 5 },
    { .bitmap_index = 40, .adv_w = 72, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 45, .adv_w = 72, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 50, .adv_w = 93, .box_w = 5, .box_h = 4, .ofs_x = 1, .ofs_y = 4 },
    { .bitmap_index = 53, .adv_w = 121, .box_w = 7, .box_h = 6, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 59, .adv_w = 47, .box_w = 1, .box_h = 3, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 60, .adv_w = 76, .box_w = 4, .box_h = 1, .ofs_x = 1, .ofs_y = 3 },
    { .bitmap_index = 61, .adv_w = 49, .box_w = 1, .box_h = 1, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 62, .adv_w = 80, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 67, .adv_w = 123, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 73, .adv_w = 70, .box_w = 3, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 76, .adv_w = 101, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 81, .adv_w = 105, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 86, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 93, .adv_w = 104, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 98, .adv_w = 116, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 104, .adv_w = 102, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 110, .adv_w = 118, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 116, .adv_w = 116, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 122, .adv_w = 49, .box_w = 1, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 123, .adv_w = 47, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 124, .adv_w = 121, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 131, .adv_w = 121, .box_w = 7, .box_h = 4, .ofs_x = 0, .ofs_y = 1 },
    { .bitmap_index = 135, .adv_w = 121, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 142, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 147, .adv_w = 190, .box_w = 10, .box_h = 10, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 160, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 167, .adv_w = 122, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 173, .adv_w = 115, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 179, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 185, .adv_w = 109, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 190, .adv_w = 106, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 195, .adv_w = 127, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 201, .adv_w = 131, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 207, .adv_w = 51, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 208, .adv_w = 79, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 212, .adv_w = 123, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 218, .adv_w = 96, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 223, .adv_w = 153, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 230, .adv_w = 133, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 236, .adv_w = 136, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 243, .adv_w = 117, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 249, .adv_w = 136, .box_w = 7, .box_h = 10, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 258, .adv_w = 119, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 264, .adv_w = 111, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 270, .adv_w = 119, .box_w = 7, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 277, .adv_w = 131, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 283, .adv_w = 114, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 290, .adv_w = 166, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 300, .adv_w = 117, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 307, .adv_w = 109, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 314, .adv_w = 110, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 320, .adv_w = 79, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 325, .adv_w = 80, .box_w = 5, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 330, .adv_w = 79, .box_w = 3, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 335, .adv_w = 121, .box_w = 6, .box_h = 5, .ofs_x = 1, .ofs_y = 3 },
    { .bitmap_index = 339, .adv_w = 108, .box_w = 6, .box_h = 1, .ofs_x = 0, .ofs_y = -2 },
    { .bitmap_index = 340, .adv_w = 55, .box_w = 3, .box_h = 2, .ofs_x = 0, .ofs_y = 7 },
    { .bitmap_index = 341, .adv_w = 99, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 345, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 350, .adv_w = 92, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 354, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 359, .adv_w = 99, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 363, .adv_w = 71, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 367, .adv_w = 113, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 373, .adv_w = 105, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 378, .adv_w = 44, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 379, .adv_w = 44, .box_w = 3, .box_h = 10, .ofs_x = -1, .ofs_y = -2 },
    { .bitmap_index = 383, .adv_w = 94, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 388, .adv_w = 44, .box_w = 1, .box_h = 8, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 389, .adv_w = 156, .box_w = 9, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 396, .adv_w = 104, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 400, .adv_w = 102, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 404, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 409, .adv_w = 103, .box_w = 5, .box_h = 8, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 414, .adv_w = 73, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 417, .adv_w = 90, .box_w = 4, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 420, .adv_w = 71, .box_w = 4, .box_h = 8, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 424, .adv_w = 103, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 428, .adv_w = 92, .box_w = 5, .box_h = 6, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 432, .adv_w = 139, .box_w = 8, .box_h = 6, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 438, .adv_w = 99, .box_w = 6, .box_h = 6, .ofs_x = 0, .ofs_y = 0 },
    { .bitmap_index = 443, .adv_w = 99, .box_w = 6, .box_h = 8, .ofs_x = 0, .ofs_y = -2 },
    { .bitmap_index = 449, .adv_w = 90, .box_w = 5, .box_h = 6, .ofs_x = 1, .ofs_y = 0 },
    { .bitmap_index = 453, .adv_w = 90, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 459, .adv_w = 51, .box_w = 1, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 461, .adv_w = 90, .box_w = 4, .box_h = 11, .ofs_x = 1, .ofs_y = -2 },
    { .bitmap_index = 467, .adv_w = 121, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 2 }
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = { { .range_start       = 32,
                                                  .range_length      = 95,
                                                  .glyph_id_start    = 1,
                                                  .unicode_list      = NULL,
                                                  .glyph_id_ofs_list = NULL,
                                                  .list_length       = 0,
                                                  .type              = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY } };

/*-----------------
 *    KERNING
 *----------------*/

/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] = {
    2,  66, 2,  69, 2,  71, 2,  72, 2,  74, 2,  75, 2,  82, 2,  85, 2,  86, 2,  87, 2,  88, 2,  90, 9,  43, 9,  75, 13,
    10, 14, 53, 15, 10, 16, 34, 16, 36, 16, 40, 16, 48, 16, 50, 16, 66, 17, 13, 17, 15, 17, 18, 17, 20, 17, 22, 17, 24,
    19, 24, 20, 20, 20, 24, 21, 19, 21, 20, 21, 21, 21, 22, 22, 19, 22, 21, 23, 18, 23, 20, 23, 22, 23, 24, 23, 26, 24,
    13, 24, 15, 24, 17, 24, 19, 24, 21, 24, 27, 24, 28, 25, 18, 25, 20, 25, 24, 25, 26, 26, 13, 26, 15, 26, 20, 26, 24,
    32, 66, 32, 68, 32, 69, 32, 70, 32, 72, 32, 75, 32, 80, 32, 82, 32, 84, 34, 36, 34, 40, 34, 48, 34, 50, 34, 53, 34,
    54, 34, 55, 34, 56, 34, 58, 34, 66, 34, 71, 34, 74, 34, 75, 34, 85, 34, 86, 34, 87, 34, 88, 34, 90, 35, 34, 35, 36,
    35, 40, 35, 43, 35, 48, 35, 50, 35, 52, 35, 53, 35, 55, 35, 56, 35, 57, 35, 58, 35, 59, 35, 66, 35, 71, 35, 72, 35,
    74, 35, 75, 35, 84, 35, 85, 35, 87, 35, 88, 35, 89, 35, 90, 35, 91, 36, 36, 36, 40, 36, 48, 36, 50, 36, 66, 36, 68,
    36, 69, 36, 70, 36, 71, 36, 72, 36, 74, 36, 75, 36, 80, 36, 82, 36, 85, 36, 86, 36, 87, 36, 88, 36, 90, 37, 34, 37,
    36, 37, 40, 37, 43, 37, 46, 37, 48, 37, 50, 37, 52, 37, 53, 37, 54, 37, 55, 37, 56, 37, 57, 37, 58, 37, 59, 37, 66,
    37, 67, 37, 71, 37, 72, 37, 73, 37, 74, 37, 75, 37, 76, 37, 77, 37, 78, 37, 79, 37, 81, 37, 83, 37, 84, 37, 85, 37,
    86, 37, 87, 37, 88, 37, 89, 37, 90, 37, 91, 38, 36, 38, 40, 38, 48, 38, 50, 38, 69, 38, 71, 38, 75, 38, 85, 38, 87,
    38, 88, 38, 90, 39, 13, 39, 15, 39, 16, 39, 34, 39, 36, 39, 40, 39, 43, 39, 48, 39, 50, 39, 52, 39, 66, 39, 68, 39,
    69, 39, 70, 39, 71, 39, 72, 39, 75, 39, 78, 39, 79, 39, 81, 39, 82, 39, 83, 39, 84, 39, 85, 39, 86, 39, 87, 39, 88,
    39, 89, 39, 90, 39, 91, 40, 34, 40, 36, 40, 40, 40, 43, 40, 48, 40, 50, 40, 52, 40, 59, 40, 66, 40, 71, 40, 72, 40,
    74, 40, 75, 40, 78, 40, 79, 40, 81, 40, 83, 40, 84, 40, 85, 40, 86, 40, 87, 40, 88, 40, 89, 40, 90, 40, 91, 41, 72,
    41, 75, 41, 90, 42, 72, 42, 75, 42, 90, 43, 34, 43, 43, 43, 52, 43, 53, 43, 55, 43, 56, 43, 57, 43, 58, 43, 59, 43,
    66, 43, 71, 43, 72, 43, 74, 43, 75, 43, 84, 43, 85, 43, 87, 43, 88, 43, 89, 43, 90, 43, 91, 44, 36, 44, 40, 44, 48,
    44, 50, 44, 66, 44, 68, 44, 69, 44, 71, 44, 72, 44, 74, 44, 75, 44, 82, 44, 85, 44, 86, 44, 87, 44, 88, 44, 90, 45,
    36, 45, 40, 45, 48, 45, 50, 45, 53, 45, 54, 45, 55, 45, 56, 45, 58, 45, 68, 45, 69, 45, 70, 45, 71, 45, 74, 45, 75,
    45, 82, 45, 85, 45, 87, 45, 88, 45, 90, 46, 75, 47, 72, 47, 75, 47, 90, 48, 34, 48, 36, 48, 40, 48, 43, 48, 46, 48,
    48, 48, 50, 48, 52, 48, 53, 48, 54, 48, 55, 48, 56, 48, 57, 48, 58, 48, 59, 48, 66, 48, 67, 48, 71, 48, 72, 48, 73,
    48, 74, 48, 75, 48, 76, 48, 77, 48, 78, 48, 79, 48, 81, 48, 83, 48, 84, 48, 85, 48, 86, 48, 87, 48, 88, 48, 89, 48,
    90, 48, 91, 49, 13, 49, 15, 49, 34, 49, 43, 49, 53, 49, 55, 49, 56, 49, 57, 49, 58, 49, 59, 49, 66, 49, 68, 49, 69,
    49, 70, 49, 72, 49, 74, 49, 75, 49, 80, 49, 82, 49, 84, 49, 90, 50, 34, 50, 36, 50, 40, 50, 43, 50, 46, 50, 48, 50,
    50, 50, 52, 50, 53, 50, 54, 50, 55, 50, 56, 50, 57, 50, 58, 50, 59, 50, 66, 50, 67, 50, 71, 50, 72, 50, 73, 50, 74,
    50, 75, 50, 76, 50, 77, 50, 78, 50, 79, 50, 81, 50, 83, 50, 84, 50, 85, 50, 86, 50, 87, 50, 88, 50, 89, 50, 90, 50,
    91, 51, 36, 51, 40, 51, 48, 51, 50, 51, 52, 51, 53, 51, 54, 51, 55, 51, 56, 51, 58, 51, 66, 51, 68, 51, 71, 51, 72,
    51, 74, 51, 75, 51, 82, 51, 85, 51, 86, 51, 87, 51, 88, 51, 90, 52, 34, 52, 43, 52, 52, 52, 53, 52, 55, 52, 56, 52,
    57, 52, 58, 52, 59, 52, 71, 52, 72, 52, 74, 52, 75, 52, 84, 52, 85, 52, 87, 52, 88, 52, 89, 52, 90, 52, 91, 53, 13,
    53, 15, 53, 34, 53, 36, 53, 40, 53, 43, 53, 48, 53, 50, 53, 52, 53, 66, 53, 71, 53, 72, 53, 75, 53, 78, 53, 79, 53,
    81, 53, 82, 53, 83, 53, 84, 53, 85, 53, 86, 53, 87, 53, 88, 53, 89, 53, 90, 53, 91, 54, 34, 54, 43, 54, 52, 54, 53,
    54, 55, 54, 56, 54, 57, 54, 58, 54, 66, 54, 71, 54, 72, 54, 74, 54, 75, 54, 84, 54, 85, 54, 87, 54, 88, 54, 89, 54,
    90, 54, 91, 55, 13, 55, 15, 55, 34, 55, 36, 55, 40, 55, 43, 55, 48, 55, 50, 55, 52, 55, 66, 55, 68, 55, 69, 55, 70,
    55, 71, 55, 72, 55, 75, 55, 78, 55, 79, 55, 81, 55, 82, 55, 83, 55, 84, 55, 85, 55, 86, 55, 87, 55, 88, 55, 89, 55,
    90, 55, 91, 56, 13, 56, 15, 56, 34, 56, 36, 56, 40, 56, 43, 56, 48, 56, 50, 56, 52, 56, 66, 56, 69, 56, 70, 56, 71,
    56, 72, 56, 75, 56, 78, 56, 79, 56, 81, 56, 83, 56, 84, 56, 85, 56, 86, 56, 87, 56, 88, 56, 89, 56, 90, 56, 91, 57,
    36, 57, 40, 57, 48, 57, 50, 57, 66, 57, 68, 57, 69, 57, 71, 57, 74, 57, 75, 57, 82, 57, 85, 57, 86, 57, 87, 57, 88,
    57, 90, 58, 13, 58, 15, 58, 34, 58, 36, 58, 40, 58, 43, 58, 48, 58, 50, 58, 52, 58, 66, 58, 68, 58, 69, 58, 70, 58,
    71, 58, 72, 58, 75, 58, 78, 58, 79, 58, 81, 58, 83, 58, 84, 58, 85, 58, 86, 58, 87, 58, 88, 58, 89, 58, 90, 58, 91,
    59, 36, 59, 40, 59, 48, 59, 50, 59, 69, 59, 71, 59, 72, 59, 75, 59, 85, 59, 87, 59, 88, 59, 90, 60, 43, 60, 75, 66,
    90, 67, 2,  67, 32, 67, 66, 67, 71, 67, 72, 67, 75, 67, 84, 67, 85, 67, 87, 67, 88, 67, 89, 67, 90, 67, 91, 68, 32,
    68, 66, 68, 69, 68, 72, 68, 74, 68, 75, 68, 82, 68, 90, 69, 75, 69, 90, 70, 2,  70, 32, 70, 66, 70, 71, 70, 72, 70,
    74, 70, 75, 70, 84, 70, 85, 70, 87, 70, 88, 70, 89, 70, 90, 70, 91, 71, 13, 71, 15, 71, 16, 71, 66, 71, 69, 71, 72,
    71, 75, 71, 82, 71, 84, 71, 90, 71, 91, 72, 2,  72, 32, 72, 66, 72, 69, 72, 74, 72, 75, 72, 82, 72, 84, 72, 90, 73,
    75, 73, 90, 74, 90, 76, 32, 76, 66, 76, 68, 76, 69, 76, 70, 76, 72, 76, 74, 76, 75, 76, 80, 76, 82, 76, 90, 76, 91,
    77, 75, 77, 90, 78, 90, 79, 90, 80, 2,  80, 32, 80, 66, 80, 71, 80, 72, 80, 75, 80, 84, 80, 85, 80, 87, 80, 88, 80,
    89, 80, 90, 80, 91, 81, 2,  81, 32, 81, 66, 81, 71, 81, 72, 81, 75, 81, 84, 81, 85, 81, 87, 81, 88, 81, 89, 81, 90,
    81, 91, 82, 32, 82, 74, 82, 90, 83, 2,  83, 13, 83, 15, 83, 32, 83, 66, 83, 68, 83, 69, 83, 70, 83, 72, 83, 74, 83,
    75, 83, 80, 83, 82, 83, 84, 83, 90, 83, 91, 84, 32, 84, 72, 84, 74, 84, 75, 84, 84, 84, 89, 84, 90, 84, 91, 85, 2,
    85, 32, 85, 66, 85, 69, 85, 71, 85, 72, 85, 74, 85, 75, 85, 82, 85, 85, 85, 86, 85, 87, 85, 88, 85, 90, 87, 2,  87,
    13, 87, 15, 87, 32, 87, 66, 87, 69, 87, 72, 87, 74, 87, 75, 87, 84, 87, 91, 88, 2,  88, 13, 88, 15, 88, 32, 88, 66,
    88, 69, 88, 72, 88, 74, 88, 75, 88, 84, 88, 91, 89, 32, 89, 66, 89, 68, 89, 69, 89, 70, 89, 72, 89, 74, 89, 75, 89,
    82, 89, 91, 90, 2,  90, 13, 90, 15, 90, 32, 90, 66, 90, 68, 90, 69, 90, 70, 90, 72, 90, 74, 90, 75, 90, 78, 90, 79,
    90, 80, 90, 82, 90, 83, 90, 84, 90, 91, 91, 32, 91, 66, 91, 68, 91, 69, 91, 70, 91, 74, 91, 80, 91, 82, 91, 84, 91,
    87, 91, 88, 91, 89, 91, 90
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] = {
    -2,  -7,  -8,  -7,  -7,  -9,  -7,  -8,  -2,  -8,  -8,  -8,  -3,  6,   -10, -19, -10, -12, -5,  -5,  -5,  -5,  -10,
    -6,  -6,  -3,  -5,  -1,  -3,  2,   -5,  -2,  4,   -4,  4,   -2,  3,   2,   -3,  -4,  -2,  -2,  -3,  -17, -17, -3,
    2,   -7,  -5,  -5,  -2,  -4,  -3,  -2,  -7,  -7,  -1,  -2,  -7,  -7,  -7,  -7,  -7,  -8,  -7,  -7,  -4,  -8,  -8,
    -8,  -8,  -18, -8,  -12, -11, -16, -2,  -12, -7,  -9,  -10, -2,  -12, -8,  -14, -6,  -2,  -2,  -8,  -2,  -2,  -7,
    -10, -8,  -6,  -13, -12, -4,  -2,  -8,  -6,  -7,  -9,  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -7,  -7,  -7,  -7,  -4,
    -8,  -8,  -8,  -12, -6,  -4,  -9,  -8,  -8,  -16, -5,  -26, -10, -17, -8,  -3,  -3,  -15, -3,  -3,  -3,  -3,  -10,
    -3,  -8,  -6,  -20, -9,  -10, -5,  -3,  -3,  -3,  -3,  -7,  -9,  -3,  -3,  -3,  -3,  -3,  -3,  -4,  -3,  -3,  -4,
    -3,  -2,  -3,  -2,  -5,  -9,  -5,  -9,  -3,  -9,  -7,  -9,  -5,  -3,  -16, -21, -21, -12, -12, -6,  -6,  -23, -6,
    -8,  -3,  -12, -9,  -9,  -8,  -8,  -12, -7,  -8,  -8,  -8,  -9,  -8,  -10, -8,  -8,  -10, -9,  -18, -12, -10, -6,
    -6,  -6,  -6,  -6,  -6,  -6,  -2,  -6,  -18, -9,  -4,  -7,  -6,  -6,  -6,  -6,  -6,  -8,  -2,  -10, -6,  -6,  -12,
    -4,  -2,  -7,  -2,  -2,  -7,  -2,  -3,  -7,  -4,  -5,  -6,  -5,  -6,  -6,  -4,  -3,  -2,  -5,  -6,  -9,  -3,  -2,
    -3,  -3,  -4,  -4,  -4,  -12, -21, -12, -21, -6,  -14, -13, -12, -2,  -7,  -9,  -13, -18, -6,  -16, -14, -16, -8,
    -8,  -8,  -9,  -21, -4,  -15, -12, -20, -4,  -3,  -4,  -12, -7,  -9,  -3,  -12, -21, -13, -14, -7,  -2,  -7,  -2,
    -8,  -3,  -3,  -14, -3,  -3,  -3,  -3,  -9,  -3,  -7,  -6,  -9,  -8,  -7,  -5,  -3,  -3,  -3,  -3,  -7,  -6,  -3,
    -3,  -3,  -3,  -3,  -3,  -4,  -3,  -3,  -4,  -3,  -2,  -3,  -4,  -23, -23, -12, -26, -5,  -7,  -5,  -17, -8,  -6,
    -7,  -8,  -8,  -5,  -6,  -7,  -8,  -8,  -8,  -3,  -2,  -14, -3,  -3,  -16, -3,  -3,  -3,  -6,  -11, -3,  -13, -11,
    -21, -18, -13, -7,  -3,  -3,  -6,  -3,  -10, -7,  -3,  -3,  -3,  -3,  -3,  -3,  -4,  -3,  -3,  -4,  -3,  -5,  -6,
    -4,  -9,  -9,  -9,  -9,  -4,  -11, -7,  -10, -7,  -10, -9,  -14, -7,  -2,  -7,  -9,  -13, -6,  -6,  -7,  -7,  -8,
    -6,  -7,  -7,  -2,  -2,  -2,  -4,  -2,  -3,  -12, -5,  -2,  -9,  -9,  -6,  -7,  -4,  -12, -10, -4,  -19, -19, -18,
    -9,  -9,  -40, -9,  -9,  -3,  -22, -12, -42, -7,  -19, -40, -40, -40, -19, -20, -12, -19, -40, -22, -19, -21, -19,
    -8,  -9,  -2,  -4,  -5,  -4,  -5,  -5,  -2,  -2,  -4,  -4,  -9,  -3,  -2,  -2,  -2,  -3,  -3,  -3,  -17, -17, -12,
    -7,  -8,  -35, -7,  -11, -4,  -13, -19, -18, -13, -7,  -19, -7,  -7,  -7,  -7,  -18, -7,  -16, -7,  -6,  -7,  -7,
    -7,  -8,  -8,  -10, -10, -11, -6,  -6,  -26, -6,  -8,  -4,  -11, -11, -11, -4,  -14, -7,  -4,  -4,  -4,  -4,  -12,
    -4,  -5,  -5,  -5,  -5,  -8,  -6,  -9,  -19, -9,  -19, -5,  -12, -12, -12, -4,  -9,  -12, -16, -4,  -22, -17, -12,
    -21, -21, -16, -8,  -16, -39, -8,  -16, -6,  -23, -30, -15, -17, -14, -19, -8,  -14, -14, -13, -14, -17, -10, -13,
    -13, -15, -15, -15, -15, -7,  -9,  -7,  -9,  -3,  -12, -2,  -7,  -12, -15, -13, -8,  -7,  12,  -4,  -7,  -20, -3,
    -5,  -4,  -9,  -3,  -5,  -7,  -4,  -8,  -7,  -4,  -18, -6,  -6,  -6,  -8,  -10, -6,  -7,  -5,  -4,  -5,  -8,  -3,
    -5,  -4,  -5,  -5,  -2,  -5,  -6,  -4,  -4,  -12, -4,  -10, -10, -5,  -7,  -5,  -5,  -5,  -5,  -2,  -5,  -3,  -4,
    -21, -7,  -5,  -7,  2,   -5,  -2,  -5,  -9,  -6,  -4,  -17, -6,  -4,  -4,  -6,  -3,  -7,  -9,  -4,  -12, -5,  -3,
    -7,  -4,  -4,  -4,  -7,  -20, -3,  -5,  -4,  -9,  -3,  -5,  -7,  -4,  -8,  -7,  -4,  -7,  -20, -3,  -5,  -4,  -6,
    -3,  -5,  -7,  -4,  -8,  -7,  -4,  -17, -7,  -4,  -8,  -12, -12, -12, -7,  -10, -11, -10, -11, -7,  -9,  -10, -11,
    -8,  -5,  -5,  -8,  -4,  -7,  -9,  -4,  -2,  -3,  -5,  -2,  -19, -7,  -4,  -3,  -4,  -5,  -10, -8,  -3,  -3,  -3,
    -3,  -5,  -8,  -17, -17, -28, -7,  -6,  -6,  -7,  -9,  -4,  -5,  -8,  -10, -10, -25, -7,  -4,  -4,  -7,  -9,  -2,
    -5,  -17, -6,  -13, -12, -5,  -2,  -7,  -9,  -12, -6,  -8,  -17, -17, -30, -7,  -7,  -7,  -7,  -8,  -7,  -4,  -4,
    -4,  -8,  -7,  -4,  -5,  -5,  -17, -2,  -6,  -6,  -6,  -7,  -6,  -5,  -6,  -7,  -7,  -8,  -10
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs = {
    .glyph_ids = kern_pair_glyph_ids, .values = kern_pair_values, .pair_cnt = 801, .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap  = glyph_bitmap,
    .glyph_dsc     = glyph_dsc,
    .cmaps         = cmaps,
    .kern_dsc      = &kern_pairs,
    .kern_scale    = 16,
    .cmap_num      = 1,
    .bpp           = 1,
    .kern_classes  = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font_syke_regular_size_12 = {
#else
lv_font_t font_syke_regular_size_12 = {
#endif
    .get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height      = 12,                            /*The maximum line height required by the font*/
    .base_line        = 2,                             /*Baseline measured from the bottom of the line*/
#if !( LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0 )
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK( 7, 4, 0 ) || LVGL_VERSION_MAJOR >= 8
    .underline_position  = -1,
    .underline_thickness = 1,
#endif
    .static_bitmap = 0,
    .dsc           = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK( 8, 2, 0 ) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};

#endif /*#if FONT_SYKE_REGULAR_SIZE_12*/
