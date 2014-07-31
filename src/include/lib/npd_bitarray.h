/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 * $Copyright: Copyright 2014 Autelan Tech Co., Ltd.
 *
 */
 /*
 * Bit Array Operations
 */

#ifndef _NPD_BITARRAY_H
#define _NPD_BITARRAY_H


#define NPD_SZLONG_LOG 5
#define NPD_SZLONG_MASK 31UL
#define NPD_BITS_PER_LONG 32
#define NPD_BIT(nr)			(1UL << (nr))
#define NPD_BIT_MASK(nr)		(1UL << ((nr) % NPD_BITS_PER_LONG))
#define NPD_BIT_WORD(nr)		((nr) / NPD_BITS_PER_LONG)
#define NPD_BITS_PER_BYTE		8
#define NPD_BITS_TO_LONGS(nr)	((nr+NPD_BITS_PER_LONG-1)/NPD_BITS_PER_LONG)
#define NPD_BITARRAY_LAST_WORD_MASK(nbits)					\
(									\
	((nbits) % NPD_BITS_PER_LONG) ?					\
		(1UL<<((nbits) % NPD_BITS_PER_LONG))-1 : ~0UL		\
)


#define npd_for_each_bit(bit, addr, size) \
	for ((bit) = npd_find_first_bit((addr), (size)); \
	     (bit) < (size); \
	     (bit) = npd_find_next_bit((addr), (size), (bit) + 1))



inline unsigned long npd_hweight_long(unsigned long w);

/**
 * find_first_bit - find the first set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit number of the first set bit.
 */
extern unsigned long npd_find_first_bit(const unsigned long *addr,
				    unsigned long size);

/**
 * find_first_zero_bit - find the first cleared bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit number of the first cleared bit.
 */
extern unsigned long npd_find_first_zero_bit(const unsigned long *addr,
					 unsigned long size);



/**
 * find_next_bit - find the next set bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 */
extern unsigned long npd_find_next_bit(const unsigned long *addr,
				   unsigned long size, unsigned long offset);

/**
 * find_next_zero_bit - find the next cleared bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The bitmap size in bits
 */

extern unsigned long npd_find_next_zero_bit(const unsigned long *addr,
					unsigned long size,
					unsigned long offset);

/*
 * The NPD_DECLARE_BITMAP(name,bits) macro, in linux/types.h, can be used
 * to declare an array named 'name' of just enough unsigned longs to
 * contain all bit positions from 0 to 'bits' - 1.
 */

#define NPD_DECLARE_BITARRAY_TYPE(name,bits) \
	typedef struct name##_s \
	{\
         unsigned long nbits[NPD_BITS_TO_LONGS(bits)];\
    }name##_t;

extern int __npd_bitarray_empty(const unsigned long *bitarray, int bits);
extern int __npd_bitarray_full(const unsigned long *bitarray, int bits);
extern int __npd_bitarray_equal(const unsigned long *bitarray1,
                	const unsigned long *bitarray2, int bits);
extern void __npd_bitarray_complement(unsigned long *dst, const unsigned long *src,
			int bits);
