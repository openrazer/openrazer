#ifndef SLAB_H_
#define SLAB_H_

typedef enum {
	GFP_KERNEL,
	GFP_ATOMIC,
	__GFP_HIGHMEM,
	__GFP_HIGH
} gfp_t;

static inline void *kzalloc(size_t s, gfp_t gfp) {
	void *p = malloc(s);

	memset(p, 0, s);
	return p;
}

inline void *kmemdup(const void *src, size_t len, gfp_t gfp) {
	void *p;
	p = malloc(len);
	if (p)
		memcpy(p, src, len);
	return p;
}

static inline void kfree(const void* p) {
	free((void*)p);
}


#endif /* SLAB_H_ */
