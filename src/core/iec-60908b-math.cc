/***************************************************************************
 *   Copyright (C) 2022 PCSX-Redux authors                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 ***************************************************************************/

#include "core/iec-60908b-math.h"

#include <stdint.h>

/*

// Building those tables is done with this code:

uint8_t m_ecc_f_lut[256];
uint8_t m_ecc_b_lut[256];
uint32_t m_edc_lut[256];

void build() {
    size_t i;
    for (i = 0; i < 256; i++) {
        uint32_t edc = i;
        size_t j = (i << 1) ^ (i & 0x80 ? 0x11d : 0);
        m_ecc_f_lut[i] = j;
        m_ecc_b_lut[i ^ j] = i;
        for (j = 0; j < 8; j++) {
            edc = (edc >> 1) ^ (edc & 1 ? 0xd8018001 : 0);
        }
        m_edc_lut[i] = edc;
    }
}

*/

static constexpr uint8_t c_ecc_f_lut[] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,  //
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e,  //
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e,  //
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e,  //
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e,  //
    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe,  //
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde,  //
    0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe,  //
    0x1d, 0x1f, 0x19, 0x1b, 0x15, 0x17, 0x11, 0x13, 0x0d, 0x0f, 0x09, 0x0b, 0x05, 0x07, 0x01, 0x03,  //
    0x3d, 0x3f, 0x39, 0x3b, 0x35, 0x37, 0x31, 0x33, 0x2d, 0x2f, 0x29, 0x2b, 0x25, 0x27, 0x21, 0x23,  //
    0x5d, 0x5f, 0x59, 0x5b, 0x55, 0x57, 0x51, 0x53, 0x4d, 0x4f, 0x49, 0x4b, 0x45, 0x47, 0x41, 0x43,  //
    0x7d, 0x7f, 0x79, 0x7b, 0x75, 0x77, 0x71, 0x73, 0x6d, 0x6f, 0x69, 0x6b, 0x65, 0x67, 0x61, 0x63,  //
    0x9d, 0x9f, 0x99, 0x9b, 0x95, 0x97, 0x91, 0x93, 0x8d, 0x8f, 0x89, 0x8b, 0x85, 0x87, 0x81, 0x83,  //
    0xbd, 0xbf, 0xb9, 0xbb, 0xb5, 0xb7, 0xb1, 0xb3, 0xad, 0xaf, 0xa9, 0xab, 0xa5, 0xa7, 0xa1, 0xa3,  //
    0xdd, 0xdf, 0xd9, 0xdb, 0xd5, 0xd7, 0xd1, 0xd3, 0xcd, 0xcf, 0xc9, 0xcb, 0xc5, 0xc7, 0xc1, 0xc3,  //
    0xfd, 0xff, 0xf9, 0xfb, 0xf5, 0xf7, 0xf1, 0xf3, 0xed, 0xef, 0xe9, 0xeb, 0xe5, 0xe7, 0xe1, 0xe3,  //
};