extern void __npd_bitarray_shift_right(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern void __npd_bitarray_shift_left(unsigned long *dst,
                        const unsigned long *src, int shift, int bits);
extern void __npd_bitarray_and(unsigned long *dst, const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern void __npd_bitarray_or(unsigned long *dst, const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern void __npd_bitarray_xor(unsigned long *dst, const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern void __npd_bitarray_andnot(unsigned long *dst, const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern int __npd_bitarray_intersects(const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern int __npd_bitarray_subset(const unsigned long *bitarray1,
			const unsigned long *bitarray2, int bits);
extern int __npd_bitarray_weight(const unsigned long *bitarray, int bits);

extern void npd_bitarray_ntoh(unsigned long *buf, int nbits);

extern void npd_bitarray_hton(unsigned long *buf, int nbits);

#define BITARRAY_LAST_WORD_MASK(nbits)					\
(									\
	((nbits) % NPD_BITS_PER_LONG) ?					\
		(1UL<<((nbits) % NPD_BITS_PER_LONG))-1 : ~0UL		\
)

static inline void npd_bitarray_zero(unsigned long *dst, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = 0UL;
	else {
		int len = NPD_BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}

static inline void npd_bitarray_fill(unsigned long *dst, int nbits)
{
	size_t nlongs = NPD_BITS_TO_LONGS(nbits);
	if (nlongs > 1) {
		int len = (nlongs - 1) * sizeof(unsigned long);
		memset(dst, 0xff,  len);
	}
	dst[nlongs - 1] = NPD_BITARRAY_LAST_WORD_MASK(nbits);
}

static inline void npd_bitarray_copy(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src;
	else {
		int len = NPD_BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memcpy(dst, src, len);
	}
}

static inline void npd_bitarray_and(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 & *src2;
	else
		__npd_bitarray_and(dst, src1, src2, nbits);
}

static inline void npd_bitarray_or(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 | *src2;
	else
		__npd_bitarray_or(dst, src1, src2, nbits);
}

static inline void npd_bitarray_xor(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 ^ *src2;
	else
		__npd_bitarray_xor(dst, src1, src2, nbits);
}

static inline void npd_bitarray_andnot(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 & ~(*src2);
	else
		__npd_bitarray_andnot(dst, src1, src2, nbits);
}

static inline void npd_bitarray_complement(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = ~(*src) & NPD_BITARRAY_LAST_WORD_MASK(nbits);
	else
		__npd_bitarray_complement(dst, src, nbits);
}

static inline int npd_bitarray_equal(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! ((*src1 ^ *src2) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_equal(src1, src2, nbits);
}

static inline int npd_bitarray_intersects(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ((*src1 & *src2) & NPD_BITARRAY_LAST_WORD_MASK(nbits)) != 0;
	else
		return __npd_bitarray_intersects(src1, src2, nbits);
}

static inline int npd_bitarray_subset(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! ((*src1 & ~(*src2)) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_subset(src1, src2, nbits);
}

static inline int npd_bitarray_empty(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! (*src & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_empty(src, nbits);
}

static inline int npd_bitarray_full(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! (~(*src) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_full(src, nbits);
}

static inline int npd_bitarray_weight(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return npd_hweight_long(*src & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	return __npd_bitarray_weight(src, nbits);
}

static inline void npd_bitarray_shift_right(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src >> n;
	else
		__npd_bitarray_shift_right(dst, src, n, nbits);
}

static inline void npd_bitarray_shift_left(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = (*src << n) & NPD_BITARRAY_LAST_WORD_MASK(nbits);
	else
		__npd_bitarray_shift_left(dst, src, n, nbits);
}

static inline int npd_bitarray_test_bit(const unsigned long *addr, unsigned long  nr)
{
	return 1UL & (addr[NPD_BIT_WORD(nr)] >> (nr & (NPD_BITS_PER_LONG-1)));
}

static inline void npd_bitarray_set_bit(const unsigned long *addr, unsigned long  nr)
{
	unsigned short bit = nr & NPD_SZLONG_MASK;

    {
		unsigned long *a = (unsigned long *)addr;
		unsigned long mask;

		a += nr >> NPD_SZLONG_LOG;
		mask = 1UL << bit;
		*a |= mask;
	}
}

static inline void npd_bitarray_clear_bit(const unsigned long *addr, unsigned long  nr)
{
	unsigned short bit = nr & NPD_SZLONG_MASK;

    {
		unsigned long *a = (unsigned long *)addr;
		unsigned long mask;

		a += nr >> NPD_SZLONG_LOG;
		mask = 1UL << bit;
		*a &= ~mask;
	}
}

static inline void npd_bitarray_change_bit(volatile unsigned long *addr, unsigned long nr)
{
	unsigned short bit = nr & NPD_SZLONG_MASK;

    {
		unsigned long *a = (unsigned long *)addr;
		unsigned long mask;

		a += nr >> NPD_SZLONG_LOG;
		mask = 1UL << bit;
		*a ^= mask;
	}
}


#include "lib/netif_index.h"

#define _NPD_PBMP_PORT_MAX MAX_SWITCHPORT_PER_SYSTEM
#define _NPD_VBMP_VLAN_MAX 4096


#define _NPD_SHR_BMP_BMCLEAR(bm)		(npd_bitarray_zero((unsigned long*)((bm).nbits),sizeof(bm)*NPD_BITS_PER_BYTE))
#define _NPD_SHR_BMP_BMNULL(bm)		(npd_bitarray_empty((const unsigned long*)((bm).nbits), sizeof(bm)*NPD_BITS_PER_BYTE))
#define _NPD_SHR_BMP_BMEQ(bma, bmb)	(npd_bitarray_equal((const unsigned long*)((bma).nbits), \
                                                             (const unsigned long*)((bmb).nbits), \
                                                              sizeof(bma)*NPD_BITS_PER_BYTE)) 
#define _NPD_SHR_BMP_BMFILL(bm)     (npd_bitarray_fill((unsigned long*)((bm).nbits), sizeof(bm)*NPD_BITS_PER_BYTE))                                                              
#define	_NPD_SHR_BMP_COUNT(bm, count)	do \
    {\
        count = npd_bitarray_weight((const unsigned long*)((bm).nbits),sizeof(bm)*NPD_BITS_PER_BYTE);\
    }while(0)

#define _NPD_SHR_BMP_NTOH(bm) (npd_bitarray_ntoh((unsigned long*)((bm).nbits), sizeof(bm)*NPD_BITS_PER_BYTE))

#define _NPD_SHR_BMP_HTON(bm) (npd_bitarray_hton((unsigned long*)((bm).nbits), sizeof(bm)*NPD_BITS_PER_BYTE))


/* generics that use the previously defined helpers */
#define _NPD_SHR_BMP_CLEAR(bm)		_NPD_SHR_BMP_BMCLEAR(bm)
#define _NPD_SHR_BMP_FILL(bm)		_NPD_SHR_BMP_BMFILL(bm)
#define _NPD_SHR_BMP_ITER(bm, bit)	npd_for_each_bit(bit, (const unsigned long*)((bm).nbits), sizeof(bm)*NPD_BITS_PER_BYTE)

#define _NPD_SHR_BMP_IS_NULL(bm)		(_NPD_SHR_BMP_BMNULL(bm))
#define _NPD_SHR_BMP_NOT_NULL(bm)		(!_NPD_SHR_BMP_BMNULL(bm))
#define _NPD_SHR_BMP_EQ(bma, bmb)		(_NPD_SHR_BMP_BMEQ(bma, bmb))
#define _NPD_SHR_BMP_NEQ(bma, bmb)		(!_NPD_SHR_BMP_BMEQ(bma, bmb))

/* Assignment operators */
#define _NPD_SHR_BMP_ASSIGN(dst, src)	(dst) = (src)
#define _NPD_SHR_BMP_AND(bma, bmb)		(npd_bitarray_and((unsigned long*)((bma).nbits),\
                                                       (const unsigned long*)((bma).nbits), \
                                                       (const unsigned long*)((bmb).nbits), \
                                                       sizeof(bma)*NPD_BITS_PER_BYTE)) 
#define _NPD_SHR_BMP_OR(bma, bmb)		(npd_bitarray_or((unsigned long*)((bma).nbits),\
                                                       (const unsigned long*)((bma).nbits), \
                                                       (const unsigned long*)((bmb).nbits), \
                                                       sizeof(bma)*NPD_BITS_PER_BYTE))
#define _NPD_SHR_BMP_XOR(bma, bmb)		(npd_bitarray_xor((unsigned long*)((bma).nbits),\
                                                       (const unsigned long*)((bma).nbits), \
                                                       (const unsigned long*)((bmb).nbits), \
                                                       sizeof(bma)*NPD_BITS_PER_BYTE))
#define _NPD_SHR_BMP_REMOVE(bma, bmb)	(npd_bitarray_andnot((unsigned long*)((bma).nbits),\
                                                       (const unsigned long*)((bma).nbits), \
                                                       (const unsigned long*)((bmb).nbits), \
                                                       sizeof(bma)*NPD_BITS_PER_BYTE))
#define _NPD_SHR_BMP_NEGATE(bma, bmb)	(npd_bitarray_complement(( unsigned long*)((bma).nbits),\
                                                       (const unsigned long*)((bma).nbits), \
                                                       (const unsigned long*)((bmb).nbits), \
                                                       sizeof(bma)*NPD_BITS_PER_BYTE)) 

