/*
 * name hash index functions.
 * Copyright (C) 2010 Chandler Ren
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2, or (at your
 * option) any later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _QUAGGA_NAME_HASH_H
#define _QUAGGA_NAME_HASH_H

static inline unsigned int name_hash(char *name, int size)
{
    unsigned long ret=0;
	long n;
	unsigned long v;
	int r;
    char *c = name;

	if ((c == NULL) || (*c == '\0'))
		return(ret);

	n=0x100;
	while (*c)
		{
		v=n|(*c);
		n+=0x100;
		r= (int)((v>>2)^v)&0x0f;
		ret=(ret<<r)|(ret>>(32-r));
		ret&=0xFFFFFFFFL;
		ret^=v*v;
		c++;
		}
	return((ret>>16)^ret)%size;
}

static inline unsigned int name_hash_name_cmp(void *data1, void *data2)
{
    char *entry1 = 
        (char*)data1;
    char *entry2 = 
        (char*)data2;

    return (0 == strcmp(entry1, entry2));
}

#endif /* _QUAGGA_NAME_HASH_H */