static constexpr uint8_t c_ecc_b_lut[] = {
    0x00, 0xf4, 0xf5, 0x01, 0xf7, 0x03, 0x02, 0xf6, 0xf3, 0x07, 0x06, 0xf2, 0x04, 0xf0, 0xf1, 0x05,  //
    0xfb, 0x0f, 0x0e, 0xfa, 0x0c, 0xf8, 0xf9, 0x0d, 0x08, 0xfc, 0xfd, 0x09, 0xff, 0x0b, 0x0a, 0xfe,  //
    0xeb, 0x1f, 0x1e, 0xea, 0x1c, 0xe8, 0xe9, 0x1d, 0x18, 0xec, 0xed, 0x19, 0xef, 0x1b, 0x1a, 0xee,  //
    0x10, 0xe4, 0xe5, 0x11, 0xe7, 0x13, 0x12, 0xe6, 0xe3, 0x17, 0x16, 0xe2, 0x14, 0xe0, 0xe1, 0x15,  //
    0xcb, 0x3f, 0x3e, 0xca, 0x3c, 0xc8, 0xc9, 0x3d, 0x38, 0xcc, 0xcd, 0x39, 0xcf, 0x3b, 0x3a, 0xce,  //
    0x30, 0xc4, 0xc5, 0x31, 0xc7, 0x33, 0x32, 0xc6, 0xc3, 0x37, 0x36, 0xc2, 0x34, 0xc0, 0xc1, 0x35,  //
    0x20, 0xd4, 0xd5, 0x21, 0xd7, 0x23, 0x22, 0xd6, 0xd3, 0x27, 0x26, 0xd2, 0x24, 0xd0, 0xd1, 0x25,  //
    0xdb, 0x2f, 0x2e, 0xda, 0x2c, 0xd8, 0xd9, 0x2d, 0x28, 0xdc, 0xdd, 0x29, 0xdf, 0x2b, 0x2a, 0xde,  //
    0x8b, 0x7f, 0x7e, 0x8a, 0x7c, 0x88, 0x89, 0x7d, 0x78, 0x8c, 0x8d, 0x79, 0x8f, 0x7b, 0x7a, 0x8e,  //
    0x70, 0x84, 0x85, 0x71, 0x87, 0x73, 0x72, 0x86, 0x83, 0x77, 0x76, 0x82, 0x74, 0x80, 0x81, 0x75,  //
    0x60, 0x94, 0x95, 0x61, 0x97, 0x63, 0x62, 0x96, 0x93, 0x67, 0x66, 0x92, 0x64, 0x90, 0x91, 0x65,  //
    0x9b, 0x6f, 0x6e, 0x9a, 0x6c, 0x98, 0x99, 0x6d, 0x68, 0x9c, 0x9d, 0x69, 0x9f, 0x6b, 0x6a, 0x9e,  //
    0x40, 0xb4, 0xb5, 0x41, 0xb7, 0x43, 0x42, 0xb6, 0xb3, 0x47, 0x46, 0xb2, 0x44, 0xb0, 0xb1, 0x45,  //
    0xbb, 0x4f, 0x4e, 0xba, 0x4c, 0xb8, 0xb9, 0x4d, 0x48, 0xbc, 0xbd, 0x49, 0xbf, 0x4b, 0x4a, 0xbe,  //
    0xab, 0x5f, 0x5e, 0xaa, 0x5c, 0xa8, 0xa9, 0x5d, 0x58, 0xac, 0xad, 0x59, 0xaf, 0x5b, 0x5a, 0xae,  //
    0x50, 0xa4, 0xa5, 0x51, 0xa7, 0x53, 0x52, 0xa6, 0xa3, 0x57, 0x56, 0xa2, 0x54, 0xa0, 0xa1, 0x55,  //
};