/* Port PBMP operators */
#define _NPD_SHR_BMP_MEMBER(bm, nbit)	\
	(npd_bitarray_test_bit((const unsigned long*)((bm).nbits), nbit))
#define _NPD_SHR_BMP_BIT_SET(bm, nbit)	do { \
		npd_bitarray_zero((const unsigned long*)((bm).nbits), sizeof(bm)); \
		npd_bitarray_set_bit((const unsigned long*)((bm).nbits), nbit); \
	} while(0)
#define _NPD_SHR_BMP_BIT_ADD(bm, nbit)	\
	(npd_bitarray_set_bit((const unsigned long*)((bm).nbits), nbit))
#define _NPD_SHR_BMP_BIT_REMOVE(bm, nbit)	\
	(npd_bitarray_clear_bit((const unsigned long*)((bm).nbits), nbit))

/*switch port bit array structure*/
NPD_DECLARE_BITARRAY_TYPE(npd_pbmp, _NPD_PBMP_PORT_MAX);

/*VLAN bit array structure*/
NPD_DECLARE_BITARRAY_TYPE(npd_vbmp, _NPD_VBMP_VLAN_MAX);    


#define NPD_PBMP_PORT_MAX       _NPD_PBMP_PORT_MAX 

#define NPD_PBMP_CLEAR(pbm)     _NPD_SHR_BMP_CLEAR(pbm)
#define NPD_PBMP_MEMBER(bmp, port)  _NPD_SHR_BMP_MEMBER(bmp, port) 
#define NPD_PBMP_ITER(bmp, port)  _NPD_SHR_BMP_ITER(bmp,port) 
#define NPD_PBMP_COUNT(pbm, count)  _NPD_SHR_BMP_COUNT(pbm, count) 
#define NPD_PBMP_FILL(pbm)      _NPD_SHR_BMP_FILL(pbm)
#define NPD_PBMP_IS_NULL(pbm)   _NPD_SHR_BMP_IS_NULL(pbm) 
#define NPD_PBMP_NOT_NULL(pbm)  (!NPD_PBMP_IS_NULL(pbm)) 
#define NPD_PBMP_EQ(pbm_a, pbm_b) _NPD_SHR_BMP_EQ(pbm_a, pbm_b) 
#define NPD_PBMP_NEQ(pbm_a, pbm_b)  (!NPD_PBMP_EQ(pbm_a, pbm_b)) 

