// SPDX-License-Identifier: GPL-2.0-or-later

#include "compat.h"

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)
/**
 *	sysfs_emit - scnprintf equivalent, aware of PAGE_SIZE buffer.
 *	@buf:	start of PAGE_SIZE buffer.
 *	@fmt:	format
 *	@...:	optional arguments to @format
 *
 *
 * Returns number of characters written to @buf.
 */
int sysfs_emit(char *buf, const char *fmt, ...)
{
    va_list args;
    int len;

    if (WARN(!buf || offset_in_page(buf),
             "invalid sysfs_emit: buf:%p\n", buf))
        return 0;

    va_start(args, fmt);
    len = vscnprintf(buf, PAGE_SIZE, fmt, args);
    va_end(args);

    return len;
}
#endif