static constexpr uint32_t c_edc_lut[] = {
    0x00000000, 0x90910101, 0x91210201, 0x01b00300, 0x92410401, 0x02d00500, 0x03600600, 0x93f10701,  //
    0x94810801, 0x04100900, 0x05a00a00, 0x95310b01, 0x06c00c00, 0x96510d01, 0x97e10e01, 0x07700f00,  //
    0x99011001, 0x09901100, 0x08201200, 0x98b11301, 0x0b401400, 0x9bd11501, 0x9a611601, 0x0af01700,  //
    0x0d801800, 0x9d111901, 0x9ca11a01, 0x0c301b00, 0x9fc11c01, 0x0f501d00, 0x0ee01e00, 0x9e711f01,  //
    0x82012001, 0x12902100, 0x13202200, 0x83b12301, 0x10402400, 0x80d12501, 0x81612601, 0x11f02700,  //
    0x16802800, 0x86112901, 0x87a12a01, 0x17302b00, 0x84c12c01, 0x14502d00, 0x15e02e00, 0x85712f01,  //
    0x1b003000, 0x8b913101, 0x8a213201, 0x1ab03300, 0x89413401, 0x19d03500, 0x18603600, 0x88f13701,  //
    0x8f813801, 0x1f103900, 0x1ea03a00, 0x8e313b01, 0x1dc03c00, 0x8d513d01, 0x8ce13e01, 0x1c703f00,  //
    0xb4014001, 0x24904100, 0x25204200, 0xb5b14301, 0x26404400, 0xb6d14501, 0xb7614601, 0x27f04700,  //
    0x20804800, 0xb0114901, 0xb1a14a01, 0x21304b00, 0xb2c14c01, 0x22504d00, 0x23e04e00, 0xb3714f01,  //
    0x2d005000, 0xbd915101, 0xbc215201, 0x2cb05300, 0xbf415401, 0x2fd05500, 0x2e605600, 0xbef15701,  //
    0xb9815801, 0x29105900, 0x28a05a00, 0xb8315b01, 0x2bc05c00, 0xbb515d01, 0xbae15e01, 0x2a705f00,  //
    0x36006000, 0xa6916101, 0xa7216201, 0x37b06300, 0xa4416401, 0x34d06500, 0x35606600, 0xa5f16701,  //
    0xa2816801, 0x32106900, 0x33a06a00, 0xa3316b01, 0x30c06c00, 0xa0516d01, 0xa1e16e01, 0x31706f00,  //
    0xaf017001, 0x3f907100, 0x3e207200, 0xaeb17301, 0x3d407400, 0xadd17501, 0xac617601, 0x3cf07700,  //
    0x3b807800, 0xab117901, 0xaaa17a01, 0x3a307b00, 0xa9c17c01, 0x39507d00, 0x38e07e00, 0xa8717f01,  //
    0xd8018001, 0x48908100, 0x49208200, 0xd9b18301, 0x4a408400, 0xdad18501, 0xdb618601, 0x4bf08700,  //
    0x4c808800, 0xdc118901, 0xdda18a01, 0x4d308b00, 0xdec18c01, 0x4e508d00, 0x4fe08e00, 0xdf718f01,  //
    0x41009000, 0xd1919101, 0xd0219201, 0x40b09300, 0xd3419401, 0x43d09500, 0x42609600, 0xd2f19701,  //
    0xd5819801, 0x45109900, 0x44a09a00, 0xd4319b01, 0x47c09c00, 0xd7519d01, 0xd6e19e01, 0x46709f00,  //
    0x5a00a000, 0xca91a101, 0xcb21a201, 0x5bb0a300, 0xc841a401, 0x58d0a500, 0x5960a600, 0xc9f1a701,  //
    0xce81a801, 0x5e10a900, 0x5fa0aa00, 0xcf31ab01, 0x5cc0ac00, 0xcc51ad01, 0xcde1ae01, 0x5d70af00,  //
    0xc301b001, 0x5390b100, 0x5220b200, 0xc2b1b301, 0x5140b400, 0xc1d1b501, 0xc061b601, 0x50f0b700,  //
    0x5780b800, 0xc711b901, 0xc6a1ba01, 0x5630bb00, 0xc5c1bc01, 0x5550bd00, 0x54e0be00, 0xc471bf01,  //
    0x6c00c000, 0xfc91c101, 0xfd21c201, 0x6db0c300, 0xfe41c401, 0x6ed0c500, 0x6f60c600, 0xfff1c701,  //
    0xf881c801, 0x6810c900, 0x69a0ca00, 0xf931cb01, 0x6ac0cc00, 0xfa51cd01, 0xfbe1ce01, 0x6b70cf00,  //
    0xf501d001, 0x6590d100, 0x6420d200, 0xf4b1d301, 0x6740d400, 0xf7d1d501, 0xf661d601, 0x66f0d700,  //
    0x6180d800, 0xf111d901, 0xf0a1da01, 0x6030db00, 0xf3c1dc01, 0x6350dd00, 0x62e0de00, 0xf271df01,  //
    0xee01e001, 0x7e90e100, 0x7f20e200, 0xefb1e301, 0x7c40e400, 0xecd1e501, 0xed61e601, 0x7df0e700,  //
    0x7a80e800, 0xea11e901, 0xeba1ea01, 0x7b30eb00, 0xe8c1ec01, 0x7850ed00, 0x79e0ee00, 0xe971ef01,  //
    0x7700f000, 0xe791f101, 0xe621f201, 0x76b0f300, 0xe541f401, 0x75d0f500, 0x7460f600, 0xe4f1f701,  //
    0xe381f801, 0x7310f900, 0x72a0fa00, 0xe231fb01, 0x71c0fc00, 0xe151fd01, 0xe0e1fe01, 0x7070ff00,  //
};

// Write ECC block (either P or Q)
static void eccWritePQ(const uint8_t *address, const uint8_t *data, size_t major_count, size_t minor_count,
                       size_t major_mult, size_t minor_inc, uint8_t *ecc) {
    size_t size = major_count * minor_count;
    for (size_t major = 0; major < major_count; major++) {
        size_t index = (major >> 1) * major_mult + (major & 1);
        uint8_t ecc_a = 0;
        uint8_t ecc_b = 0;
        for (size_t minor = 0; minor < minor_count; minor++) {
            uint8_t temp;
            if (index < 4) {
                temp = address[index];
            } else {
                temp = data[index - 4];
            }
            index += minor_inc;
            if (index >= size) index -= size;

            ecc_a ^= temp;
            ecc_b ^= temp;
            ecc_a = c_ecc_f_lut[ecc_a];
        }
        ecc_a = c_ecc_b_lut[c_ecc_f_lut[ecc_a] ^ ecc_b];
        ecc[major] = ecc_a;
        ecc[major + major_count] = ecc_a ^ ecc_b;
    }
}

void PCSX::IEC60908b::computeECC(const uint8_t *address, const uint8_t *data, uint8_t *ecc) {
    eccWritePQ(address, data, 86, 24, 2, 86, ecc);          // P
    eccWritePQ(address, data, 52, 43, 86, 88, ecc + 0xac);  // Q
}

uint32_t PCSX::IEC60908b::computeEDC(uint32_t edc, const uint8_t *src, size_t size) {
    for (; size; size--) {
        edc = (edc >> 8) ^ c_edc_lut[(edc ^ *src++) & 0xff];
    }
    return edc;
}