/* Assignment operators */
#define NPD_PBMP_ASSIGN(dst, src)  _NPD_SHR_BMP_ASSIGN(dst,src)
#define NPD_PBMP_AND(pbm_a, pbm_b) _NPD_SHR_BMP_AND(pbm_a, pbm_b) 
#define NPD_PBMP_OR(pbm_a, pbm_b)  _NPD_SHR_BMP_OR(pbm_a, pbm_b) 
#define NPD_PBMP_XOR(pbm_a, pbm_b)  _NPD_SHR_BMP_XOR(pbm_a, pbm_b)  
#define NPD_PBMP_REMOVE(pbm_a, pbm_b) _NPD_SHR_BMP_REMOVE(pbm_a, pbm_b) 
#define NPD_PBMP_NEGATE(pbm_a, pbm_b)  _NPD_SHR_BMP_NEGATE(pbm_a, pbm_b)
/* Port PBMP operators */
#define NPD_PBMP_PORT_SET(pbm, port)  _NPD_SHR_BMP_BIT_SET(pbm, port) 
#define NPD_PBMP_PORT_ADD(pbm, port)  _NPD_SHR_BMP_BIT_ADD(pbm, port) 
#define NPD_PBMP_PORT_REMOVE(pbm, port)  _NPD_SHR_BMP_BIT_REMOVE(pbm, port) 

