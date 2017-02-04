#ifndef INIT_H_
#define INIT_H_

static inline int sprint_f(char* buf, const char* fmt, ...) {
	va_list args;
	int i;
	va_start(args, fmt);
	i = sprintf_s(buf, strlen(buf) - 1, fmt, args);
	va_end(args);
	return i;
}
#define sprintf sprint_f

static inline int strncpy_f(char* dest, const char* source, const size_t sourceLen) {
	return strncpy_s(dest, strlen(dest) - 1, source, sourceLen);
}
#define strncpy strncpy_f

static inline int strcpy_f(char* dest, const char* source) {
	return strcpy_s(dest, strlen(dest) - 1, source);
}
#define strcpy strcpy_f

#define strdup _strdup

#define KERN_WARNING
#define KERN_ALERT
#define KERN_CRIT

#define printk printf

inline unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base) {
	return strtoul(cp, endp, base);
}

inline void usleep(__int64 usec) {
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

inline void msleep(__int64 msec) {
	usleep(1000 * msec);
}

inline void usleep_range(__int64 usec1, __int64 usec2) {
	usleep((usec1 + usec2) / 2);
}

inline unsigned short eflip(unsigned short val) {
	return (val & 0xff) * 0xFF + (val >> 8);
}

#endif /* INIT_H_ */
