#ifndef INIT_H_
#define INIT_H_

#define strdup _strdup

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


inline unsigned short eflip(unsigned short val) {
	return (val & 0xff) * 0xFF + (val >> 8);
}

#endif /* INIT_H_ */