#define NPD_PBMP_PORT_NTOH(bmp) _NPD_SHR_BMP_NTOH(bmp)
#define NPD_PBMP_PORT_HTON(bmp) _NPD_SHR_BMP_HTON(bmp)


#define NPD_VBMP_VLAN_MAX       _NPD_VBMP_VLAN_MAX 

#define NPD_VBMP_CLEAR(bmp)     _NPD_SHR_BMP_CLEAR(bmp) 
#define NPD_VBMP_FILL(bmp)      _NPD_SHR_BMP_FILL(bmp)
#define NPD_VBMP_MEMBER(bmp, vlan)  _NPD_SHR_BMP_MEMBER((bmp), (vlan)) 
#define NPD_VBMP_ITER(bmp, vlan)  _NPD_SHR_BMP_ITER((bmp), (vlan)) 
#define NPD_VBMP_COUNT(bmp, count)  _NPD_SHR_BMP_COUNT(bmp, count) 

#define NPD_VBMP_IS_NULL(bmp)   _NPD_SHR_BMP_IS_NULL(bmp) 
#define NPD_VBMP_NOT_NULL(bmp)  _NPD_SHR_BMP_NOT_NULL(bmp) 
#define NPD_VBMP_EQ(bmp_a, bmp_b)  _NPD_SHR_BMP_EQ(bmp_a, bmp_b) 
#define NPD_VBMP_NEQ(bmp_a, bmp_b)  _NPD_SHR_BMP_NEQ(bmp_a, bmp_b) 

/* Assignment operators */
#define NPD_VBMP_ASSIGN(dst, src)  _NPD_SHR_BMP_ASSIGN(dst, src) 
#define NPD_VBMP_AND(bmp_a, bmp_b)  _NPD_SHR_BMP_AND(bmp_a, bmp_b) 
#define NPD_VBMP_OR(bmp_a, bmp_b)  _NPD_SHR_BMP_OR(bmp_a, bmp_b) 
#define NPD_VBMP_XOR(bmp_a, bmp_b)  _NPD_SHR_BMP_XOR(bmp_a, bmp_b) 
#define NPD_VBMP_REMOVE(bmp_a, bmp_b)  _NPD_SHR_BMP_REMOVE(bmp_a, bmp_b) 
#define NPD_VBMP_NEGATE(bmp_a, bmp_b)  _NPD_SHR_BMP_NEGATE(bmp_a, bmp_b) 

/* Port PBMP operators */
#define NPD_VBMP_VLAN_SET(bmp, vlan)  _NPD_SHR_BMP_BIT_SET(bmp, vlan) 
#define NPD_VBMP_VLAN_ADD(bmp, vlan)  _NPD_SHR_BMP_BIT_ADD(bmp, vlan) 
#define NPD_VBMP_VLAN_REMOVE(bmp, vlan)  _NPD_SHR_BMP_BIT_REMOVE(bmp, vlan) 

#define NPD_VBMP_VLAN_NTOH(bmp) _NPD_SHR_BMP_NTOH(bmp)
#define NPD_VBMP_VLAN_HTON(bmp) _NPD_SHR_BMP_HTON(bmp)

int vbmp_2_vlanlist_str(npd_vbmp_t bmp, char *string);
int vlanlist_str_2_vbmp(const char *string, npd_vbmp_t bmp);
int pbmp_2_portlist_str(npd_pbmp_t bmp, char * string);
int portlist_str_2_pbmp(const char * string, npd_pbmp_t bmp);

#endif	/* !_NPD_BITARRAY_H_ */



