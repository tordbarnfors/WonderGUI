/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/
#include <wg_pixeltools.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <cstring>
#include <algorithm>

namespace wg { namespace PixelTools
{

typedef	void(*PixelReadFunc)(const uint8_t*, uint8_t*, int, const void*, const void*);
typedef	void(*PixelWriteFunc)(const uint8_t*, uint8_t*, int);


const uint8_t conv_2_linear_to_8_sRGB[4] 	= {0, 155, 212, 255 };

const uint8_t conv_3_linear_to_8_sRGB[8] 	= {0, 105, 144, 173, 198, 219, 238, 255};

const uint8_t conv_4_linear_to_8_sRGB[16] 	= {0, 74, 102, 123, 140, 155, 168, 180, 192, 202,
	212, 221, 230, 239, 247, 255};

const uint8_t conv_5_linear_to_8_sRGB[32] 	= {0, 54, 73, 88, 101, 111, 121, 130, 138, 145, 152,
	159, 166, 172, 178, 183, 189, 194, 199, 204, 209, 214, 218, 223, 227, 231, 235, 239, 243, 247,
	251, 255};

const uint8_t conv_6_linear_to_8_sRGB[64] 	= {0, 39, 53, 64, 73, 81, 88, 94, 100, 105, 110, 115,
	120, 124, 129, 133, 137, 141, 144, 148, 151, 155, 158, 161, 164, 168, 171, 173, 176, 179, 182,
	185, 187, 190, 193, 195, 198, 200, 203, 205, 207, 210, 212, 214, 217, 219, 221, 223, 225, 227,
	230, 232, 234, 236, 238, 240, 242, 244, 246, 248, 249, 251, 253, 255 };

const uint8_t conv_7_linear_to_8_sRGB[128] 	= {0, 28, 39, 46, 53, 59, 64, 68, 73, 77, 80, 84, 87,
	90, 94, 97, 99, 102, 105, 108, 110, 113, 115, 117, 120, 122, 124, 126, 128, 130, 132, 134, 136,
	138, 140, 142, 144, 146, 147, 149, 151, 153, 154, 156, 158, 159, 161, 162, 164, 165, 167, 168,
	170, 171, 173, 174, 176, 177, 179, 180, 181, 183, 184, 185, 187, 188, 189, 191, 192, 193, 195,
	196, 197, 198, 199, 201, 202, 203, 204, 206, 207, 208, 209, 210, 211, 212, 214, 215, 216, 217,
	218, 219, 220, 221, 222, 223, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237,
	238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 249, 250, 251, 252, 253, 254, 255 };

const uint8_t conv_8_linear_to_8_sRGB[256] = {
	0, 21, 28, 34, 39, 43, 46, 50, 53, 56, 59, 61, 64, 66, 68, 70,
	72, 74, 76, 78, 80, 82, 84, 85, 87, 89, 90, 92, 93, 95, 96, 98,
	99, 101, 102, 103, 105, 106, 107, 109, 110, 111, 112, 114, 115, 116, 117, 118,
	119, 120, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145, 146, 147, 148, 149, 150,
	151, 151, 152, 153, 154, 155, 156, 156, 157, 158, 159, 160, 160, 161, 162, 163,
	164, 164, 165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175,
	175, 176, 177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184, 184, 185, 186,
	186, 187, 188, 188, 189, 190, 190, 191, 192, 192, 193, 194, 194, 195, 195, 196,
	197, 197, 198, 199, 199, 200, 200, 201, 202, 202, 203, 203, 204, 205, 205, 206,
	206, 207, 207, 208, 209, 209, 210, 210, 211, 212, 212, 213, 213, 214, 214, 215,
	215, 216, 217, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 223, 223, 224,
	224, 225, 225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232,
	232, 233, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 240,
	240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247, 247, 248,
	248, 249, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255 };


const uint8_t conv_2_sRGB_to_8_linear[4] 	= {0, 23, 105, 255};

const uint8_t conv_3_sRGB_to_8_linear[8] 	= {0, 4, 16, 40, 74, 122, 182, 255};

const uint8_t conv_4_sRGB_to_8_linear[16] = {0, 1, 3, 7, 14, 23, 34, 48, 64, 83, 105, 129, 156, 186, 219, 255};

const uint8_t conv_5_sRGB_to_8_linear[32] = { 0, 0, 1, 1, 3, 5, 7, 10, 13, 17, 21, 26, 32, 38, 44, 52,
	60, 68,	77, 87, 97, 108, 120, 132, 145, 159, 173, 188, 204, 220, 237, 255};

const uint8_t conv_6_sRGB_to_8_linear[64] = { 0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 4, 5, 7, 8, 9, 11, 13, 14,
	16, 18, 20, 23, 25, 28, 31, 33, 36, 40, 43, 46, 50, 54, 57, 61, 66, 70, 74, 79, 84, 89, 94,
	99, 105, 110, 116, 122, 128, 134, 140, 147, 153, 160, 167, 174, 182, 189, 197, 205, 213, 221,
	229, 238, 246, 255};

const uint8_t conv_7_sRGB_to_8_linear[128] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3,
	3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24,
	25, 26, 27, 29, 30, 31, 33, 34, 36, 37, 39, 40, 42, 44, 45, 47, 49, 51, 53, 55, 56, 58, 60, 62,
	65, 67, 69, 71, 73, 75, 78, 80, 82, 85, 87, 90, 92, 95, 97, 100, 103, 105, 108, 111, 114, 117,
	120, 122, 125, 128, 132, 135, 138, 141, 144, 147, 151, 154, 157, 161, 164, 168, 171, 175, 179,
	182, 186, 190, 193, 197, 201, 205, 209, 213, 217, 221, 225, 229, 233, 238, 242, 246, 251, 255};

const uint8_t conv_8_sRGB_to_8_linear[256] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 8,
	8, 8, 9, 9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18,
	19, 19, 20, 20, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 33, 33,
	34, 35, 35, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78,
	79, 81, 82, 83, 84, 85, 87, 88, 89, 90, 91, 93, 94, 95, 97, 98, 99, 100, 102, 103, 105, 106, 107,
	109, 110, 111, 113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
	137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161, 163, 165, 166,
	168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190, 192, 194, 196, 197, 199, 201,
	203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 227, 229, 231, 234, 236, 238, 240,
	242, 244, 246, 248, 251, 253, 255 };


const uint8_t conv_0_to_8_straight[1] 	= { 0xff };
const uint8_t conv_1_to_8_straight[2] 	= { 0, 0xff };
const uint8_t conv_2_to_8_straight[4] 	= {	0, 0x55, 0xaa, 0xff };
const uint8_t conv_3_to_8_straight[8] 	= { 0, 0x24, 0x48, 0x6d, 0x91, 0xb6, 0xda, 0xff };

const uint8_t conv_4_to_8_straight[16] = { 0, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

const uint8_t conv_5_to_8_straight[32] = { 0x00, 0x08, 0x10, 0x18, 0x20, 0x29, 0x31, 0x39, 0x41,
	0x4a, 0x52, 0x5a, 0x62, 0x6a, 0x73, 0x7b, 0x83, 0x8b, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbd, 0xc5,
	0xcd, 0xd5, 0xde, 0xe6, 0xee, 0xf6, 0xff };

const uint8_t conv_6_to_8_straight[64] 	= {	0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20,
	0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x50, 0x55, 0x59, 0x5d, 0x61,
	0x65, 0x69, 0x6d, 0x71, 0x75, 0x79, 0x7d, 0x81, 0x85, 0x89, 0x8d, 0x91, 0x95, 0x99, 0x9d, 0xa1,
	0xa5, 0xaa, 0xae, 0xb2, 0xb6, 0xba, 0xbe, 0xc2, 0xc6, 0xca, 0xce, 0xd2, 0xd6, 0xda, 0xde, 0xe2,
	0xe6, 0xea, 0xee, 0xf2, 0xf6, 0xfa, 0xff };

const uint8_t conv_7_to_8_straight[128] = { 0, 0x2, 0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x12, 0x14,
	0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34,
	0x36, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52,
	0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, 0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70,
	0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e,
	0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, 0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac,
	0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe, 0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca,
	0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde, 0xe0, 0xe2, 0xe4, 0xe6, 0xe8,
	0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xff };

const uint8_t conv_8_to_8_straight[256] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF };

const uint16_t conv_8_sRGB_to_16_linear[256] = { 0, 1, 2, 4, 7, 11, 17, 24, 32, 42, 53, 65, 79, 94,
	111, 129, 148, 169, 192, 216, 242, 270, 299, 330, 362, 396, 432, 469, 508, 549, 591, 635, 681,
	729, 779, 830, 883, 938, 995, 1053, 1113, 1175, 1239, 1305, 1373, 1443, 1514, 1587, 1663, 1740,
	1819, 1900, 1983, 2068, 2155, 2243, 2334, 2427, 2521, 2618, 2717, 2817, 2920, 3024, 3131, 3240,
	3350, 3463, 3578, 3694, 3813, 3934, 4057, 4182, 4309, 4438, 4570, 4703, 4838, 4976, 5115, 5257,
	5401, 5547, 5695, 5845, 5998, 6152, 6309, 6468, 6629, 6792, 6957, 7124, 7294, 7466, 7640, 7816,
	7994, 8175, 8358, 8543, 8730, 8919, 9111, 9305, 9501, 9699, 9900, 10102, 10307, 10515, 10724,
	10936, 11150, 11366, 11585, 11806, 12029, 12254, 12482, 12712, 12944, 13179, 13416, 13655, 13896,
	14140, 14386, 14635, 14885, 15138, 15394, 15652, 15912, 16174, 16439, 16706, 16975, 17247, 17521,
	17798, 18077, 18358, 18642, 18928, 19216, 19507, 19800, 20095, 20393, 20694, 20996, 21301, 21609,
	21919, 22231, 22546, 22863, 23182, 23504, 23829, 24156, 24485, 24817, 25151, 25487, 25826, 26168,
	26512, 26858, 27207, 27558, 27912, 28268, 28627, 28988, 29351, 29717, 30086, 30457, 30830, 31206,
	31585, 31966, 32349, 32735, 33124, 33514, 33908, 34304, 34702, 35103, 35507, 35913, 36321, 36732,
	37146, 37562, 37981, 38402, 38825, 39252, 39680, 40112, 40546, 40982, 41421, 41862, 42306, 42753,
	43202, 43654, 44108, 44565, 45025, 45487, 45951, 46418, 46888, 47360, 47835, 48313, 48793, 49275,
	49761, 50249, 50739, 51232, 51728, 52226, 52727, 53230, 53736, 54245, 54756, 55270, 55787, 56306,
	56828, 57352, 57879, 58409, 58941, 59476, 60014, 60554, 61097, 61642, 62190, 62741, 63295, 63851,
	64410, 64971, 65535 };

uint8_t* pConv_16_linear_to_8_sRGB = nullptr;


const uint8_t * conv_srgb_to_linear_tabs[9] = { conv_0_to_8_straight, conv_1_to_8_straight,
	conv_2_sRGB_to_8_linear, conv_3_sRGB_to_8_linear, conv_4_sRGB_to_8_linear,
	conv_5_sRGB_to_8_linear, conv_6_sRGB_to_8_linear, conv_7_sRGB_to_8_linear, conv_8_sRGB_to_8_linear };

const uint8_t * conv_linear_to_sRGB_tabs[9] = { conv_0_to_8_straight, conv_1_to_8_straight, conv_2_linear_to_8_sRGB,
	conv_3_linear_to_8_sRGB, conv_4_linear_to_8_sRGB, conv_5_linear_to_8_sRGB,
	conv_6_linear_to_8_sRGB, conv_7_linear_to_8_sRGB, conv_8_linear_to_8_sRGB };

