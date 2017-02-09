#ifndef KERNEL_H_
#define KERNEL_H_

#include <windows.h>

#define DLL_INTERNAL __declspec( dllexport )

#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long

#define __u8 unsigned char
#define __u16 unsigned short
#define __u32 unsigned int
#define __u64 unsigned long
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long
#define __le8 unsigned char
#define __le16 unsigned short
#define __le32 unsigned int
#define __le64 unsigned long
#define __s8 signed char
#define __s16 signed short
#define __s32 signed int
#define __s64 signed long
#define uint unsigned int
#define ulong unsigned long

#define socklen_t int

#define bool int
#define true 1
#define false 0

#define size_t SIZE_T
#define ssize_t SSIZE_T

struct mutex {
	CRITICAL_SECTION lock;
};

inline void mutex_init(struct mutex* mutex) {
	InitializeCriticalSection(&mutex->lock);
}

inline void mutex_lock(struct mutex* mutex) {
	EnterCriticalSection(&mutex->lock);
}

inline void mutex_unlock(struct mutex* mutex) {
	LeaveCriticalSection(&mutex->lock);
}

inline int mutex_trylock(struct mutex* mutex) {
	return TryEnterCriticalSection(&mutex->lock);
}

inline int mutex_is_locked(struct mutex* mutex) {
	if (mutex_trylock(mutex)) {
		mutex_unlock(mutex);
		return 0;
	}
	else
		return 1;
}

inline void set_bit(int nr, volatile unsigned long *addr) {
        int *a = (int *)addr;
        int mask;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        *a |= mask;
}
#define __set_bit set_bit

inline void clear_bit(int nr, volatile unsigned long *addr) {
        int *a = (int *)addr;
        int mask;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        *a &= ~mask;
}

inline int test_bit(int nr, const void *addr) {
        int *a = (int *)addr;
        int mask;

		a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        return ((mask & *a) != 0);
}

#endif /* KERNEL_H_ */
