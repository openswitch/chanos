/*
 * lib/npd_bitarray.c
 * Helper functions for npd_bitarray.h.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <linux/types.h>
#include <limits.h>
#include <stdarg.h>
#include <netinet/in.h>

#include "lib/npd_bitarray.h"

unsigned int npd_hweight32(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55555555);
	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);
	res = (res + (res >> 4)) & 0x0F0F0F0F;
	res = res + (res >> 8);
	return (res + (res >> 16)) & 0x000000FF;
}

unsigned int npd_hweight16(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x5555);
	res = (res & 0x3333) + ((res >> 2) & 0x3333);
	res = (res + (res >> 4)) & 0x0F0F;
	return (res + (res >> 8)) & 0x00FF;
}

unsigned int npd_hweight8(unsigned int w)
{
	unsigned int res = w - ((w >> 1) & 0x55);
	res = (res & 0x33) + ((res >> 2) & 0x33);
	return (res + (res >> 4)) & 0x0F;
}

unsigned long npd_hweight64(unsigned long long w)
{
#if NPD_BITS_PER_LONG == 32
	return npd_hweight32((unsigned int)(w >> 32)) + npd_hweight32((unsigned int)w);
#elif NPD_BITS_PER_LONG == 64
	__u64 res = w - ((w >> 1) & 0x5555555555555555ul);
	res = (res & 0x3333333333333333ul) + ((res >> 2) & 0x3333333333333333ul);
	res = (res + (res >> 4)) & 0x0F0F0F0F0F0F0F0Ful;
	res = res + (res >> 8);
	res = res + (res >> 16);
	return (res + (res >> 32)) & 0x00000000000000FFul;
#endif
}

inline unsigned long npd_hweight_long(unsigned long w)
{
	return sizeof(w) == 4 ? npd_hweight32(w) : npd_hweight64(w);
}

int __npd_bitarray_empty(const unsigned long *bitarray, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitarray[k])
			return 0;

	if (bits % NPD_BITS_PER_LONG)
		if (bitarray[k] & NPD_BITARRAY_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

int __npd_bitarray_full(const unsigned long *bitarray, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (~bitarray[k])
			return 0;

	if (bits % NPD_BITS_PER_LONG)
		if (~bitarray[k] & NPD_BITARRAY_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

int __npd_bitarray_equal(const unsigned long *bitarray1,
		const unsigned long *bitarray2, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitarray1[k] != bitarray2[k])
			return 0;

	if (bits % NPD_BITS_PER_LONG)
		if ((bitarray1[k] ^ bitarray2[k]) & NPD_BITARRAY_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

void __npd_bitarray_complement(unsigned long *dst, const unsigned long *src, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		dst[k] = ~src[k];

	if (bits % NPD_BITS_PER_LONG)
		dst[k] = ~src[k] & NPD_BITARRAY_LAST_WORD_MASK(bits);
}

/**
 * __bitarray_shift_right - logical right shift of the bits in a bitarray
 *   @dst : destination bitarray
 *   @src : source bitarray
 *   @shift : shift by this many bits
 *   @bits : bitarray size, in bits
 *
 * Shifting right (dividing) means moving bits in the MS -> LS bit
 * direction.  Zeros are fed into the vacated MS positions and the
 * LS bits shifted off the bottom are lost.
 */
void __npd_bitarray_shift_right(unsigned long *dst,
			const unsigned long *src, int shift, int bits)
{
	int k, lim = NPD_BITS_TO_LONGS(bits), left = bits % NPD_BITS_PER_LONG;
	int off = shift/NPD_BITS_PER_LONG, rem = shift % NPD_BITS_PER_LONG;
	unsigned long mask = (1UL << left) - 1;
	for (k = 0; off + k < lim; ++k) {
		unsigned long upper, lower;

		/*
		 * If shift is not word aligned, take lower rem bits of
		 * word above and make them the top rem bits of result.
		 */
		if (!rem || off + k + 1 >= lim)
			upper = 0;
		else {
			upper = src[off + k + 1];
			if (off + k + 1 == lim - 1 && left)
				upper &= mask;
		}
		lower = src[off + k];
		if (left && off + k == lim - 1)
			lower &= mask;
		dst[k] = upper << (NPD_BITS_PER_LONG - rem) | lower >> rem;
		if (left && k == lim - 1)
			dst[k] &= mask;
	}
	if (off)
		memset(&dst[lim - off], 0, off*sizeof(unsigned long));
}