const uint8_t * conv_straight_tabs[9] = { conv_0_to_8_straight, conv_1_to_8_straight, conv_2_to_8_straight,
	conv_3_to_8_straight, conv_4_to_8_straight, conv_5_to_8_straight,
	conv_6_to_8_straight, conv_7_to_8_straight, conv_8_to_8_straight };


static void createConv16toSRGBTab()
{
	uint8_t * pBuff = new uint8_t[65536];

	for (int i = 0; i < 255; i++)
	{
		int ofs = conv_8_sRGB_to_16_linear[i];
		int end = conv_8_sRGB_to_16_linear[i+1];

		int midPoint = (ofs + end) / 2;

		while (ofs <= midPoint)
		{
			pBuff[ofs] = i;
			ofs++;
		}

		while (ofs < end)
		{
			pBuff[ofs] = i+1;
			ofs++;
		}
	}

	pBuff[65535] = 255;
	pConv_16_linear_to_8_sRGB = pBuff;
}


static void readBGRA8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* (uint32_t*) pDst = *(uint32_t*) pSrc;
		pSrc += 4;
		pDst += 4;
	}
}

static void readConvBGRA8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pRGBConvTab = (uint8_t*) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = * pSrc++;
	}
}

static void readBGRX8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* (uint32_t*) pDst = (*(uint32_t*) pSrc) | 0xFF000000;
		pSrc += 4;
		pDst += 4;
	}
}

static void readConvBGRX8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pRGBConvTab = (uint8_t*) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = 255;
	}
}


static void readBGR8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* pDst++ = *pSrc++;
		* pDst++ = *pSrc++;
		* pDst++ = *pSrc++;
		* pDst++ = 255;
	}
}

static void readConvBGR8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pRGBConvTab = (uint8_t *) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = pRGBConvTab[*pSrc++];
		* pDst++ = 255;
	}
}

static void readBGRA16(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	pSrc++;								// Point at high-order byte of channel
	for (int i = 0; i < nbPixels; i++)
	{
		*pDst++ = *pSrc;
		pSrc += 2;
		*pDst++ = *pSrc;
		pSrc += 2;
		*pDst++ = *pSrc;
		pSrc += 2;
		*pDst++ = *pSrc;
		pSrc += 2;
	}
}

static void readConvBGRA16(const uint8_t* _pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	//TODO: We lose a lot of precision by only using 8-bit linear values when converting to 8-bit SRGB.

	uint8_t* pRGBConvTab = (uint8_t*)p1;

	uint16_t * pSrc = (uint16_t*) _pSrc;								// Point at high-order byte of channel
	for (int i = 0; i < nbPixels; i++)
	{
		*pDst++ = pRGBConvTab[*pSrc++];
		*pDst++ = pRGBConvTab[*pSrc++];
		*pDst++ = pRGBConvTab[*pSrc++];
		*pDst++ = (*pSrc++) >> 8;
	}
}



static void readAlpha8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* pDst++ = ((*pSrc++) << 24) | 0x00FFFFFF;
	}
}

static void readIndex8( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint32_t * pPalette = (uint32_t*) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		* (uint32_t*) pDst = pPalette[*pSrc++];
		pDst += 4;
	}
}

static void readIndex16(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint32_t* pPalette = (uint32_t*)p1;

	for (int i = 0; i < nbPixels; i++)
	{

		*(uint32_t*)pDst = pPalette[*((uint16_t*)pSrc)];
		pDst += 4;
		pSrc += 2;
	}
}


static void readBGRA_4( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pConvTab = (uint8_t*) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		uint8_t bg = * pSrc++;
		* pDst++ = pConvTab[bg&0xF];
		* pDst++ = pConvTab[bg>>4];

		uint8_t ra = * pSrc++;
		* pDst++ = pConvTab[ra&0xF];
		* pDst++ = conv_4_to_8_straight[ra>>4];
	}
}


static void readBGR_565( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2  )
{
	uint8_t * pConvTab5 = (uint8_t*) p1;
	uint8_t * pConvTab6 = (uint8_t*) p2;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		uint16_t bgr = *(uint16_t*) pSrc;

		* pDst++ = pConvTab5[bgr & 0x1F];
		* pDst++ = pConvTab6[(bgr >> 5)  & 0x3F];
		* pDst++ = pConvTab5[bgr >> 11];
		* pDst++ = 255;
		pSrc += 2;
	}
}

static void readRGB_565_bigendian( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pConvTab5 = (uint8_t*) p1;
	uint8_t * pConvTab6 = (uint8_t*) p2;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		uint16_t rgb = *(uint16_t*) pSrc;
		rgb = (rgb >> 8 | rgb << 8);

		* pDst++ = pConvTab5[rgb >> 11];
		* pDst++ = pConvTab6[(rgb >> 5)  & 0x3F];
		* pDst++ = pConvTab5[rgb & 0x1F];
		* pDst++ = 255;
		pSrc += 2;
	}
}

static void readRGB_555_bigendian( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void * p1, const void * p2 )
{
	uint8_t * pConvTab5 = (uint8_t*) p1;

	for( int i = 0 ; i < nbPixels ; i++ )
	{
		uint16_t rgb = *(uint16_t*) pSrc;
		rgb = (rgb >> 8 | rgb << 8);

		* pDst++ = pConvTab5[rgb >> 11];
		* pDst++ = pConvTab5[(rgb >> 6)  & 0x1F];
		* pDst++ = pConvTab5[rgb & 0x1F];
		* pDst++ = 255;
		pSrc += 2;
	}
}

struct ShiftReadConvTab
{
	uint32_t maskR;
	uint32_t maskG;
	uint32_t maskB;
	uint32_t maskA;
	int shiftR;
	int shiftG;
	int shiftB;
	int shiftA;
	const uint8_t* pConvR;
	const uint8_t* pConvG;
	const uint8_t* pConvB;
	const uint8_t* pConvA;
	uint64_t	maskR64;
	uint64_t	maskG64;
	uint64_t	maskB64;
	uint64_t	maskA64;

};


static void shiftRead_64bit_to64(const uint8_t* _pSrc, uint8_t* _pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	uint64_t* pSrc = (uint64_t*)_pSrc;
	uint16_t* pDst = (uint16_t*)_pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t rgba = * pSrc++;

		*pDst++ = (rgba & t.maskB64) >> t.shiftB;
		*pDst++ = (rgba & t.maskG64) >> t.shiftG;
		*pDst++ = (rgba & t.maskR64) >> t.shiftR;
		*pDst++ = (rgba & t.maskA64) >> t.shiftA;
	}
}


static void shiftRead_64bit(const uint8_t* _pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	uint64_t* pSrc = (uint64_t*)_pSrc;

	int shiftB = t.shiftB + 8;
	int shiftG = t.shiftG + 8;
	int shiftR = t.shiftR + 8;
	int shiftA = t.shiftA + 8;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t rgba = *pSrc++;

		*pDst++ = (rgba & t.maskB64) >> shiftB;
		*pDst++ = (rgba & t.maskG64) >> shiftG;
		*pDst++ = (rgba & t.maskR64) >> shiftR;
		*pDst++ = (rgba & t.maskA64) >> shiftA;
	}
}


static void shiftReadConv_64bit(const uint8_t* _pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	uint64_t* pSrc = (uint64_t*)_pSrc;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t rgba = * pSrc++;

		*pDst++ = t.pConvB[(rgba & t.maskB64) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG64) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR64) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA64) >> t.shiftA];
	}
}


static void shiftReadConv_32bit( const uint8_t * pSrc, uint8_t * pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint32_t rgba = *(uint32_t*)pSrc;

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA) >> t.shiftA];
		pSrc += 4;
	}
}

static void shiftReadConv_32bit_swap(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void * pConvTab, const void * pDummy )
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint32_t rgba = Util::endianSwap(*(uint32_t*)pSrc);

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA) >> t.shiftA];
		pSrc += 4;
	}
}


static void shiftReadConv_24bit_le(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint32_t rgba = pSrc[0] + (uint32_t(pSrc[1]) << 8) + (uint32_t(pSrc[2]) << 16);

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = 255;
		pSrc += 3;
	}
}

static void shiftReadConv_24bit_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint32_t rgba = pSrc[2] + (uint32_t(pSrc[1]) << 8) + (uint32_t(pSrc[0]) << 16);

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = 255;
		pSrc += 3;
	}
}

static void shiftReadConv_16bit(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint16_t rgba = *(uint16_t*)pSrc;

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA) >> t.shiftA];
		pSrc += 2;
	}
}

static void shiftReadConv_16bit_swap(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint16_t rgba = *(uint16_t*)pSrc;
		rgba = (rgba >> 8 | rgba << 8);

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA) >> t.shiftA];
		pSrc += 2;
	}
}

static void shiftReadConv_8bit(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* pConvTab, const void* pDummy)
{
	ShiftReadConvTab& t = *(ShiftReadConvTab*)pConvTab;

	for (int i = 0; i < nbPixels; i++)
	{
		uint8_t rgba = * pSrc++;

		*pDst++ = t.pConvB[(rgba & t.maskB) >> t.shiftB];
		*pDst++ = t.pConvG[(rgba & t.maskG) >> t.shiftG];
		*pDst++ = t.pConvR[(rgba & t.maskR) >> t.shiftR];
		*pDst++ = t.pConvA[(rgba & t.maskA) >> t.shiftA];
	}
}

