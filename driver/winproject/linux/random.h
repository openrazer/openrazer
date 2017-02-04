#ifndef RANDOM_H_
#define RANDOM_H_

static inline void get_random_bytes(void* rand_ptr, unsigned int rand_size) {
	char failed = 0;
	static HCRYPTPROV prov = 0;
	if (prov == 0) {
		if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, 0))
			failed = 1;
	}
	if (!failed && !CryptGenRandom(prov, rand_size, (unsigned char*)rand_ptr))
		printf("get_random_bytes failed\n");
}


#endif /* RANDOM_H_ */
