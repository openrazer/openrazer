/* SPDX-License-Identifier: GPL-2.0-or-later */

// Compatbility for fallthrough pseudo keyword for Linux versions older than v5.4
// See also https://git.kernel.org/torvalds/c/294f69e
#ifndef fallthrough
#if __has_attribute(__fallthrough__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif
#endif

// KEY_MACRO* has been added in Linux 5.5, so define ourselves for older kernels.
// See also https://git.kernel.org/torvalds/c/b5625db
#ifndef KEY_MACRO1
#define KEY_MACRO1  0x290
#define KEY_MACRO2  0x291
#define KEY_MACRO3  0x292
#define KEY_MACRO4  0x293
#define KEY_MACRO5  0x294
#define KEY_MACRO6  0x295
#define KEY_MACRO7  0x296
#define KEY_MACRO8  0x297
#define KEY_MACRO9  0x298
#define KEY_MACRO10 0x299
#define KEY_MACRO11 0x2a0
#define KEY_MACRO12 0x2a1
// ...
#define KEY_MACRO27 0x2aa
#define KEY_MACRO28 0x2ab
#define KEY_MACRO29 0x2ac
#define KEY_MACRO30 0x2ad
#endif

/* REL_HWHEEL_HI_RES was added in Linux 5.0, so define ourselves for older kernels
 * See also https://git.kernel.org/torvalds/c/52ea899 */
#ifndef REL_HWHEEL_HI_RES
#define REL_HWHEEL_HI_RES 0x0c
#endif