static void readConv_planes_8i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while(nbPixels > 0)
	{
		uint16_t	plane1 = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane2 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane3 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane4 = (pSrc[6] << 8) + pSrc[7];
		uint16_t	plane5 = (pSrc[8] << 8) + pSrc[9];
		uint16_t	plane6 = (pSrc[10] << 8) + pSrc[11];
		uint16_t	plane7 = (pSrc[12] << 8) + pSrc[13];
		uint16_t	plane8 = (pSrc[14] << 8) + pSrc[15];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12)
				| ((plane5 & 0x80) >> 11) | ((plane6 & 0x80) >> 10) | ((plane7 & 0x80) >> 9) | ((plane8 & 0x80) >> 8);
			*(uint32_t*)pDst = pPalette[index];
			pDst += 4;

			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
			plane5 <<= 1;
			plane6 <<= 1;
			plane7 <<= 1;
			plane8 <<= 1;
		}

		pSrc += 16;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_5i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	plane1 = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane2 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane3 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane4 = (pSrc[6] << 8) + pSrc[7];
		uint16_t	plane5 = (pSrc[8] << 8) + pSrc[9];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12) | ((plane5 & 0x80) >> 11);
			*(uint32_t*)pDst = pPalette[index];
			pDst += 4;

			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
			plane5 <<= 1;
		}

		pSrc += 10;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_4i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	plane1 = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane2 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane3 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane4 = (pSrc[6] << 8) + pSrc[7];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12);
			*(uint32_t*)pDst = pPalette[index];
			pDst += 4;

			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
		}

		pSrc += 8;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_2i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	plane1 = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane2 = (pSrc[2] << 8) + pSrc[3];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14);
			*(uint32_t*)pDst = pPalette[index];
			pDst += 4;

			plane1 <<= 1;
			plane2 <<= 1;
		}

		pSrc += 4;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_1i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	plane1 = (pSrc[0] << 8) + pSrc[1];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15);
			*(uint32_t*)pDst = pPalette[index];
			pDst += 4;

			plane1 <<= 1;
		}

		pSrc += 2;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_a1_8i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	planeA = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane1 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane2 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane3 = (pSrc[6] << 8) + pSrc[7];
		uint16_t	plane4 = (pSrc[8] << 8) + pSrc[9];
		uint16_t	plane5 = (pSrc[10] << 8) + pSrc[11];
		uint16_t	plane6 = (pSrc[12] << 8) + pSrc[13];
		uint16_t	plane7 = (pSrc[14] << 8) + pSrc[15];
		uint16_t	plane8 = (pSrc[16] << 8) + pSrc[17];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12)
				| ((plane5 & 0x80) >> 11) | ((plane6 & 0x80) >> 10) | ((plane7 & 0x80) >> 9) | ((plane8 & 0x80) >> 8);

			uint32_t argb = pPalette[index];

			argb |= conv_1_to_8_straight[((planeA & 0x80) >> 15)];

			*(uint32_t*)pDst = argb;
			pDst += 4;

			planeA <<= 1;
			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
			plane5 <<= 1;
			plane6 <<= 1;
			plane7 <<= 1;
			plane8 <<= 1;
		}

		pSrc += 18;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_a1_5i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	planeA = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane1 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane2 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane3 = (pSrc[6] << 8) + pSrc[7];
		uint16_t	plane4 = (pSrc[8] << 8) + pSrc[9];
		uint16_t	plane5 = (pSrc[10] << 8) + pSrc[11];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12) | ((plane5 & 0x80) >> 11);

			uint32_t argb = pPalette[index];

			argb |= conv_1_to_8_straight[((planeA & 0x80) >> 15)];

			*(uint32_t*)pDst = argb;
			pDst += 4;

			planeA <<= 1;
			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
			plane5 <<= 1;
		}

		pSrc += 12;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_a1_4i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	planeA = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane1 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane2 = (pSrc[4] << 8) + pSrc[5];
		uint16_t	plane3 = (pSrc[6] << 8) + pSrc[7];
		uint16_t	plane4 = (pSrc[8] << 8) + pSrc[9];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14) | ((plane3 & 0x80) >> 13) | ((plane4 & 0x80) >> 12);

			uint32_t argb = pPalette[index];

			argb |= conv_1_to_8_straight[((planeA & 0x80) >> 15)];

			*(uint32_t*)pDst = argb;
			pDst += 4;

			planeA <<= 1;
			plane1 <<= 1;
			plane2 <<= 1;
			plane3 <<= 1;
			plane4 <<= 1;
		}

		pSrc += 10;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_a1_2i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	planeA = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane1 = (pSrc[2] << 8) + pSrc[3];
		uint16_t	plane2 = (pSrc[4] << 8) + pSrc[5];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15) | ((plane2 & 0x80) >> 14);

			uint32_t argb = pPalette[index];

			argb |= conv_1_to_8_straight[((planeA & 0x80) >> 15)];

			*(uint32_t*)pDst = argb;
			pDst += 4;

			planeA <<= 1;
			plane1 <<= 1;
			plane2 <<= 1;
		}

		pSrc += 6;
		nbPixels -= pixNow;
	}
}

static void readConv_planes_a1_1i_be(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* _pPalette, const void* pDummy)
{
	auto pPalette = (const uint32_t *) _pPalette;

	while (nbPixels > 0)
	{
		uint16_t	planeA = (pSrc[0] << 8) + pSrc[1];
		uint16_t	plane1 = (pSrc[2] << 8) + pSrc[3];

		int pixNow = std::min(16, nbPixels);
		for (int i = 0; i < pixNow; i++)
		{
			int		index = ((plane1 & 0x80) >> 15);

			uint32_t argb = pPalette[index];

			argb |= conv_1_to_8_straight[((planeA & 0x80) >> 15)];

			*(uint32_t*)pDst = argb;
			pDst += 4;

			planeA <<= 1;
			plane1 <<= 1;
		}

		pSrc += 4;
		nbPixels -= pixNow;
	}
}

static void read16_BGRA16_linear(const uint8_t* _pSrc, uint8_t* _pDst, int nbPixels, const void* p1, const void* p2)
{
	auto pSrc = (const uint64_t*) _pSrc;
	auto pDst = (uint64_t*) _pDst;

	for (int i = 0; i < nbPixels; i++)
		*pDst++ = *pSrc++;
}


static void read16_BGRA8_linear(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;

		*(uint64_t*)pDst = dest | (dest << 8);
		pDst += 8;
	}
}

static void read16_BGRA8_sRGB(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];

		*(uint64_t*)pDst = dest;
		pDst += 8;
	}
}

static void read16_BGRX8_linear(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;
		pSrc++;

		*(uint64_t*)pDst = dest | (dest << 8) | 0xFFFF;
		pDst += 8;
	}
}

static void read16_BGRX8_sRGB(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		pSrc++;

		*(uint64_t*)pDst = dest | 0xFFFF;;
		pDst += 8;
	}
}

static void read16_BGR8_linear(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;
		dest |= *pSrc++;
		dest <<= 16;

		*(uint64_t*)pDst = dest | (dest << 8) | 0xFFFF;
		pDst += 8;
	}
}

static void read16_BGR8_sRGB(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t dest = conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;
		dest |= conv_8_sRGB_to_16_linear[*pSrc++];
		dest <<= 16;

		*(uint64_t*)pDst = dest | 0xFFFF;
		pDst += 8;
	}
}

static void read16_Alpha_8(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	for (int i = 0; i < nbPixels; i++)
	{
		int dest = *pSrc++;
		*(uint64_t*)pDst = 0xFFFFFFFFFFFFFF00 | dest | (dest << 8 );
		pDst += 8;
	}
}

static void read16_Index8(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pPalette = (uint64_t*)p1;

	for (int i = 0; i < nbPixels; i++)
	{
		*(uint64_t*)pDst = pPalette[*pSrc++];
		pDst += 8;
	}
}

static void read16_Index16(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pPalette = (uint64_t*)p1;

	for (int i = 0; i < nbPixels; i++)
	{

		*(uint64_t*)pDst = pPalette[*((uint16_t*)pSrc)];
		pDst += 8;
		pSrc += 2;
	}
}


static void read16_BGRA_4(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pOut = (uint64_t*) pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t	acc = 0;

		uint8_t bg = *pSrc++;
		acc = conv_4_to_8_straight[bg & 0xF];
		acc <<= 16;

		acc |= conv_4_to_8_straight[bg >> 4];
		acc <<= 16;

		uint8_t ra = *pSrc++;
		acc |= conv_4_to_8_straight[ra & 0xF];
		acc <<= 16;

		acc |= conv_4_to_8_straight[ra >> 4];

		*pDst++ = acc | (acc << 8);
	}
}

static void read16_BGR_565_sRGB(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pOut = (uint64_t*)pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t	acc = 0;

		uint16_t bgr = *(uint16_t*)pSrc;

		acc = conv_5_sRGB_to_8_linear[bgr & 0x1F];
		acc <<= 16;

		acc |= conv_6_sRGB_to_8_linear[(bgr >> 5) & 0x3F];
		acc <<= 16;

		acc |= conv_5_sRGB_to_8_linear[bgr >> 11];
		acc <<= 16;

		*pDst++ = acc | (acc << 8) | 0xFFFF;
		pSrc += 2;
	}
}

static void read16_BGR_565_linear(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pOut = (uint64_t*)pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t	acc = 0;

		uint16_t bgr = *(uint16_t*)pSrc;

		acc = conv_5_to_8_straight[bgr & 0x1F];
		acc <<= 16;

		acc |= conv_6_to_8_straight[(bgr >> 5) & 0x3F];
		acc <<= 16;

		acc |= conv_5_to_8_straight[bgr >> 11];
		acc <<= 16;

		*pDst++ = acc | (acc << 8) | 0xFFFF;
		pSrc += 2;
	}
}

static void read16_RGB_565_bigendian(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pOut = (uint64_t*)pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t	acc = 0;

		uint16_t rgb = *(uint16_t*)pSrc;
		rgb = (rgb >> 8 | rgb << 8);

		acc = conv_5_to_8_straight[rgb & 0x1F];
		acc <<= 16;

		acc |= conv_6_to_8_straight[(rgb >> 5) & 0x3F];
		acc <<= 16;

		acc |= conv_5_to_8_straight[rgb >> 11];
		acc <<= 16;

		*pDst++ = acc | (acc << 8) | 0xFFFF;
		pSrc += 2;
	}
}

static void read16_RGB_555_bigendian(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	uint64_t* pOut = (uint64_t*)pDst;

	for (int i = 0; i < nbPixels; i++)
	{
		uint64_t	acc = 0;

		uint16_t rgb = *(uint16_t*)pSrc;
		rgb = (rgb >> 8 | rgb << 8);

		acc = conv_5_to_8_straight[rgb & 0x1F];
		acc <<= 16;

		acc |= conv_5_to_8_straight[(rgb >> 6) & 0x1F];
		acc <<= 16;

		acc |= conv_5_to_8_straight[rgb >> 11];
		acc <<= 16;

		*pDst++ = acc | (acc << 8) | 0xFFFF;
		pSrc += 2;
	}
}

static void read16_bitplanes(const uint8_t* pSrc, uint8_t* pDst, int nbPixels, const void* p1, const void* p2)
{
	PixelReadFunc pFunc = (PixelReadFunc) p2;

	pFunc(pSrc, pDst, nbPixels, p1, nullptr);

	uint8_t* pCpySrc = pDst + nbPixels*4;
	uint8_t* pCpyDst = pDst + nbPixels*8;

	for (int i = 0; i < nbPixels * 4; i++)
	{
		* --pCpyDst = * --pCpySrc;
		* --pCpyDst = * pCpySrc;
	}
}



static void copy_BGRA_8_to_BGR_8(const uint8_t* pSrc, uint8_t* pDst, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		pSrc += 1;
	}
}

static void copy_BGRA_8_to_BGRA_4(const uint8_t* pSrc, uint8_t* pDst, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		uint8_t dst = ((*pSrc++) >> 4);
		dst |= ((*pSrc++) & 0xF0);
		*pDst++ = dst;

		dst = ((*pSrc++) >> 4);
		dst |= ((*pSrc++) & 0xF0);
		*pDst++ = dst;
	}
}

static void copy_BGRA_8_to_BGR_565(const uint8_t* _pSrc, uint8_t* _pDst, int amount)
{
	auto pSrc = (uint32_t*)_pSrc;
	auto pDst = (uint16_t*)_pDst;

	for (int i = 0; i < amount; i++)
	{
		uint32_t col = * pSrc++;
		* pDst++ = ((col >> 3) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF80000) >> 8);
	}
}

static void copy_BGRA_8_to_RGB_565BE(const uint8_t* _pSrc, uint8_t* _pDst, int amount)
{
	auto pSrc = (uint32_t*)_pSrc;
	auto pDst = (uint16_t*)_pDst;

	for (int i = 0; i < amount; i++)
	{
		uint32_t col = * pSrc++;
        uint16_t out = ((col >> 8) & 0xF800) | ((col >> 5) & 0x7E0) | ((col >> 3 ) & 0x1F);
//        uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF8) << 8);
		out = (out >> 8 | out << 8);
		* pDst++ = out;
	}
}

static void copy_BGRA_8_to_RGB_555BE(const uint8_t* _pSrc, uint8_t* _pDst, int amount)
{
	auto pSrc = (uint32_t*)_pSrc;
	auto pDst = (uint16_t*)_pDst;

	for (int i = 0; i < amount; i++)
	{
		uint32_t col = *pSrc++;
		uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7C0) | ((col & 0xF8) << 8);
		out = (out >> 8 | out << 8);
		*pDst++ = out;
	}
}