/**
 * __bitarray_shift_left - logical left shift of the bits in a bitarray
 *   @dst : destination bitarray
 *   @src : source bitarray
 *   @shift : shift by this many bits
 *   @bits : bitarray size, in bits
 *
 * Shifting left (multiplying) means moving bits in the LS -> MS
 * direction.  Zeros are fed into the vacated LS bit positions
 * and those MS bits shifted off the top are lost.
 */

void __npd_bitarray_shift_left(unsigned long *dst,
			const unsigned long *src, int shift, int bits)
{
	int k, lim = NPD_BITS_TO_LONGS(bits), left = bits % NPD_BITS_PER_LONG;
	int off = shift/NPD_BITS_PER_LONG, rem = shift % NPD_BITS_PER_LONG;
	for (k = lim - off - 1; k >= 0; --k) {
		unsigned long upper, lower;

		/*
		 * If shift is not word aligned, take upper rem bits of
		 * word below and make them the bottom rem bits of result.
		 */
		if (rem && k > 0)
			lower = src[k - 1];
		else
			lower = 0;
		upper = src[k];
		if (left && k == lim - 1)
			upper &= (1UL << left) - 1;
		dst[k + off] = lower  >> (NPD_BITS_PER_LONG - rem) | upper << rem;
		if (left && k + off == lim - 1)
			dst[k + off] &= (1UL << left) - 1;
	}
	if (off)
		memset(dst, 0, off*sizeof(unsigned long));
}

void __npd_bitarray_and(unsigned long *dst, const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitarray1[k] & bitarray2[k];
}

void __npd_bitarray_or(unsigned long *dst, const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitarray1[k] | bitarray2[k];
}

void __npd_bitarray_xor(unsigned long *dst, const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitarray1[k] ^ bitarray2[k];
}

void __npd_bitarray_andnot(unsigned long *dst, const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(bits);

	for (k = 0; k < nr; k++)
		dst[k] = bitarray1[k] & ~bitarray2[k];
}

int __npd_bitarray_intersects(const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitarray1[k] & bitarray2[k])
			return 1;

	if (bits % NPD_BITS_PER_LONG)
		if ((bitarray1[k] & bitarray2[k]) & NPD_BITARRAY_LAST_WORD_MASK(bits))
			return 1;
	return 0;
}

int __npd_bitarray_subset(const unsigned long *bitarray1,
				const unsigned long *bitarray2, int bits)
{
	int k, lim = bits/NPD_BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitarray1[k] & ~bitarray2[k])
			return 0;

	if (bits % NPD_BITS_PER_LONG)
		if ((bitarray1[k] & ~bitarray2[k]) & NPD_BITARRAY_LAST_WORD_MASK(bits))
			return 0;
	return 1;
}

int __npd_bitarray_weight(const unsigned long *bitarray, int bits)
{
	int k, w = 0, lim = bits/NPD_BITS_PER_LONG;

	for (k = 0; k < lim; k++)
		w += npd_hweight_long(bitarray[k]);

	if (bits % NPD_BITS_PER_LONG)
		w += npd_hweight_long(bitarray[k] & NPD_BITARRAY_LAST_WORD_MASK(bits));

	return w;
}



static int npd_bitarray_pos_to_ord(const unsigned long *buf, int pos, int bits)
{
	int i, ord;

	if (pos < 0 || pos >= bits || !npd_bitarray_test_bit(buf,pos))
		return -1;

	i = npd_find_first_bit(buf, bits);
	ord = 0;
	while (i < pos) {
		i = npd_find_next_bit(buf, bits, i + 1);
	     	ord++;
	}
	return ord;
}

static int npd_bitarray_ord_to_pos(const unsigned long *buf, int ord, int bits)
{
	int pos = 0;

	if (ord >= 0 && ord < bits) {
		int i;

		for (i = npd_find_first_bit(buf, bits);
		     i < bits && ord > 0;
		     i = npd_find_next_bit(buf, bits, i + 1))
	     		ord--;
		if (i < bits && ord == 0)
			pos = i;
	}

	return pos;
}

void npd_bitarray_ntoh(unsigned long *buf, int nbits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(nbits);

	for (k = 0; k < nr; k++)
		buf[k] = ntohl(buf[k]);
    
}

void npd_bitarray_hton(unsigned long *buf, int nbits)
{
	int k;
	int nr = NPD_BITS_TO_LONGS(nbits);

	for (k = 0; k < nr; k++)
		buf[k] = htonl(buf[k]);
}
#if 0
inline void npd_bitarray_zero(unsigned long *dst, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = 0UL;
	else {
		int len = NPD_BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}

inline void npd_bitarray_fill(unsigned long *dst, int nbits)
{
	size_t nlongs = NPD_BITS_TO_LONGS(nbits);
	if (nlongs > 1) {
		int len = (nlongs - 1) * sizeof(unsigned long);
		memset(dst, 0xff,  len);
	}
	dst[nlongs - 1] = NPD_BITARRAY_LAST_WORD_MASK(nbits);
}

inline void npd_bitarray_copy(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src;
	else {
		int len = NPD_BITS_TO_LONGS(nbits) * sizeof(unsigned long);
		memcpy(dst, src, len);
	}
}

inline void npd_bitarray_and(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 & *src2;
	else
		__npd_bitarray_and(dst, src1, src2, nbits);
}

inline void npd_bitarray_or(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 | *src2;
	else
		__npd_bitarray_or(dst, src1, src2, nbits);
}

inline void npd_bitarray_xor(unsigned long *dst, const unsigned long *src1,
		const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 ^ *src2;
	else
		__npd_bitarray_xor(dst, src1, src2, nbits);
}

inline void npd_bitarray_andnot(unsigned long *dst, const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src1 & ~(*src2);
	else
		__npd_bitarray_andnot(dst, src1, src2, nbits);
}

inline void npd_bitarray_complement(unsigned long *dst, const unsigned long *src,
			int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = ~(*src) & NPD_BITARRAY_LAST_WORD_MASK(nbits);
	else
		__npd_bitarray_complement(dst, src, nbits);
}

inline int npd_bitarray_equal(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! ((*src1 ^ *src2) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_equal(src1, src2, nbits);
}

inline int npd_bitarray_intersects(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ((*src1 & *src2) & NPD_BITARRAY_LAST_WORD_MASK(nbits)) != 0;
	else
		return __npd_bitarray_intersects(src1, src2, nbits);
}

inline int npd_bitarray_subset(const unsigned long *src1,
			const unsigned long *src2, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! ((*src1 & ~(*src2)) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_subset(src1, src2, nbits);
}

inline int npd_bitarray_empty(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! (*src & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_empty(src, nbits);
}

inline int npd_bitarray_full(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return ! (~(*src) & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	else
		return __npd_bitarray_full(src, nbits);
}

inline int npd_bitarray_weight(const unsigned long *src, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		return npd_hweight_long(*src & NPD_BITARRAY_LAST_WORD_MASK(nbits));
	return __npd_bitarray_weight(src, nbits);
}

inline void npd_bitarray_shift_right(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = *src >> n;
	else
		__npd_bitarray_shift_right(dst, src, n, nbits);
}

inline void npd_bitarray_shift_left(unsigned long *dst,
			const unsigned long *src, int n, int nbits)
{
	if (nbits <= NPD_BITS_PER_LONG)
		*dst = (*src << n) & NPD_BITARRAY_LAST_WORD_MASK(nbits);
	else
		__npd_bitarray_shift_left(dst, src, n, nbits);
}

inline int npd_bitarray_test_bit(const unsigned long *addr, unsigned long  nr)
{
	return 1UL & (addr[NPD_BIT_WORD(nr)] >> (nr & (NPD_BITS_PER_LONG-1)));
}

inline void npd_bitarray_set_bit(const unsigned long *addr, unsigned long  nr)
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

inline void npd_bitarray_clear_bit(const unsigned long *addr, unsigned long  nr)
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

inline void npd_bitarray_change_bit(volatile unsigned long *addr, unsigned long nr)
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
#endif

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __npd_ffs(unsigned long word)
{
	int num = 0;

#if NPD_BITS_PER_LONG == 64
	if ((word & 0xffffffff) == 0) {
		num += 32;
		word >>= 32;
	}
#endif
	if ((word & 0xffff) == 0) {
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0) {
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0) {
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0) {
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

#define npd_ffz(x)  __npd_ffs(~(x))

/*
 * Find the next set bit in a memory region.
 */
unsigned long npd_find_next_bit(const unsigned long *addr, unsigned long size,
			    unsigned long offset)
{
	const unsigned long *p = addr + NPD_BIT_WORD(offset);
	unsigned long result = offset & ~(NPD_BITS_PER_LONG-1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset %= NPD_BITS_PER_LONG;
	if (offset) {
		tmp = *(p++);
		tmp &= (~0UL << offset);
		if (size < NPD_BITS_PER_LONG)
			goto found_first;
		if (tmp)
			goto found_middle;
		size -= NPD_BITS_PER_LONG;
		result += NPD_BITS_PER_LONG;
	}
	while (size & ~(NPD_BITS_PER_LONG-1)) {
		if ((tmp = *(p++)))
			goto found_middle;
		result += NPD_BITS_PER_LONG;
		size -= NPD_BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp &= (~0UL >> (NPD_BITS_PER_LONG - size));
	if (tmp == 0UL)		/* Are any bits set? */
		return result + size;	/* Nope. */
found_middle:
	return result + __npd_ffs(tmp);
}

/*
 * This implementation of find_{first,next}_zero_bit was stolen from
 * Linus' asm-alpha/bitops.h.
 */
unsigned long npd_find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset)
{
	const unsigned long *p = addr + NPD_BIT_WORD(offset);
	unsigned long result = offset & ~(NPD_BITS_PER_LONG-1);
	unsigned long tmp;

	if (offset >= size)
		return size;
	size -= result;
	offset %= NPD_BITS_PER_LONG;
	if (offset) {
		tmp = *(p++);
		tmp |= ~0UL >> (NPD_BITS_PER_LONG - offset);
		if (size < NPD_BITS_PER_LONG)
			goto found_first;
		if (~tmp)
			goto found_middle;
		size -= NPD_BITS_PER_LONG;
		result += NPD_BITS_PER_LONG;
	}
	while (size & ~(NPD_BITS_PER_LONG-1)) {
		if (~(tmp = *(p++)))
			goto found_middle;
		result += NPD_BITS_PER_LONG;
		size -= NPD_BITS_PER_LONG;
	}
	if (!size)
		return result;
	tmp = *p;

found_first:
	tmp |= ~0UL << size;
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size;	/* Nope. */
found_middle:
	return result + npd_ffz(tmp);
}
/*
 * Find the first set bit in a memory region.
 */
unsigned long npd_find_first_bit(const unsigned long *addr, unsigned long size)
{
	const unsigned long *p = addr;
	unsigned long result = 0;
	unsigned long tmp;

	while (size & ~(NPD_BITS_PER_LONG-1)) {
		if ((tmp = *(p++)))
			goto found;
		result += NPD_BITS_PER_LONG;
		size -= NPD_BITS_PER_LONG;
	}
	if (!size)
		return result;

	tmp = (*p) & (~0UL >> (NPD_BITS_PER_LONG - size));
	if (tmp == 0UL)		/* Are any bits set? */
		return result + size;	/* Nope. */
found:
	return result + __npd_ffs(tmp);
}

/*
 * Find the first cleared bit in a memory region.
 */
unsigned long npd_find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	const unsigned long *p = addr;
	unsigned long result = 0;
	unsigned long tmp;

	while (size & ~(NPD_BITS_PER_LONG-1)) {
		if (~(tmp = *(p++)))
			goto found;
		result += NPD_BITS_PER_LONG;
		size -= NPD_BITS_PER_LONG;
	}
	if (!size)
		return result;

	tmp = (*p) | (~0UL << size);
	if (tmp == ~0UL)	/* Are any bits zero? */
		return result + size;	/* Nope. */
found:
	return result + npd_ffz(tmp);
}


int vbmp_2_vlanlist_str(npd_vbmp_t bmp, char *string)
{
    char vlan_str[20];
    int vlan = 0, start_vlan = 0, end_vlan = 0;
    
    NPD_VBMP_ITER(bmp, vlan)
    {
        if (0 == start_vlan)
        {
            start_vlan = vlan;
            end_vlan = vlan;
            continue;
        }

        if (vlan > end_vlan+1)
        {
            if (start_vlan == end_vlan)
                sprintf(vlan_str,"%d,",start_vlan);
            else
                sprintf(vlan_str, "%d-%d,", start_vlan, end_vlan);

            if (strlen(string)+strlen(vlan_str) < 255)
                strcat(string, vlan_str);
            else
                break;

            start_vlan = vlan;
            end_vlan = vlan;
        }
        else
            end_vlan++;
    }

    if (0 != start_vlan)
    {
        if (start_vlan != end_vlan)
        {
            sprintf(vlan_str, "%d-%d", start_vlan, end_vlan);
        }
        else
            sprintf(vlan_str, "%d", start_vlan);

        if (strlen(string)+strlen(vlan_str) < 255)
        {
            strcat(string, vlan_str);
        }

        if (string[strlen(string)] == ',')
            string[strlen(string)] = 0;
    }
    else
    {
        string[strlen(string)] = 0;
    }

    return 0;
}
#define STATE_VS 0
#define STATE_VE 1
int vlanlist_str_2_vbmp(const char *string, npd_vbmp_t bmp)
{
    unsigned long state = STATE_VS;
    char digit_temp[ 12 ];
    unsigned long vlan_list[ NPD_VBMP_VLAN_MAX];
    unsigned long vlan_s = 0;
    unsigned long vlan_e = 0;
    char cToken;
    unsigned long iflist_i = 0;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long vid;
    unsigned long ulListLen = 0;
    char * list;
    char *endptr = NULL;

    memset(vlan_list, 0, NPD_VBMP_VLAN_MAX * 4 );
    ulListLen = strlen( string );
    list = malloc( ulListLen + 2);
    if ( list == NULL )
    {
        return -1;
    }
    strncpy( list, string, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( state )
        {
               
            case STATE_VS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 5 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);

                    vid = vlan_s;
                    if (( 0 < vlan_s ) && (vlan_s < 4095))
                    {
                        vlan_list[ iflist_i ] = vid;
                        NPD_VBMP_VLAN_ADD(bmp, vid);
                        iflist_i++;
                    }
                    else
                        goto error;
                    temp_i = 0;
                    state = STATE_VS;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_s = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_VE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_VE:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 5 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    unsigned long i;
                    unsigned long i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    vlan_e = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if ( vlan_e > vlan_s )
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    else
                    {
                        i_s = vlan_s;
                        i_e = vlan_e;
                    }
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        vid = i;
                        if (( 1 < vid)&&(vid < 4096))
                        {
                            vlan_list[ iflist_i ] = vid;
                            NPD_VBMP_VLAN_ADD(bmp, vid);
                            iflist_i++;
							if (vid == 4095)
							{
								goto error;
							}
                        } 
                    }
                    temp_i = 0;
                    state = STATE_VS;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    free( list );
    return 0;
error:
    free( list );
    return -1;    
}

int pbmp_2_portlist_str(npd_pbmp_t bmp, char *string)
{
	unsigned int count = 0;
    unsigned int array_index;
	char port_name[64] = {0};
	char port_string[64] = {0};
	unsigned int eth_g_index = 0;
    int start_port = 0;
    int end_port = 0;
    int end_array_id = 0;
	

	NPD_PBMP_ITER(bmp, array_index)
	{
		eth_g_index = netif_array_index_to_ifindex(array_index);
        count++;
        if(0 == start_port)
        {
            start_port = end_port = eth_g_index;
            end_array_id = array_index;
            continue;
        }
        if(end_array_id == array_index-1)
        {
            int slot1 = npd_netif_eth_get_slot(end_port);
            int slot2 = npd_netif_eth_get_slot(eth_g_index);
            if(slot1 == slot2)
            {
                end_port = eth_g_index;
                end_array_id = array_index;
                continue;
            }
        }
        if(start_port == end_port)
        {
		    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s, ", port_string, port_name);
        }
        else
        {
		    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s-",port_string, port_name);
            parse_eth_index_to_name(end_port, port_name);
            sprintf(port_string,"%s%s, ",port_string, port_name);
        }                
        start_port = end_port = eth_g_index;
        end_array_id = array_index;
        if(strlen(port_string)+15 < 39)
            continue;
        else
        {
            strcat(string, port_string);
            strcat(string, "\n");
            memset(port_string, 0, sizeof(port_string));
        }
	}
    if(start_port)
    {
        if(start_port == end_port)
        {
    	    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s", port_string, port_name);
        }
        else
        {
    	    parse_eth_index_to_name(start_port, port_name);
            sprintf(port_string,"%s%s-",port_string, port_name);
            parse_eth_index_to_name(end_port, port_name);
            sprintf(port_string,"%s%s",port_string, port_name);
        }                
        strcat(string, port_string);
        strcat(string,"\n");
        memset(port_string, 0, sizeof(port_string));
    }
    strcat(string, "\n");
    return 0;
}

#define STATE_S 1
#define STATE_SS 2
#define STATE_PS 3
#define STATE_PE 4
int portlist_str_2_pbmp(const char *string, npd_pbmp_t bmp)
{
    unsigned long state = STATE_S;
    unsigned long slot = 0;
    unsigned long chassis = 0;
    unsigned long sub_slot = 0;
    char digit_temp[ 12 ];
    unsigned long ulInterfaceList[ NPD_PBMP_PORT_MAX];
    unsigned long ulPortS = 0;
    unsigned long ulPortE = 0;
    char cToken;
    unsigned long iflist_i = 0;
    unsigned long list_i = 0;
    unsigned long temp_i = 0;
    unsigned long ulIfindex;
    unsigned long ulListLen = 0;
    char * list;
    char *endptr = NULL;

    memset(ulInterfaceList, 0, NPD_PBMP_PORT_MAX * 4 );
    ulListLen = strlen( string );
    list = malloc( ulListLen + 2);
    if ( list == NULL )
    {
        return -1;
    }
    strncpy( list, string, ulListLen + 1 );
    list[ ulListLen ] = ',';
    list[ ulListLen + 1 ] = '\0';

    cToken = list[ list_i ];

    while ( cToken != 0 )
    {
        switch ( state )
        {
            case STATE_S:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == '/' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    chassis = (unsigned long)strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    
                    state = STATE_SS;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_SS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == '/' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    slot = (unsigned long)strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    
                    state = STATE_PS;
                } 
                /*we support slot/port expression, so give a decide wether it is*/
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    chassis = 0;
                    slot = chassis;

                    ulIfindex = generate_eth_index(chassis, slot, 0, ulPortS, 0);
                    if ( 0 != ulIfindex )
                    {
                        int array_index;
                        ulInterfaceList[ iflist_i ] = ulIfindex;
                        array_index = netif_array_index_from_ifindex(ulIfindex);
                        NPD_PBMP_PORT_ADD(bmp, array_index);
                        iflist_i++;
                    }
                    if ( iflist_i >= NPD_PBMP_PORT_MAX )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    chassis = 0;
                    slot = chassis;
                    
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
                
            case STATE_PS:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if (cToken == '/')
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    sub_slot = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    /*for box system, we use chassis no as slot to generate eth index*/
                    temp_i = 0;
                    state = STATE_PS;                    
                }
                else if ( cToken == ',' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);

                    ulIfindex = generate_eth_index(chassis, slot, sub_slot, ulPortS, 0);
                    if ( 0 != ulIfindex )
                    {
                        unsigned int array_index;
                        ulInterfaceList[ iflist_i ] = ulIfindex;
                        array_index = netif_array_index_from_ifindex(ulIfindex);
                        NPD_PBMP_PORT_ADD(bmp, array_index);
                        iflist_i++;
                    }
                    if ( iflist_i >= NPD_PBMP_PORT_MAX )
                    {
                        goto error;
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else if ( cToken == '-' )
                {
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortS = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    temp_i = 0;
                    state = STATE_PE;
                }
                else
                {
                    goto error;
                }
                break;
            case STATE_PE:
                if ( isdigit( cToken ) )
                {
                    digit_temp[ temp_i ] = cToken;
                    temp_i++;
                    if ( temp_i >= 11 )
                    {
                        goto error;
                    }
                }
                else if ( cToken == ',' )
                {
                    unsigned long i;
                    unsigned long i_s, i_e;
                    if ( temp_i == 0 )
                    {
                        goto error;
                    }
                    digit_temp[ temp_i ] = 0;
                    ulPortE = ( unsigned long )strtoul(digit_temp,&endptr,10);
                    if ( ulPortE > ulPortS )
                    {
                        i_s = ulPortS;
                        i_e = ulPortE;
                    }
                    else
                    {
                        i_s = ulPortE;
                        i_e = ulPortS;
                    }
                    /*for box system, we use chassis no as slot to generate eth index*/
                    for ( i = i_s;i <= i_e;i++ )
                    {
                        ulIfindex = generate_eth_index(chassis, slot, sub_slot, i, 0);
                        if ( 0 != ulIfindex )
                        {
                            unsigned int array_index;
                            ulInterfaceList[ iflist_i ] = ulIfindex;
                            array_index = netif_array_index_from_ifindex(ulIfindex);
                            NPD_PBMP_PORT_ADD(bmp, array_index);
                            iflist_i++;
                        }
                        if ( iflist_i >= NPD_PBMP_PORT_MAX )
                        {
                            goto error;
                        }
                    }
                    temp_i = 0;
                    state = STATE_S;
                }
                else
                {
                    goto error;
                }
                break;
            default:
                goto error;
        }
        list_i++;
        cToken = list[ list_i ];
    }
    free( list );
    return 0;
error:
    if(list)
        free( list );
    return -1;    
}