static void copy_BGRA_8_to_Alpha_8(const uint8_t* pSrc, uint8_t* pDst, int amount)
{
	for (int i = 0; i < amount; i++)
	{
		pSrc += 3;
		*pDst++ = * pSrc++;
	}
}

//____ getChunkyWriteFuncFromBGRA8() _______________________________________________

static PixelWriteFunc getChunkyWriteFuncFromBGRA8(PixelFormat dstFmt)
{
	switch (dstFmt)
	{
	case PixelFormat::BGR_8_sRGB:
	case PixelFormat::BGR_8_linear:
		return copy_BGRA_8_to_BGR_8;

	case PixelFormat::BGRA_4_linear:
		return copy_BGRA_8_to_BGRA_4;

	case PixelFormat::BGR_565_sRGB:
	case PixelFormat::BGR_565_linear:
		return copy_BGRA_8_to_BGR_565;

	case PixelFormat::RGB_565_bigendian:
		return copy_BGRA_8_to_RGB_565BE;

	case PixelFormat::RGB_555_bigendian:
		return copy_BGRA_8_to_RGB_555BE;

	case PixelFormat::Alpha_8:
		return copy_BGRA_8_to_Alpha_8;

	default:
		return nullptr;
	}
}


//____ twoStepCopyToChunky() _________________________________________________

static bool twoStepCopyToChunky(int width, int height, const uint8_t* pSrc, int srcPixelBits, int srcPitchAdd,
	uint8_t* pDst, int dstPixelBits, int dstPitchAdd, PixelReadFunc pReadFunc, const void* pTab1, const void* pTab2,
	PixelWriteFunc pWriteFunc )
{
	uint8_t	buffer[64 * 4];

	for (int y = 0; y < height; y++)
	{
		int x = 0;
		int widthLeft = width;
		while (widthLeft > 64)
		{
			pReadFunc(pSrc, buffer, 64, pTab1, pTab2);
			pSrc += srcPixelBits * 8;

			pWriteFunc(buffer, pDst, 64);
			pDst += dstPixelBits * 8;

			widthLeft -= 64;
		}

		if (widthLeft > 0)
		{
			pReadFunc(pSrc, buffer, widthLeft, pTab1, pTab2);
			pSrc += srcPixelBits / 8 * widthLeft;

			pWriteFunc(buffer, pDst, widthLeft);
			pDst += dstPixelBits / 8 * widthLeft;
		}

		pSrc += srcPitchAdd;
		pDst += dstPitchAdd;
	}

	return true;
}


//____ convertPixelsToKnownType() _________________________________________________________

static bool convertPixelsToKnownType( int width, int height, const uint8_t * pSrc, int srcPixelBits, int srcPitchAdd,
										uint8_t * pDst, PixelFormat dstFmt, int dstPitchAdd, Color8 * pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries,
										PixelReadFunc pReadFunc, const void * pTab1, const void * pTab2 )
{
	switch( dstFmt)
	{
		case PixelFormat::BGR_8_sRGB:
		case PixelFormat::BGR_8_linear:
		{
			uint8_t	buffer[64*4];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					uint8_t * p = buffer;
					for( int i = 0 ; i < 64 ; i++ )
					{
						* pDst++ = * p++;
						* pDst++ = * p++;
						* pDst++ = * p++;
						p += 1;
					}
					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					uint8_t * p = buffer;
					for( int i = 0 ; i < widthLeft ; i++ )
					{
						* pDst++ = * p++;
						* pDst++ = * p++;
						* pDst++ = * p++;
						p += 1;
					}
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		case PixelFormat::BGRA_4_linear:
		{
			uint8_t	buffer[64*4];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					uint8_t * p = buffer;
					for( int i = 0 ; i < 64 ; i++ )
					{
						uint8_t dst = ((* p++) >> 4);
						dst |= ((* p++) & 0xF0);
						* pDst++ = dst;

						dst = ((* p++) >> 4);
						dst |= ((* p++) & 0xF0);
						* pDst++ = dst;
					}
					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					uint8_t * p = buffer;
					for( int i = 0 ; i < widthLeft ; i++ )
					{
						uint8_t dst = ((* p++) >> 4);
						dst |= ((* p++) & 0xF0);
						* pDst++ = dst;

						dst = ((* p++) >> 4);
						dst |= ((* p++) & 0xF0);
						* pDst++ = dst;
					}
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		case PixelFormat::BGR_565_sRGB:
		case PixelFormat::BGR_565_linear:
		{
			uint32_t	buffer[64];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					for( int i = 0 ; i < 64 ; i++ )
					{
						uint32_t col = buffer[i];
						* (uint16_t*)pDst = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF8) << 8);
						pDst+= 2;
					}

					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					for( int i = 0 ; i < widthLeft ; i++ )
					{
						uint32_t col = buffer[i];
						* (uint16_t*)pDst = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF8) << 8);
						pDst+= 2;
					}
				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		case PixelFormat::Index_8_sRGB:
		case PixelFormat::Index_8_linear:
		{
			uint32_t	buffer[64];
			int 		nColors = dstPaletteEntries;

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					for( int i = 0 ; i < 64 ; i++ )
					{
						Color8 col = * (Color8*) &buffer[i];

						int ofs = 0;
						while( ofs < nColors && pDstPalette[ofs] != col )
							ofs++;
						if( ofs == nColors )
						{
							if( nColors == maxDstPaletteEntries )
							{
								GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Pixel copying aborted. Out of palette entries in destination.", nullptr, nullptr, __func__, __FILE__, __LINE__ );
								return false;
							}

							pDstPalette[ofs] = col;
							nColors++;
						}
						* pDst++ = (uint8_t) ofs;
					}

					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					for( int i = 0 ; i < widthLeft ; i++ )
					{
						Color8 col = * (Color8*) &buffer[i];

						int ofs = 0;
						while( ofs < nColors && pDstPalette[ofs] != col )
							ofs++;
						if( ofs == nColors )
						{
							if( nColors == maxDstPaletteEntries )
							{
								GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Pixel copying aborted. Out of palette entries in destination.", nullptr, nullptr, __func__, __FILE__, __LINE__ );
								return false;
							}

							pDstPalette[ofs] = col;
							nColors++;
						}
						* pDst++ = (uint8_t) ofs;
					}
				}

				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}

			dstPaletteEntries = nColors;
			break;
		}


		case PixelFormat::RGB_565_bigendian:
		{
			uint32_t	buffer[64];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					for( int i = 0 ; i < 64 ; i++ )
					{
						uint32_t col = buffer[i];
						uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF8) << 8);
						out = (out >> 8 | out << 8);
						* (uint16_t*)pDst = out;
						pDst+= 2;
					}

					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					for( int i = 0 ; i < widthLeft ; i++ )
					{
						uint32_t col = buffer[i];
						uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7E0) | ((col & 0xF8) << 8);
						out = (out >> 8 | out << 8);
						* (uint16_t*)pDst = out;
						pDst+= 2;
					}

				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		case PixelFormat::RGB_555_bigendian:
		{
			uint32_t	buffer[64];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits * 8;

					for( int i = 0 ; i < 64 ; i++ )
					{
						uint32_t col = buffer[i];
						uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7C0) | ((col & 0xF8) << 8);
						out = (out >> 8 | out << 8);
						* (uint16_t*)pDst = out;
						pDst+= 2;
					}

					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, (uint8_t*) buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					for( int i = 0 ; i < widthLeft ; i++ )
					{
						uint32_t col = buffer[i];
						uint16_t out = ((col >> 19) & 0x1F) | ((col >> 5) & 0x7C0) | ((col & 0xF8) << 8);
						out = (out >> 8 | out << 8);
						* (uint16_t*)pDst = out;
						pDst+= 2;
					}

				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		case PixelFormat::Alpha_8:
		{
			uint8_t	buffer[64*4];

			for( int y = 0 ; y < height ; y++ )
			{
				int widthLeft = width;
				while( widthLeft > 64 )
				{
					pReadFunc( pSrc, buffer, 64, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * 64;

					for( int i = 0 ; i < 64 ; i++ )
						* pDst++ = buffer[i*4+3];

					widthLeft -= 64;
				}

				if( widthLeft > 0 )
				{
					pReadFunc( pSrc, buffer, widthLeft, pTab1, pTab2 );
					pSrc += srcPixelBits / 8 * widthLeft;

					for( int i = 0 ; i < widthLeft ; i++ )
						* pDst++ = buffer[i*4+3];

				}
				pSrc += srcPitchAdd;
				pDst += dstPitchAdd;
			}
			break;
		}

		default:
		{
			int srcPitch = width * srcPixelBits / 8 + srcPitchAdd;
			int dstPitch = width * 32 / 8 + dstPitchAdd;

			for( int y = 0 ; y < height ; y++ )
			{
				pReadFunc( pSrc, pDst, width, pTab1, pTab2 );
				pSrc += srcPitch;
				pDst += dstPitch;
			}
			break;
		}
	}

	return true;
}

//____ shiftAndBitsFromMask() _________________________________________________

static inline void shiftAndBitsFromMask( uint64_t mask64, int& shift, int& bits )
{
	shift = 0;
	bits = 0;

	if( mask64 == 0 )
		return;

	while( (mask64 & 0xFF) == 0)
	{
		shift += 8;
		mask64 >>= 8;
	}

	uint32_t mask32 = (uint32_t)mask64;	// It all fits into 32 bits now, so lets be nice to 32-bit CPUs. ;)

	while( (mask32 & 0x1) == 0 )
	{
		shift++;
		mask32 >>= 1;
	}

	while ((mask32 & 0xF) == 0xF)
	{
		bits+=4;
		mask32 >>= 4;
	}

	while( (mask32 & 0x1) == 1 )
	{
		bits++;
		mask32 >>= 1;
	}
}


//____ copyPixels() [PixelDescription] ________________________________________

bool copyPixels(int width, int height, const uint8_t* pSrc, const PixelDescription& srcDesc, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette,
	Color8* pDstPalette, int srcPaletteEntries, int& dstPaletteEntries, int maxDstPaletteEntries)
{
	auto dstDesc = Util::pixelFormatToDescription( dstFmt );

	//TODO: Optimize by calling other copyPixels() if source pixel format is known.


	if (srcDesc.type == PixelType::Bitplanes)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Conversion from bitplanes not supported yet", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}
	else if(srcDesc.type == PixelType::Index)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::IllegalCall, "Conversion from palette indexes not supported yet", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}
	else
	{
		ShiftReadConvTab convTab;

		convTab.maskR = (uint32_t) srcDesc.R_mask;
		convTab.maskG = (uint32_t) srcDesc.G_mask;
		convTab.maskB = (uint32_t) srcDesc.B_mask;
		convTab.maskA = (uint32_t) srcDesc.A_mask;

		convTab.maskR64 = srcDesc.R_mask;
		convTab.maskG64 = srcDesc.G_mask;
		convTab.maskB64 = srcDesc.B_mask;
		convTab.maskA64 = srcDesc.A_mask;

		int bitsR, bitsG, bitsB, bitsA;

		shiftAndBitsFromMask( srcDesc.R_mask, convTab.shiftR, bitsR );
		shiftAndBitsFromMask( srcDesc.G_mask, convTab.shiftG, bitsG );
		shiftAndBitsFromMask( srcDesc.B_mask, convTab.shiftB, bitsB );
		shiftAndBitsFromMask( srcDesc.A_mask, convTab.shiftA, bitsA );

		PixelReadFunc pReadFunc;

		if (srcDesc.bits == 64)
		{
			if (dstDesc.colorSpace == ColorSpace::sRGB)
			{
				if (pConv_16_linear_to_8_sRGB)
					createConv16toSRGBTab();

				convTab.pConvR = pConv_16_linear_to_8_sRGB;
				convTab.pConvG = pConv_16_linear_to_8_sRGB;
				convTab.pConvB = pConv_16_linear_to_8_sRGB;
				convTab.pConvA = pConv_16_linear_to_8_sRGB;
			}
			else
			{
				convTab.pConvR = nullptr;
				convTab.pConvG = nullptr;
				convTab.pConvB = nullptr;
				convTab.pConvA = nullptr;
			}

			if (dstFmt == PixelFormat::BGRA_16_linear)
				pReadFunc = shiftRead_64bit_to64;
			else if (dstDesc.colorSpace == ColorSpace::Linear)
				pReadFunc = shiftRead_64bit;
			else
				pReadFunc = shiftReadConv_64bit;
		}
		else
		{
			const uint8_t** pTabList;

			if (dstDesc.colorSpace == srcDesc.colorSpace)
				pTabList = conv_straight_tabs;
			else if (srcDesc.colorSpace == ColorSpace::Linear)
				pTabList = conv_linear_to_sRGB_tabs;
			else
				pTabList = conv_srgb_to_linear_tabs;

			convTab.pConvR = pTabList[bitsR];
			convTab.pConvG = pTabList[bitsG];
			convTab.pConvB = pTabList[bitsB];
			convTab.pConvA = pTabList[bitsA];

			switch (srcDesc.bits)
			{
			case 8:
				pReadFunc = shiftReadConv_8bit;
				break;
			case 16:
				pReadFunc = (srcDesc.type == PixelType::Chunky_BE) ? shiftReadConv_16bit_swap : shiftReadConv_16bit;
				break;
			case 24:
				pReadFunc = (srcDesc.type == PixelType::Chunky_BE) ? shiftReadConv_24bit_be : shiftReadConv_24bit_le;
				break;
			case 32:
				pReadFunc = (srcDesc.type == PixelType::Chunky_BE) ? shiftReadConv_32bit_swap : shiftReadConv_32bit;
				break;
			default:
				GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Conversion from chunky pixels requires 8, 16, 24 or 32 bit pixels.", nullptr, nullptr, __func__, __FILE__, __LINE__);
				return false;
			}
		}

		return convertPixelsToKnownType(width, height, pSrc, srcDesc.bits, srcPitchAdd, pDst, dstFmt, dstPitchAdd,
										  pDstPalette, dstPaletteEntries, maxDstPaletteEntries, pReadFunc, &convTab, nullptr );
	}
}

//____ getReadFuncFor32bitDest() _____________________________________________

static std::tuple<PixelReadFunc, const void *, const void *, int> getReadFuncFor32bitDest(PixelFormat srcFmt, PixelFormat dstFmt, const Color8* pSrcPalette, int srcPaletteEntries )
{
	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);
	auto& dstDesc = Util::pixelFormatToDescription(dstFmt);

	PixelReadFunc pReadFunc = nullptr;

	bool bLinearSource = (srcDesc.colorSpace == ColorSpace::Linear);
	bool bLinearDest = (dstDesc.colorSpace == ColorSpace::Linear);

	const void* pTab1 = nullptr;
	const void* pTab2 = nullptr;

	int nAllocatedBytes = 0;

	switch (srcDesc.type)
	{
		case PixelType::Index:
			pReadFunc = srcDesc.bits == 8 ? readIndex8 : readIndex16;
			if (srcDesc.colorSpace == dstDesc.colorSpace)
				pTab1 = pSrcPalette;
			else
			{
				auto convTab = srcDesc.colorSpace == ColorSpace::Linear ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;

				nAllocatedBytes = srcPaletteEntries * sizeof(Color8);
				Color8* pConvPalette = (Color8*)GfxBase::memStackAlloc(nAllocatedBytes);
				readConvBGRA8((uint8_t*)pSrcPalette, (uint8_t*)pConvPalette, srcPaletteEntries, convTab, nullptr);
				pTab1 = pConvPalette;
			}
			break;

		case PixelType::Bitplanes:
		{
			const PixelReadFunc readFunc_planes_nonAlpha[8] = {
				readConv_planes_1i_be, readConv_planes_2i_be, nullptr, readConv_planes_4i_be,
				readConv_planes_5i_be, nullptr, nullptr, readConv_planes_8i_be };

			const PixelReadFunc readFunc_planes_withAlpha[8] = {
				readConv_planes_a1_1i_be, readConv_planes_a1_2i_be, nullptr, readConv_planes_a1_4i_be,
				readConv_planes_a1_5i_be, nullptr, nullptr, readConv_planes_a1_8i_be };

			if (srcDesc.A_mask == 0)
				pReadFunc = readFunc_planes_nonAlpha[srcDesc.bits - 1];
			else
				pReadFunc = readFunc_planes_withAlpha[srcDesc.bits - 2];

			if (srcDesc.colorSpace == dstDesc.colorSpace)
				pTab1 = pSrcPalette;
			else
			{
				auto convTab = srcDesc.colorSpace == ColorSpace::Linear ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;

				nAllocatedBytes = srcPaletteEntries * sizeof(Color8);
				Color8* pConvPalette = (Color8*)GfxBase::memStackAlloc(nAllocatedBytes);
				readConvBGRA8((uint8_t*)pSrcPalette, (uint8_t*)pConvPalette, srcPaletteEntries, convTab, nullptr);
				pTab1 = pConvPalette;
			}
			break;
		}


		case PixelType::Chunky_BE:
		{
			if (srcFmt == PixelFormat::RGB_565_bigendian)
			{
				pReadFunc = readRGB_565_bigendian;
				if (bLinearDest)
				{
					pTab1 = conv_5_to_8_straight;
					pTab2 = conv_6_to_8_straight;
				}
				else
				{
					pTab1 = conv_5_linear_to_8_sRGB;
					pTab2 = conv_6_linear_to_8_sRGB;
				}
			}
			else
			{
				pReadFunc = readRGB_555_bigendian;
				if (bLinearDest)
					pTab1 = conv_5_to_8_straight;
				else
					pTab1 = conv_5_linear_to_8_sRGB;
			}
			break;
		}

		case PixelType::Chunky:
		{
			switch (srcFmt)
			{
			case PixelFormat::BGR_8_sRGB:
			case PixelFormat::BGR_8_linear:
				if (srcDesc.colorSpace == dstDesc.colorSpace)
					pReadFunc = readBGR8;
				else
				{
					pReadFunc = readConvBGR8;
					pTab1 = bLinearSource ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;
				}
				break;


			case PixelFormat::BGRX_8_sRGB:
			case PixelFormat::BGRX_8_linear:
				if (srcDesc.colorSpace == dstDesc.colorSpace)
					pReadFunc = readBGRX8;
				else
				{
					pReadFunc = readConvBGRX8;
					pTab1 = bLinearSource ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;
				}
				break;

			case PixelFormat::BGRA_8_sRGB:
			case PixelFormat::BGRA_8_linear:
				if (srcDesc.colorSpace == dstDesc.colorSpace)
					pReadFunc = readBGRA8;
				else
				{
					pReadFunc = readConvBGRA8;
					pTab1 = bLinearSource ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;
				}
				break;

			case PixelFormat::BGRA_4_linear:
				pReadFunc = readBGRA_4;
				if (bLinearDest)
					pTab1 = conv_4_to_8_straight;
				else
					pTab1 = conv_4_linear_to_8_sRGB;

				break;

			case PixelFormat::BGR_565_sRGB:
			case PixelFormat::BGR_565_linear:
				pReadFunc = readBGR_565;
				if (srcDesc.colorSpace == dstDesc.colorSpace)
				{
					pTab1 = conv_5_to_8_straight;
					pTab2 = conv_6_to_8_straight;
				}
				else
				{
					if(bLinearDest)
					{
						pTab1 = conv_5_sRGB_to_8_linear;
						pTab2 = conv_6_sRGB_to_8_linear;
					}
					else
					{
						pTab1 = conv_5_linear_to_8_sRGB;
						pTab2 = conv_6_linear_to_8_sRGB;
					}
				}
				break;

			case PixelFormat::Alpha_8:
				pReadFunc = readAlpha8;
				break;

			case PixelFormat::BGRA_16_linear:
				if (srcDesc.colorSpace == ColorSpace::Linear)
					pReadFunc = readBGRA16;
				else
				{
					if (pConv_16_linear_to_8_sRGB == nullptr)
						createConv16toSRGBTab();

					pReadFunc = readConvBGRA16;
					pTab1 = pConv_16_linear_to_8_sRGB;
				}
				break;


			default:
				break;			// Should never get here.
			}
			break;
		}
	}

	return std::make_tuple(pReadFunc, pTab1, pTab2, nAllocatedBytes);
}

//____ getReadFuncFor64bitDest() _____________________________________________

static std::tuple<PixelReadFunc, const void*, const void*, int> getReadFuncFor64bitDest(PixelFormat srcFmt, const Color8* pSrcPalette, int srcPaletteEntries)
{
	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);

	PixelReadFunc pReadFunc = nullptr;

	bool bLinearSource = (srcDesc.colorSpace == ColorSpace::Linear);

	const void* pTab1 = nullptr;
	const void* pTab2 = nullptr;

	int nAllocatedBytes = 0;

	switch (srcDesc.type)
	{
		case PixelType::Index:
		{
			// Generate a 16-bit palette.

			nAllocatedBytes = srcPaletteEntries * 8;
			uint16_t* pConvPalette = (uint16_t*)GfxBase::memStackAlloc(nAllocatedBytes);

			if (bLinearSource)
				read16_BGRA8_linear((uint8_t*)pSrcPalette, (uint8_t*)pConvPalette, srcPaletteEntries, nullptr, nullptr);
			else
				read16_BGRA8_sRGB((uint8_t*)pSrcPalette, (uint8_t*)pConvPalette, srcPaletteEntries, nullptr, nullptr);

			//

			pReadFunc = srcDesc.bits == 8 ? read16_Index8 : read16_Index16;
			pTab1 = pConvPalette;

			break;
		}

		case PixelType::Bitplanes:
		{
			const PixelReadFunc readFunc_planes_nonAlpha[8] = {
				readConv_planes_1i_be, readConv_planes_2i_be, nullptr, readConv_planes_4i_be,
				readConv_planes_5i_be, nullptr, nullptr, readConv_planes_8i_be };

			const PixelReadFunc readFunc_planes_withAlpha[8] = {
				readConv_planes_a1_1i_be, readConv_planes_a1_2i_be, nullptr, readConv_planes_a1_4i_be,
				readConv_planes_a1_5i_be, nullptr, nullptr, readConv_planes_a1_8i_be };

			if (srcDesc.A_mask == 0)
				pTab2 = (const void*) (readFunc_planes_nonAlpha[srcDesc.bits - 1]);
			else
				pTab2 = (const void*) (readFunc_planes_withAlpha[srcDesc.bits - 2]);

			pTab1 = pSrcPalette;

			pReadFunc = read16_bitplanes;
			break;
		}

		case PixelType::Chunky_BE:
		{
			if (srcFmt == PixelFormat::RGB_565_bigendian)
				pReadFunc = read16_RGB_565_bigendian;
			else
				pReadFunc = read16_RGB_555_bigendian;
			break;
		}

		case PixelType::Chunky:
		{
			switch (srcFmt)
			{
			case PixelFormat::BGR_8_sRGB:
				pReadFunc = read16_BGR8_sRGB;
				break;

			case PixelFormat::BGR_8_linear:
				pReadFunc = read16_BGR8_linear;
				break;

			case PixelFormat::BGRX_8_sRGB:
				pReadFunc = read16_BGRX8_sRGB;
				break;

			case PixelFormat::BGRX_8_linear:
				pReadFunc = read16_BGRX8_linear;
				break;

			case PixelFormat::BGRA_8_sRGB:
				pReadFunc = read16_BGRA8_sRGB;
				break;

			case PixelFormat::BGRA_8_linear:
				pReadFunc = read16_BGRA8_linear;
				break;

			case PixelFormat::BGRA_4_linear:
				pReadFunc = read16_BGRA_4;
				break;

			case PixelFormat::BGR_565_sRGB:
				pReadFunc = read16_BGR_565_sRGB;
				break;

			case PixelFormat::BGR_565_linear:
				pReadFunc = read16_BGR_565_linear;
				break;

			case PixelFormat::Alpha_8:
				pReadFunc = read16_Alpha_8;
				break;

			case PixelFormat::BGRA_16_linear:
				pReadFunc = read16_BGRA16_linear;
				break;

			default:
				assert(false);
				break;			// Should never get here.
			}
			break;
		}
	}

	return std::make_tuple(pReadFunc, pTab1, pTab2, nAllocatedBytes);
}


//____ copyTo32bitDestination() ______________________________________________

static bool copyTo32bitDestination(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette, int srcPaletteEntries)
{
	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);

	PixelReadFunc pReadFunc = nullptr;

	const void* pTab1 = nullptr;
	const void* pTab2 = nullptr;

	int nAllocatedBytes = 0;

	std::tie(pReadFunc, pTab1, pTab2, nAllocatedBytes) = getReadFuncFor32bitDest(srcFmt, dstFmt, pSrcPalette, srcPaletteEntries);
	if (pReadFunc == nullptr)
		return false;

	int srcPitch = width * srcDesc.bits / 8 + srcPitchAdd;
	int dstPitch = width * 32 / 8 + dstPitchAdd;

	for (int y = 0; y < height; y++)
	{
		pReadFunc(pSrc, pDst, width, pTab1, pTab2);
		pSrc += srcPitch;
		pDst += dstPitch;
	}

	GfxBase::memStackFree(nAllocatedBytes);
	return true;
}

//____ copyTo64bitDestination() ______________________________________________

static bool copyTo64bitDestination(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette, int srcPaletteEntries)
{
	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);

	PixelReadFunc pReadFunc = nullptr;

	const void* pTab1 = nullptr;
	const void* pTab2 = nullptr;

	int nAllocatedBytes = 0;

	std::tie(pReadFunc, pTab1, pTab2, nAllocatedBytes) = getReadFuncFor64bitDest(srcFmt, pSrcPalette, srcPaletteEntries );
	if (pReadFunc == nullptr)
		return false;

	int srcPitch = width * srcDesc.bits / 8 + srcPitchAdd;
	int dstPitch = width * 64 / 8 + dstPitchAdd;

	for (int y = 0; y < height; y++)
	{
		pReadFunc(pSrc, pDst, width, pTab1, pTab2);
		pSrc += srcPitch;
		pDst += dstPitch;
	}

	GfxBase::memStackFree(nAllocatedBytes);
	return true;
}

//____ copyToChunkyDestination() ______________________________________________

static bool copyToChunkyDestination(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette, int srcPaletteEntries)
{
	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);
	auto& dstDesc = Util::pixelFormatToDescription(dstFmt);

	if (srcFmt == dstFmt)
	{
		// We can do a straight memcopy

		int bytes = width * srcDesc.bits / 8;

		for (int y = 0; y < height; y++)
		{
			memcpy(pDst, pSrc, bytes);
			pSrc += srcPitchAdd + bytes;
			pDst += dstPitchAdd + bytes;
		}
		return true;
	}
	else if ( (srcDesc.colorSpace == dstDesc.colorSpace || dstFmt == PixelFormat::Alpha_8) && (srcFmt == PixelFormat::BGRA_8_linear || srcFmt == PixelFormat::BGRA_8_sRGB ||
		srcFmt == PixelFormat::BGRX_8_linear || srcFmt == PixelFormat::BGRX_8_sRGB) )
	{
		// We can do a one step process

		PixelWriteFunc pWriteFunc = getChunkyWriteFuncFromBGRA8(dstFmt);

		for (int y = 0; y < height; y++)
		{
			pWriteFunc(pSrc, pDst, width);

			pSrc += srcPitchAdd + width * srcDesc.bits / 8;
			pDst += dstPitchAdd + width * dstDesc.bits / 8;
		}
		return true;
	}
	else
	{
		// We need to make a two-step process

		PixelReadFunc pReadFunc = nullptr;

		const void* pTab1 = nullptr;
		const void* pTab2 = nullptr;

		int nAllocatedBytes = 0;

		std::tie(pReadFunc, pTab1, pTab2, nAllocatedBytes) = getReadFuncFor32bitDest(srcFmt, dstFmt, pSrcPalette, srcPaletteEntries);
		if (pReadFunc == nullptr)
			return false;

		PixelWriteFunc pWriteFunc = getChunkyWriteFuncFromBGRA8(dstFmt);

		return twoStepCopyToChunky(width, height, pSrc, srcDesc.bits, srcPitchAdd, pDst, dstDesc.bits, dstPitchAdd,
			pReadFunc, pTab1, pTab2, pWriteFunc);
	}

}


//____ identicalPalettes() ____________________________________________________

static bool identicalPalettes( const Color8* pPalette1, const Color8* pPalette2, int nEntries )
{
	for( int i = 0 ; i < nEntries ; i++ )
		if( pPalette1[i] != pPalette2[i] )
			return false;

	return true;
}

//____ initPaletteRemapTable() ________________________________________________

static bool	initPaletteRemapTable( uint16_t * pRemapTab, const Color8 * pSrcPalette, int srcPaletteEntries, Color8 * pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries, int width, int height, const uint8_t * pSrcPixels, int srcPitchAdd, int pixelBits )
{

	// Create a "color use mask" to know which colors are actually used

	int allocBytes = srcPaletteEntries*sizeof(bool);
	bool * pColorsInUse = (bool*) GfxBase::memStackAlloc(allocBytes);
	memset( pColorsInUse, 0, srcPaletteEntries );

	if( pixelBits == 8 )
	{
		const uint8_t * p = pSrcPixels;
		for( int y = 0 ; y < height ; y++ )
		{
			for( int x = 0 ; x < width ; x++ )
				pColorsInUse[*p++] = true;
			p += srcPitchAdd;
		}
	}
	else
	{
		const uint16_t * p = (uint16_t*) pSrcPixels;
		for( int y = 0 ; y < height ; y++ )
		{
			for( int x = 0 ; x < width ; x++ )
				pColorsInUse[*p++] = true;
			p += srcPitchAdd/2;
		}
	}

	// Remap colors

	for( int i = 0 ; i < srcPaletteEntries ; i++ )
	{
		if( pColorsInUse[i] )
		{
			Color8 src = pSrcPalette[i];

			int ofs = 0;
			while( ofs < dstPaletteEntries && src != pDstPalette[ofs] )
				ofs++;

			if( ofs == dstPaletteEntries )
			{
				if( dstPaletteEntries == maxDstPaletteEntries )
				{
					GfxBase::memStackFree(allocBytes);
					return false;
				}

				dstPaletteEntries++;
			}
			pRemapTab[i] = ofs;
		}
	}

	// All colors remapped. Cleanup and return.

	GfxBase::memStackFree(allocBytes);
	return true;
}


class ChunkyToIndexConverter
{
public:
	ChunkyToIndexConverter( Color8 * pPalette, int nPaletteEntries, int maxPaletteEntries, int indexBits );
	~ChunkyToIndexConverter() { delete [] m_pTable; }

	bool convert(uint8_t * pDst, const uint32_t * pSrc, int nPixels );

	inline int nbPaletteEntries() const { return m_nPaletteEntries; }

protected:

	bool _convertPixelsToIndex8(uint8_t * pDst, const uint32_t * pSrc, int nPixels );
	bool _convertPixelsToIndex16(uint16_t * pDst, const uint32_t * pSrc, int nPixels );

	Color8 *	m_pPalette;
	int			m_nPaletteEntries;
	int			m_maxPaletteEntries;
	int			m_indexBits;

	struct ChunkyIndex
	{
		uint32_t chunkyValue;
		int		index;
	};

	ChunkyIndex *	m_pTable;
	int				m_nTableEntries;

};

//____ ChunkyToIndexConverter constructor ______________________________________________

ChunkyToIndexConverter::ChunkyToIndexConverter( Color8 * pPalette, int nPaletteEntries, int maxPaletteEntries, int indexBits )
{
	m_pPalette = pPalette;
	m_nPaletteEntries = nPaletteEntries;
	m_maxPaletteEntries = maxPaletteEntries;
	m_indexBits = indexBits;

	m_pTable = new ChunkyIndex[m_maxPaletteEntries];
	m_nTableEntries = nPaletteEntries;

	// Init the table

	uint32_t * pPal = (uint32_t*) pPalette;

	for( int i = 0 ; i < nPaletteEntries ; i++ )
	{
		m_pTable[i].index = i;
		m_pTable[i].chunkyValue = pPal[i];
	}

	std::sort( m_pTable, m_pTable + m_nTableEntries, [](ChunkyIndex& a, ChunkyIndex& b){ return a.chunkyValue > b.chunkyValue; } );
}

//____ convert() ________________________________________________________

bool ChunkyToIndexConverter::convert(uint8_t * pDst, const uint32_t * pSrc, int nPixels )
{
	if( m_indexBits == 8 )
		return _convertPixelsToIndex8(pDst, pSrc, nPixels );
	else
		return _convertPixelsToIndex16( (uint16_t*) pDst, pSrc, nPixels );
}

//____ _convertPixelsToIndex8() ________________________________________________________

bool ChunkyToIndexConverter::_convertPixelsToIndex8(uint8_t * pDst, const uint32_t * pSrc, int nPixels )
{
	for( int i = 0 ; i < nPixels ; i++ )
	{
		uint32_t pixel = * pSrc++;

		int left = 0;
		int right = m_nTableEntries-1;

		if( m_nTableEntries == 0 )
			right = 0;
		else if( pixel < m_pTable[left].chunkyValue )
			right = 0;
		else if( pixel > m_pTable[right].chunkyValue )
			right++;
		else
		{
			while (right - left > 1)
			{
				int mid = left + (right - left) / 2;

				if (m_pTable[mid].chunkyValue < pixel)
				  left = mid;
				else
				  right = mid;
			}

			if(m_pTable[left].chunkyValue == pixel)
			{
				* pDst++ = m_pTable[left].index;
				continue;
			}

			if(m_pTable[right].chunkyValue == pixel)
			{
				* pDst++ = m_pTable[right].index;
				continue;
			}
		}

		if( m_nPaletteEntries == m_maxPaletteEntries )
			return false;										// Out of palette entries!

		if( right < m_nTableEntries )
			memmove(&m_pTable[right+1], &m_pTable[right], sizeof(ChunkyIndex)*(m_nTableEntries-right) );

		m_pTable[right].index = m_nPaletteEntries;
		m_pTable[right].chunkyValue = pixel;
		m_nTableEntries++;

		m_pPalette[m_nPaletteEntries] = pixel;

		* pDst++ = m_nPaletteEntries++;


	}

	return true;
}

//____ _convertPixelsToIndex16() ________________________________________________________

bool ChunkyToIndexConverter::_convertPixelsToIndex16(uint16_t * pDst, const uint32_t * pSrc, int nPixels )
{
	for( int i = 0 ; i < nPixels ; i++ )
	{
		uint32_t pixel = * pSrc++;

		int left = 0;
		int right = m_nTableEntries-1;

		if( m_nTableEntries == 0 )
			right = 0;
		else if( pixel < m_pTable[left].chunkyValue )
			right = 0;
		else if( pixel > m_pTable[right].chunkyValue )
			right++;
		else
		{
			while (right - left > 1)
			{
				int mid = left + (right - left) / 2;

				if (m_pTable[mid].chunkyValue < pixel)
				  left = mid;
				else
				  right = mid;
			}

			if(m_pTable[left].chunkyValue == pixel)
			{
				* pDst++ = m_pTable[left].index;
				continue;
			}

			if(m_pTable[right].chunkyValue == pixel)
			{
				* pDst++ = m_pTable[right].index;
				continue;
			}
		}

		if( m_nPaletteEntries == m_maxPaletteEntries )
			return false;										// Out of palette entries!

		if( right < m_nTableEntries )
			memmove(&m_pTable[right+1], &m_pTable[right], sizeof(ChunkyIndex)*(m_nTableEntries-right) );

		m_pTable[right].index = m_nPaletteEntries;
		m_pTable[right].chunkyValue = pixel;
		m_nTableEntries++;

		m_pPalette[m_nPaletteEntries] = pixel;

		* pDst++ = m_nPaletteEntries++;


	}

	return true;
}


//____ copyToIndexedDestination() _____________________________________________

static bool copyToIndexedDestination(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette,
	Color8* pDstPalette, int srcPaletteEntries, int& dstPaletteEntries, int maxDstPaletteEntries)
{

	auto& srcDesc = Util::pixelFormatToDescription(srcFmt);
	auto& dstDesc = Util::pixelFormatToDescription(dstFmt);

	if (srcDesc.type == PixelType::Index)
	{
		int allocSize1 = 0;

		// Make sure source palette is in same color space as
		// destination palette

		if( srcDesc.colorSpace != dstDesc.colorSpace )
		{
			// Palettes have different color space. We need
			// to convert source palette before proceeding.

			allocSize1 = srcPaletteEntries*sizeof(Color8);

			auto pFixedPalette = (Color8*) GfxBase::memStackAlloc(allocSize1);

			auto pConvTab = srcDesc.colorSpace == ColorSpace::Linear ? conv_8_linear_to_8_sRGB : conv_8_sRGB_to_8_linear;

			for( int i = 0 ; i < srcPaletteEntries ; i++ )
			{
				pFixedPalette[i].r = pConvTab[pSrcPalette[i].r];
				pFixedPalette[i].g = pConvTab[pSrcPalette[i].g];
				pFixedPalette[i].b = pConvTab[pSrcPalette[i].b];
				pFixedPalette[i].a = pSrcPalette[i].a;
			}

			pSrcPalette = pFixedPalette;
		}

		// Check if we can do a fast, straight pixel copy.

		if( (srcDesc.type == PixelType::Index && srcDesc.bits == dstDesc.bits) && (srcPaletteEntries <= dstPaletteEntries) && identicalPalettes( pSrcPalette, pDstPalette, srcPaletteEntries ) )
		{
			// We can do a straight pixel copy.

			int bytes = width * srcDesc.bits / 8;

			for (int y = 0; y < height; y++)
			{
				memcpy(pDst, pSrc, bytes);
				pSrc += srcPitchAdd + bytes;
				pDst += dstPitchAdd + bytes;
			}

			GfxBase::memStackFree(allocSize1);
			return true;
		}

		// We need to remap colors and possibly add to dest palette.
		// Start by creating a palette remap tab and add colors to destination palette as needed

		int allocSize2 = srcPaletteEntries*sizeof(uint16_t);
		auto pRemapTab = (uint16_t*) GfxBase::memStackAlloc(allocSize2);

		if( !initPaletteRemapTable( pRemapTab, pSrcPalette, srcPaletteEntries, pDstPalette, dstPaletteEntries, maxDstPaletteEntries, width, height, pSrc, srcPitchAdd, srcDesc.bits ) )
		{
			// Not enough/right colors in destination palette.

			GfxBase::memStackFree(allocSize2);
			GfxBase::memStackFree(allocSize1);
			return false;
		}

		// Copy pixels and remap indexes.

		if( srcFmt == PixelFormat::Index_8 )
		{
			if( dstFmt == PixelFormat::Index_8 )
			{
				for( int y = 0 ; y < height ; y++ )
				{
					for( int x = 0 ; x < width ; x++ )
						* pDst++ = pRemapTab[*pSrc++];

					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				uint16_t * pDst2 = (uint16_t*) pDst;
				for( int y = 0 ; y < height ; y++ )
				{
					for( int x = 0 ; x < width ; x++ )
						* pDst2++ = pRemapTab[*pSrc++];

					pSrc += srcPitchAdd;
					pDst2 += dstPitchAdd/2;
				}
			}
		}
		else
		{
			uint16_t * pSrc2 = (uint16_t*) pSrc;

			if( dstFmt == PixelFormat::Index_8 )
			{
				for( int y = 0 ; y < height ; y++ )
				{
					for( int x = 0 ; x < width ; x++ )
						* pDst++ = pRemapTab[*pSrc2++];

					pSrc2 += srcPitchAdd/2;
					pDst += dstPitchAdd;
				}
			}
			else
			{
				uint16_t * pDst2 = (uint16_t*) pDst;
				for( int y = 0 ; y < height ; y++ )
				{
					for( int x = 0 ; x < width ; x++ )
						* pDst2++ = pRemapTab[*pSrc2++];

					pSrc2 += srcPitchAdd/2;
					pDst2 += dstPitchAdd/2;
				}
			}
		}

		return true;
	}
	else
	{
		PixelReadFunc pReadFunc = nullptr;

		const void* pTab1 = nullptr;
		const void* pTab2 = nullptr;

		int nAllocatedBytes = 0;

		std::tie(pReadFunc, pTab1, pTab2, nAllocatedBytes) = getReadFuncFor32bitDest(srcFmt, dstFmt, pSrcPalette, srcPaletteEntries);
		if (pReadFunc == nullptr)
			return false;

		ChunkyToIndexConverter conv(pDstPalette, dstPaletteEntries, maxDstPaletteEntries, dstDesc.bits );

		uint8_t	buffer[64 * 4];

		for (int y = 0; y < height; y++)
		{
			int x = 0;
			int widthLeft = width;
			while (widthLeft > 64)
			{
				pReadFunc(pSrc, buffer, 64, pTab1, pTab2);
				pSrc += srcDesc.bits * 8;

				if( !conv.convert(pDst, (uint32_t*) buffer, 64) )
					return false;

				pDst += dstDesc.bits * 8;

				widthLeft -= 64;
			}

			if (widthLeft > 0)
			{
				pReadFunc(pSrc, buffer, widthLeft, pTab1, pTab2);
				pSrc += srcDesc.bits / 8 * widthLeft;

				if( !conv.convert(pDst, (uint32_t*) buffer, widthLeft) )
					return false;

				pDst += dstDesc.bits / 8 * widthLeft;
			}

			pSrc += srcPitchAdd;
			pDst += dstPitchAdd;
		}

		dstPaletteEntries = conv.nbPaletteEntries();
		return true;
	}

}


//____ copyPixels() [PixelFormat] _____________________________________________

bool copyPixels(int width, int height, const uint8_t* pSrc, PixelFormat srcFmt, int srcPitchAdd,
	uint8_t* pDst, PixelFormat dstFmt, int dstPitchAdd, const Color8* pSrcPalette,
	Color8* pDstPalette, int srcPaletteEntries, int& dstPaletteEntries, int maxDstPaletteEntries)
{

	srcFmt = Util::clarifyPixelFormat(srcFmt);
	dstFmt = Util::clarifyPixelFormat(dstFmt);

	if (dstFmt == PixelFormat::BGRA_8_linear || dstFmt == PixelFormat::BGRA_8_sRGB || dstFmt == PixelFormat::BGRX_8_linear || dstFmt == PixelFormat::BGRX_8_sRGB)
		return copyTo32bitDestination(width, height, pSrc, srcFmt, srcPitchAdd, pDst, dstFmt, dstPitchAdd, pSrcPalette, srcPaletteEntries);
	else if (dstFmt == PixelFormat::BGRX_16_linear)
		return copyTo64bitDestination(width, height, pSrc, srcFmt, srcPitchAdd, pDst, dstFmt, dstPitchAdd, pSrcPalette, srcPaletteEntries);
	else
	{
		auto& dstDesc = Util::pixelFormatToDescription(dstFmt);

		switch (dstDesc.type)
		{
			case PixelType::Bitplanes:
				return false;				// Not supported yet!

			case PixelType::Chunky:
			case PixelType::Chunky_BE:
				return copyToChunkyDestination(width, height, pSrc, srcFmt, srcPitchAdd, pDst, dstFmt, dstPitchAdd, pSrcPalette, srcPaletteEntries);

			case PixelType::Index:
				return copyToIndexedDestination(width, height, pSrc, srcFmt, srcPitchAdd, pDst, dstFmt, dstPitchAdd, pSrcPalette,
												pDstPalette, srcPaletteEntries, dstPaletteEntries, maxDstPaletteEntries);
		}
	}
	return false;
}

//____ colorToPixelBytes() ____________________________________________________

int colorToPixelBytes( HiColor color, PixelFormat format, uint8_t pixelArea[18], Color8* pPalette, int paletteSize )
{
	auto& desc = Util::pixelFormatToDescription(format);

	if (desc.type == PixelType::Bitplanes)
	{
		HiColor alphaLess = color;
		alphaLess.a = 0xFF;
		int colorNb = findBestMatchInPalette(alphaLess, pPalette, paletteSize, ColorSpace::Linear);

		uint16_t* pOutput = (uint16_t*) pixelArea;
		int mask = colorNb ^ 0xFFFF;

		int alphaBits = int(desc.A_mask);
		int colorBits = desc.bits - alphaBits;

		for (int i = 0; i < colorBits; i++)
		{
			* pOutput++ = (uint16_t)((mask & 0x1) - 1);
			mask >>= 1;
		}

		if( alphaBits )
			* pOutput++ = color.a >= 2048 ? 0xFFFF : 0x0000;

		return desc.bits*2;
	}
	else
	{
		uint8_t* pConvTab;
		if (desc.colorSpace == ColorSpace::Undefined )
			pConvTab = GfxBase::defaultToSRGB() ? HiColor::packSRGBTab : HiColor::packLinearTab;
		else if(desc.colorSpace == ColorSpace::Linear )
			pConvTab = HiColor::packLinearTab;
		else
			pConvTab = HiColor::packSRGBTab;

		switch (format)
		{
		case PixelFormat::BGR_8:
		case PixelFormat::BGR_8_sRGB:
		case PixelFormat::BGR_8_linear:
		{
			pixelArea[0] = pConvTab[color.b];
			pixelArea[1] = pConvTab[color.g];
			pixelArea[2] = pConvTab[color.r];
			return 3;
		}


		case PixelFormat::BGRX_8:
		case PixelFormat::BGRX_8_sRGB:
		case PixelFormat::BGRX_8_linear:
		{
			pixelArea[0] = pConvTab[color.b];
			pixelArea[1] = pConvTab[color.g];
			pixelArea[2] = pConvTab[color.r];
			pixelArea[3] = 0;
			return 4;
		}


		case PixelFormat::BGRA_8:
		case PixelFormat::BGRA_8_sRGB:
		case PixelFormat::BGRA_8_linear:
		{
			pixelArea[0] = pConvTab[color.b];
			pixelArea[1] = pConvTab[color.g];
			pixelArea[2] = pConvTab[color.r];
			pixelArea[3] = HiColor::packLinearTab[color.a];
			return 4;
		}

		case PixelFormat::BGRA_4_linear:
		{
			int b = pConvTab[color.b];
			int g = pConvTab[color.g];
			int r = pConvTab[color.r];
			int a = pConvTab[color.a];

			uint16_t pixel = ((b & 0xF0) << 8) | ((g & 0xF0) << 4) | (r & 0xF0) | (a >> 4);
			*(uint16_t*)pixelArea = pixel;
			return 2;
		}

		case PixelFormat::BGR_565:
		case PixelFormat::BGR_565_sRGB:
		case PixelFormat::BGR_565_linear:
		{
			int b = pConvTab[color.b];
			int g = pConvTab[color.g];
			int r = pConvTab[color.r];

			uint16_t pixel = ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
			*(uint16_t*)pixelArea = pixel;
			return 2;
		}

		case PixelFormat::Index_8:
		case PixelFormat::Index_8_sRGB:
		case PixelFormat::Index_8_linear:
		{
			pixelArea[0] = (uint8_t)findBestMatchInPalette(color, pPalette, paletteSize, desc.colorSpace);
			return 1;
		}

		case PixelFormat::Index_16:
		case PixelFormat::Index_16_sRGB:
		case PixelFormat::Index_16_linear:
		{
			*reinterpret_cast<uint16_t*>(pixelArea) = (uint16_t)findBestMatchInPalette(color, pPalette, paletteSize, desc.colorSpace);
			return 2;
		}

		case PixelFormat::RGB_565_bigendian:
		{
			int b = pConvTab[color.b];
			int g = pConvTab[color.g];
			int r = pConvTab[color.r];

			uint16_t pixel = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			*(uint16_t*)pixelArea = Util::endianSwap(pixel);
			return 2;
		}

		case PixelFormat::RGB_555_bigendian:
		{
			int b = pConvTab[color.b];
			int g = pConvTab[color.g];
			int r = pConvTab[color.r];

			uint16_t pixel = ((r & 0xF8) << 8) | ((g & 0xF8) << 3) | (b >> 3);
			*(uint16_t*)pixelArea = Util::endianSwap(pixel);
			return 2;
		}

		case PixelFormat::Alpha_8:
			pixelArea[0] = HiColor::packLinearTab[color.a];
			return 1;

		case PixelFormat::BGRA_16_linear:
		{
			uint16_t* pOutput = (uint16_t*)pixelArea;

			*pOutput++ = (color.b * 65535) / 4096;
			*pOutput++ = (color.g * 65535) / 4096;
			*pOutput++ = (color.r * 65535) / 4096;
			*pOutput++ = (color.a * 65535) / 4096;
			return 8;
		}

		default:
			return 0;
		}

	}

}



//____ fillBitmap() ___________________________________________________________

void fillBitmap(uint8_t* pBitmap, PixelFormat pixelFormat, int pitch, RectI fillRect, HiColor color, Color8* pPalette, int paletteSize)
{
	pixelFormat = Util::clarifyPixelFormat(pixelFormat);

	auto&	pixelDesc = Util::pixelFormatToDescription(pixelFormat);

	uint8_t pixelArea[18];
	int pixelBytes = colorToPixelBytes(color, pixelFormat, pixelArea, pPalette, paletteSize);

	if( pixelDesc.type == PixelType::Bitplanes )
	{
		int pixelWords = pixelBytes / 2;

		uint16_t leftPixelMask = 0xFFFF >> (fillRect.x & 0xF);
		uint16_t rightPixelMask = 0xFFFF << (15 - ((fillRect.x + fillRect.w) & 0xF));

		int wordWidth = 1 + (((fillRect.x + fillRect.w) & 0xF) - (fillRect.x & 0xF)) / 16;


		uint8_t * pDest = pBitmap + fillRect.y * pitch + (pixelDesc.bits * 2 * (fillRect.x / 16));
		int eolAdd = pitch - wordWidth*pixelDesc.bits*2;

		uint16_t* wordArea = (uint16_t*)pixelArea;

		if (wordWidth == 1)
		{
			uint16_t pixelMask = leftPixelMask & rightPixelMask;
			uint16_t invMask = ~pixelMask;

			for (int y = 0; y < fillRect.h; y++)
			{
				for (int plane = 0; plane < pixelWords; plane++)
				{
					*pDest = ((*pDest) & invMask) | (wordArea[plane] & pixelMask);
					pDest++;
				}
				pDest += eolAdd;
			}
		}
		else
		{
			uint16_t invLeftMask = ~leftPixelMask;
			uint16_t invRightMask = ~rightPixelMask;

			for (int y = 0; y < fillRect.h; y++)
			{
				for (int plane = 0; plane < pixelWords; plane++)
				{
					*pDest = ((*pDest) & invLeftMask) | (wordArea[plane] & leftPixelMask);
					pDest++;
				}

				for( int x = 0 ; x < (wordWidth - 2) ; x++ )
					for (int plane = 0; plane < pixelWords; plane++)
					{
						*pDest = wordArea[plane];
						pDest++;
					}

				for (int plane = 0; plane < pixelWords; plane++)
				{
					*pDest = ((*pDest) & invRightMask) | (wordArea[plane] & rightPixelMask);
					pDest++;
				}

				pDest += eolAdd;
			}
		}
	}
	else
	{
		uint8_t * pDest = pBitmap + fillRect.y*pitch + fillRect.x*pixelBytes;
		int eolAdd = pitch - fillRect.w * pixelBytes;

		switch(pixelBytes)
		{
			case 1:
			{
				uint8_t pixel = pixelArea[0];

				for( int y = 0 ; y < fillRect.h ; y++ )
				{
					for( int x = 0 ; x < fillRect.w ; x++ )
						* pDest++ = pixel;
					pDest += eolAdd;
				}
				break;
			}

			case 2:
			{
				uint16_t pixel = * (uint16_t*) pixelArea;

				for( int y = 0 ; y < fillRect.h ; y++ )
				{
					for( int x = 0 ; x < fillRect.w ; x++ )
					{
						* ((uint16_t*)pDest) = pixel;
						pDest += 2;
					}
					pDest += eolAdd;
				}
				break;
			}

			case 3:
			{
				for( int y = 0 ; y < fillRect.h ; y++ )
				{
					for( int x = 0 ; x < fillRect.w ; x++ )
					{
						* pDest++ = pixelArea[0];
						* pDest++ = pixelArea[1];
						* pDest++ = pixelArea[2];
					}
					pDest += eolAdd;
				}
				break;
			}

			case 4:
			{
				uint32_t pixel = * (uint32_t*) pixelArea;

				for( int y = 0 ; y < fillRect.h ; y++ )
				{
					for( int x = 0 ; x < fillRect.w ; x++ )
					{
						* ((uint32_t*)pDest) = pixel;
						pDest += 4;
					}
					pDest += eolAdd;
				}
				break;
			}

			case 8:
			{
				uint64_t pixel = *(uint64_t*)pixelArea;

				for (int y = 0; y < fillRect.h; y++)
				{
					for (int x = 0; x < fillRect.w; x++)
					{
						*((uint64_t*)pDest) = pixel;
						pDest += 8;
					}
					pDest += eolAdd;
				}
				break;
			}
		}
	}
}

//____ findBestMatchInPalette() ____________________________________________

int findBestMatchInPalette(HiColor color, Color8* pPalette, int paletteSize, ColorSpace paletteColorSpace)
{
	uint8_t* pConvTab;
	if (paletteColorSpace == ColorSpace::Undefined)
		pConvTab = GfxBase::defaultToSRGB() ? HiColor::packSRGBTab : HiColor::packLinearTab;
	else if (paletteColorSpace == ColorSpace::Linear)
		pConvTab = HiColor::packLinearTab;
	else
		pConvTab = HiColor::packSRGBTab;

	Color8 col;

	col.b = pConvTab[color.b];
	col.g = pConvTab[color.b];
	col.r = pConvTab[color.b];
	col.a = pConvTab[color.b];

	// First we make a quick check for exact match

	for( int i = 0 ; i < paletteSize ; i++ )
		if( col == pPalette[i] )
			return i;

	// Find best match

	int bestMatchIndex = 0;
	int bestMatchDiff = 256*256*4;

	for( int i = 0 ; i < paletteSize ; i++ )
	{
		Color8& palCol = pPalette[i];

		int diffB = abs(col.b - palCol.b);
		int diffG = abs(col.g - palCol.g);
		int diffR = abs(col.r - palCol.r);
		int diffA = abs(col.a - palCol.a);

		int combDiff = diffB*diffB + diffG*diffG + diffR*diffR + diffA*diffA;
		if(combDiff < bestMatchDiff)
		{
			bestMatchDiff = combDiff;
			bestMatchIndex = i;
		}
	}

	return bestMatchIndex;
}

//____ extractAlphaChannel() __________________________________________________

bool extractAlphaChannel(PixelFormat format, const uint8_t* pSrc, int srcPitch, RectI srcRect, uint8_t* pDst, int dstPitch, const Color8* pPalette)
{

	auto& pixDesc = Util::pixelFormatToDescription(format);

	if (pixDesc.A_mask == 0 && pixDesc.type != PixelType::Index)
		return false;										// These pixels have no alpha.

	pSrc += srcRect.y * srcPitch + srcRect.x * pixDesc.bits/8;

	int srcPitchAdd = srcPitch - srcRect.w * pixDesc.bits/8;	//TODO: Won't work for bitplane graphics.
	int dstPitchAdd = dstPitch - srcRect.w;

	switch (pixDesc.type)
	{
		case PixelType::Index:
		{
			if (pixDesc.bits == 8)
			{
				for (int y = 0; y < srcRect.h; y++)
				{
					for (int x = 0; x < srcRect.w; x++)
						*pDst++ = pPalette[*pSrc++].a;

					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			else if (pixDesc.bits == 16)
			{
				for (int y = 0; y < srcRect.h; y++)
				{
					for (int x = 0; x < srcRect.w; x++)
					{
						*pDst++ = pPalette[*(uint16_t*)pSrc].a;
						pSrc += 2;
					}

					pSrc += srcPitchAdd;
					pDst += dstPitchAdd;
				}
			}
			return true;
		}

		case PixelType::Chunky:
		{
			switch (pixDesc.bits)
			{
				case 8:									// Only PixelFormat::Alpha_8 has this size.
					for (int y = 0; y < srcRect.h; y++)
					{
						for (int x = 0; x < srcRect.w; x++)
							*pDst++ = *pSrc++;

						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}

				case 16:									// Only PixelFormat::BGRA_4 has this size and alpha.
				{
					for (int y = 0; y < srcRect.h; y++)
					{
						for (int x = 0; x < srcRect.w; x++)
						{
							uint16_t val = (* ((uint16_t*)pSrc)) >> 12;
							*pDst++ = val | (val << 4);
							pSrc += 2;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
					break;
				}
				case 32:									// Only PixelFormat::BGRA_8 has this size and alpha
				{
					for (int y = 0; y < srcRect.h; y++)
					{
						for (int x = 0; x < srcRect.w; x++)
						{
							*pDst++ = pSrc[3];
							pSrc += 4;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
					break;
				}
				case 64:									// Only PixelFormat::BGRA_16 has this size and alpha
				{
					for (int y = 0; y < srcRect.h; y++)
					{
						for (int x = 0; x < srcRect.w; x++)
						{
							*pDst++ = pSrc[7];
							pSrc += 8;
						}
						pSrc += srcPitchAdd;
						pDst += dstPitchAdd;
					}
					break;
				}
			}

		}

		default:
			return false;		// PixelFormat not supported yet.
	}
}



} } // namespace wg::PixelTool

