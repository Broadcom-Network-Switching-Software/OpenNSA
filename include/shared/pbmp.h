/***************************************************************************
 * 
 * This software is governed by the Broadcom Switch APIs license.
 * This license is set out in https://raw.githubusercontent.com/Broadcom-Network-Switching-Software/OpenNSA/master/Legal/LICENSE file.
 * 
 * Copyright 2005-2020 Broadcom Inc. All rights reserved.
 * 
 ***************************************************************************
 *
 * Port Bitmap Macros
 */

#ifndef _SHR_PBMP_H
#define _SHR_PBMP_H

#include <shared/types.h>

/*
 * Port Bitmap Macros
 *
 * These macros are never used directly (well, except in the C code
 * support routines that are used to help implement the macros).
 * For many macro here, there are BCM_* and SOC_* versions that are
 * defined in <bcm/types.h> and <soc/types.h>. SDK and customer code
 * should use those macros, not these.
 *
 * This header requires that the uint32 type be defined before inclusion.
 * Using <sal/types.h> is the simplest (and recommended) way of doing
 * this.
 *
 * There following macros are made available.  All have names starting
 * with _SHR_, which have been elided from this list:
 *
 * Constants or Expressions suitable for assignment:
 * PBMP_PORT_MAX		the maximum number of ports supported
 * PBMP_WORD_MAX		the maximum number of words in a bitmap
 * PBMP_WORD_GET(b, w)		return word w (0..WORD_MAX-1) from bitmap b
 * PBMP_FMT_LEN			length of char[] array for PBMP_FMT
 * PBMP_FMT(b,s)		%s'able format of bitmap b into string s
 *				returns s
 *
 * Predicates: (return 0 or 1, suitable for using in if statements)
 * PBMP_MEMBER(b, p)		is port p a member of bitmap b?
 * PBMP_IS_NULL(b)		is bitmap b empty?
 * PBMP_NOT_NULL(b)		is bitmap b not empty?
 * PBMP_EQ(b1, b2)		are bitmaps b1 and b2 equal?
 * PBMP_NEQ(b1, b2)		are bitmaps b1 and b2 not equal?
 * PBMP_PORT_VALID(p)		is p a valid port number? (0..PORT_MAX)
 *
 * Statements: (cannot be used as a predicate)
 * PBMP_CLEAR(b)		zero out a bitmap
 * PBMP_COUNT(b, c)		store how many bits are on in b into integer c
 * PBMP_ITER(b, p) {...}	iterate over bitmap b, setting p to each bit
 * PBMP_ASSIGN(b1, b2)		copy bitmap b2 into b1
 * PBMP_AND(b1, b2)		and bitmap b2 into b1
 * PBMP_OR(b1, b2)		or bitmap b2 into b1
 * PBMP_XOR(b1, b2)		exclusive or bitmap b2 into b1
 * PBMP_REMOVE(b1, b2)		remove bits in bitmap b2 from b1
 * PBMP_NEGATE(b1, b2)		copy the bitwise negation of bitmap b2 into b1
 * PBMP_PORT_SET(b, p)		clear bitmap b, then turn bit p on
 * PBMP_PORT_ADD(b, p)		turn bit p on in bitmap b
 * PBMP_PORT_REMOVE(b, p)	turn bit p off in bitmap b
 * PBMP_PORT_FLIP(b, p)		flip the sense of bit p on in bitmap b
 * PBMP_WORD_SET(b, w, v)	set word w (0..WORD_MAX-1) from bitmap b to v
 *
 * Internal forms: (should not be used outside of this header file)
 * PBMP_WENT(p)			word index for bit p
 * PBMP_WBIT(p)			word bitmask for bit p
 * PBMP_BMCLEAR(b)		clear a bitmap
 * PBMP_BMNULL(b)		is bitmap b empty?
 * PBMP_BMEQ(b1, b2)		are bitmaps b1 and b2 equal?
 * PBMP_BMOP(b1, b2, op)	execute op on a word basis on bitmaps b1, b2
 * PBMP_ENTRY(b, p)		the word of bitmap b that holds bit p
 *
 * There are 4 styles of implementation for the bitmap macros supported:
 * 1. The historic single uint32 implementation (-D_SHR_PBMP_WIDTH=0)
 *    This is expected to be removed after a decent time of mourning.
 * 2. An array of a single uint32 in a struct (-D_SHR_PBMP_WIDTH=32,
 *    the default).  Macros are special cased to handle this efficiently.
 * 3. An array of two uint32 in a struct (-D_SHR_PBMP_WIDTH=64, used
 *    when BCM_TUCANA_SUPPORT is defined).  Macros are again special
 *    cased for efficiency.
 * 4. An array of more than two uint32 in a struct (-D_SHR_PBMP_WIDTH=256
 *    for example).  Most things are done inline, with a couple of helper
 *    functions used to implement some macros.
 */

#ifndef	_SHR_PBMP_WIDTH
/*
 * Could get this from <soc/mcm/allenum.h> (SOC_MAX_NUM_PORTS)
 * but that makes every file include allenum.h and grow somewhat
 * large...
 */
#ifdef	BCM_ESW_SUPPORT
#define	_SHR_PBMP_PORT_MAX	256
#else
#define	_SHR_PBMP_PORT_MAX      64
#endif  /* BCM_ESW_SUPPORT */

#ifdef BCM_PETRA_SUPPORT
#if 571 > _SHR_PBMP_PORT_MAX
#undef   _SHR_PBMP_PORT_MAX
#define  _SHR_PBMP_PORT_MAX 571
#endif
#endif /* BCM_PETRA_SUPPORT */

#ifdef BCM_DNX_SUPPORT
#if 1280 > _SHR_PBMP_PORT_MAX
#undef   _SHR_PBMP_PORT_MAX
#define  _SHR_PBMP_PORT_MAX 1280
#endif
#endif /* BCM_PETRA_SUPPORT */
#ifdef BCM_DFE_SUPPORT
#if 145 > _SHR_PBMP_PORT_MAX
#undef   _SHR_PBMP_PORT_MAX
#define  _SHR_PBMP_PORT_MAX 145
#endif
#endif /* BCM_DFE_SUPPORT */

#ifdef BCM_DNXF_SUPPORT
#if 193 > _SHR_PBMP_PORT_MAX
#undef _SHR_PBMP_PORT_MAX
#define _SHR_PBMP_PORT_MAX 193
#endif
#endif /* BCM_DNXF_SUPPORT */

#ifdef BCM_LTSW_SUPPORT
#if 320 > _SHR_PBMP_PORT_MAX
#undef   _SHR_PBMP_PORT_MAX
#define  _SHR_PBMP_PORT_MAX 320
#endif
#endif /* BCM_LTSW_SUPPORT */

#define	_SHR_PBMP_WIDTH		(((_SHR_PBMP_PORT_MAX + 32 - 1)/32)*32)
#endif

#if _SHR_PBMP_WIDTH == 0		/* old style */
typedef uint32 _shr_pbmp_t;

#ifndef	_SHR_PBMP_PORT_MAX
#define	_SHR_PBMP_PORT_MAX		32
#endif
#define	_SHR_PBMP_WORD_MAX		1
#define	_SHR_PBMP_WBIT(port)		(1U<<(port))
#define	_SHR_PBMP_WORD_GET(pbm, word)	(pbm)
#define	_SHR_PBMP_WORD_SET(pbm, word, val)	((pbm) = (val))

#define _SHR_PBMP_CLEAR(pbm)		((pbm) = 0)
#define _SHR_PBMP_MEMBER(bmp, port)	(((bmp) & _SHR_PBMP_WBIT(port)) != 0)
#define	_SHR_PBMP_COUNT(bmp, count)	(count = _shr_popcount(bmp))
#define _SHR_PBMP_ITER(bmp, port) \
        for ((port) = 0; (port) < 32; (port)++) \
            if (_SHR_PBMP_MEMBER((bmp), (port)))

#define _SHR_PBMP_REVERSE_ITER(bmp, port) \
        for ((port) = 31; (port) > -1; (port)--) \
            if (_SHR_PBMP_MEMBER((bmp), (port)))

#define _SHR_PBMP_IS_NULL(pbm)           ((pbm) == 0)
#define _SHR_PBMP_NOT_NULL(pbm)          ((pbm) != 0)
#define _SHR_PBMP_EQ(pbm_a, pbm_b)       ((pbm_a) == (pbm_b))
#define _SHR_PBMP_NEQ(pbm_a, pbm_b)      ((pbm_a) != (pbm_b))

/* Assignment operators */
#define _SHR_PBMP_ASSIGN(dst, src)       (dst) = (src)
#define _SHR_PBMP_AND(pbm_a, pbm_b)      ((pbm_a) &= (pbm_b))
#define _SHR_PBMP_OR(pbm_a, pbm_b)       ((pbm_a) |= (pbm_b))
#define _SHR_PBMP_XOR(pbm_a, pbm_b)      ((pbm_a) ^= (pbm_b))
#define _SHR_PBMP_REMOVE(pbm_a, pbm_b)   ((pbm_a) &= ~(pbm_b))
#define _SHR_PBMP_NEGATE(pbm_a, pbm_b)   ((pbm_a) = ~(pbm_b))

/* Port PBMP operators */
#define _SHR_PBMP_PORT_SET(pbm, port)    ((pbm) = (1U << (port)))
#define _SHR_PBMP_PORT_ADD(pbm, port)    ((pbm) |= (1U << (port)))
#define _SHR_PBMP_PORT_REMOVE(pbm, port) ((pbm) &= ~(1U << (port)))
#define _SHR_PBMP_PORT_FLIP(pbm, port)	 ((pbm) ^= (1U << (port)))

#define _SHR_PBMP_PORTS_RANGE_ADD(bm, first_port, range) \
    do {\
        uint32 _mask_ = ~0;\
        _mask_ >>= (_SHR_PBMP_PORT_MAX - _range_);\
        _mask_ <<= (_first_port_ % _SHR_PBMP_WORD_WIDTH);\
        _SHR_PBMP_ENTRY(bm, _first_port_) |= _mask_; \
    } while (0); 

#else	/* _SHR_PBMP_WIDTH == 0 */

#ifndef	_SHR_PBMP_PORT_MAX
#define	_SHR_PBMP_PORT_MAX		_SHR_PBMP_WIDTH
#endif
#define	_SHR_PBMP_WORD_WIDTH		32
#define	_SHR_PBMP_WORD_MAX		\
	((_SHR_PBMP_WIDTH + _SHR_PBMP_WORD_WIDTH-1) / _SHR_PBMP_WORD_WIDTH)

typedef struct _shr_pbmp {
	uint32	pbits[_SHR_PBMP_WORD_MAX];
} _shr_pbmp_t;

#define	_SHR_PBMP_WORD_GET(bm, word)		((bm).pbits[(word)])
#define	_SHR_PBMP_WORD_SET(bm, word, val)	((bm).pbits[(word)] = (val))

/*
 * Common cases are one word (1..32 ports) and two words (33..64 ports).
 * If not the common cases, more complicated code is generated using helper
 * functions.
 */
#if	_SHR_PBMP_WORD_MAX == 1		/* 32 bit maps */
#define	_SHR_PBMP_WENT(port)		(0)
#define	_SHR_PBMP_WBIT(port)		(1U<<(port))

/* helper defines used in the generic section below */
#define _SHR_PBMP_BMCLEAR(bm)		(_SHR_PBMP_WORD_GET(bm, 0) = 0)
#define _SHR_PBMP_BMNULL(bm)		(_SHR_PBMP_WORD_GET(bm, 0) == 0)
#define _SHR_PBMP_BMEQ(bma, bmb)	\
	(_SHR_PBMP_WORD_GET(bma, 0) == _SHR_PBMP_WORD_GET(bmb, 0))
#define _SHR_PBMP_BMOP(bma, bmb, op)	do { \
		_SHR_PBMP_WORD_GET(bma, 0) op _SHR_PBMP_WORD_GET(bmb, 0); \
	} while (0)
#define	_SHR_PBMP_COUNT(bm, count)	\
	(count = _shr_popcount(_SHR_PBMP_WORD_GET(bm, 0)))

#elif	_SHR_PBMP_WORD_MAX == 2		/* 64 bit maps */
#define	_SHR_PBMP_WENT(port)		((port)/_SHR_PBMP_WORD_WIDTH)
#define	_SHR_PBMP_WBIT(port)		(1U<<((port) % _SHR_PBMP_WORD_WIDTH))

#define _SHR_PBMP_BMCLEAR(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) = _SHR_PBMP_WORD_GET(bm, 1) = 0)
#define _SHR_PBMP_BMNULL(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) == 0 && _SHR_PBMP_WORD_GET(bm, 1) == 0)
#define _SHR_PBMP_BMEQ(bma, bmb)	\
	((_SHR_PBMP_WORD_GET(bma, 0) == _SHR_PBMP_WORD_GET(bmb, 0)) && \
	 (_SHR_PBMP_WORD_GET(bma, 1) == _SHR_PBMP_WORD_GET(bmb, 1)))
#define _SHR_PBMP_BMOP(bma, bmb, op)	do { \
	_SHR_PBMP_WORD_GET(bma, 0) op _SHR_PBMP_WORD_GET(bmb, 0); \
	_SHR_PBMP_WORD_GET(bma, 1) op _SHR_PBMP_WORD_GET(bmb, 1); \
	} while (0)
#define	_SHR_PBMP_COUNT(bm, count)	\
	(count = _shr_popcount(_SHR_PBMP_WORD_GET(bm, 0)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 1)))

#elif	_SHR_PBMP_WORD_MAX == 3		/* 96 bit maps */
#define	_SHR_PBMP_WENT(port)		((port)/_SHR_PBMP_WORD_WIDTH)
#define	_SHR_PBMP_WBIT(port)		(1U << ((port) % _SHR_PBMP_WORD_WIDTH))

#define _SHR_PBMP_BMCLEAR(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) = _SHR_PBMP_WORD_GET(bm, 1) = \
         _SHR_PBMP_WORD_GET(bm, 2) = 0)
#define _SHR_PBMP_BMNULL(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) == 0 && _SHR_PBMP_WORD_GET(bm, 1) == 0 && \
         _SHR_PBMP_WORD_GET(bm, 2) == 0)
#define _SHR_PBMP_BMEQ(bma, bmb)	\
	((_SHR_PBMP_WORD_GET(bma, 0) == _SHR_PBMP_WORD_GET(bmb, 0)) && \
	 (_SHR_PBMP_WORD_GET(bma, 1) == _SHR_PBMP_WORD_GET(bmb, 1)) && \
	 (_SHR_PBMP_WORD_GET(bma, 2) == _SHR_PBMP_WORD_GET(bmb, 2)))
#define _SHR_PBMP_BMOP(bma, bmb, op)	do { \
	_SHR_PBMP_WORD_GET(bma, 0) op _SHR_PBMP_WORD_GET(bmb, 0); \
	_SHR_PBMP_WORD_GET(bma, 1) op _SHR_PBMP_WORD_GET(bmb, 1); \
	_SHR_PBMP_WORD_GET(bma, 2) op _SHR_PBMP_WORD_GET(bmb, 2); \
	} while (0)
#define	_SHR_PBMP_COUNT(bm, count)	\
	(count = _shr_popcount(_SHR_PBMP_WORD_GET(bm, 0)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 1)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 2)))

#elif	_SHR_PBMP_WORD_MAX == 5		/* 160 bit maps */
#define	_SHR_PBMP_WENT(port)		((port)/_SHR_PBMP_WORD_WIDTH)
#define	_SHR_PBMP_WBIT(port)		(1U << ((port) % _SHR_PBMP_WORD_WIDTH))

#define _SHR_PBMP_BMCLEAR(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) = _SHR_PBMP_WORD_GET(bm, 1) = \
	 _SHR_PBMP_WORD_GET(bm, 2) = _SHR_PBMP_WORD_GET(bm, 3) = \
         _SHR_PBMP_WORD_GET(bm, 4) = 0)
#define _SHR_PBMP_BMNULL(bm)		\
	(_SHR_PBMP_WORD_GET(bm, 0) == 0 && _SHR_PBMP_WORD_GET(bm, 1) == 0 && \
         _SHR_PBMP_WORD_GET(bm, 2) == 0 && _SHR_PBMP_WORD_GET(bm, 3) == 0 && \
         _SHR_PBMP_WORD_GET(bm, 4) == 0)
#define _SHR_PBMP_BMEQ(bma, bmb)	\
	((_SHR_PBMP_WORD_GET(bma, 0) == _SHR_PBMP_WORD_GET(bmb, 0)) && \
	 (_SHR_PBMP_WORD_GET(bma, 1) == _SHR_PBMP_WORD_GET(bmb, 1)) && \
	 (_SHR_PBMP_WORD_GET(bma, 2) == _SHR_PBMP_WORD_GET(bmb, 2)) && \
	 (_SHR_PBMP_WORD_GET(bma, 3) == _SHR_PBMP_WORD_GET(bmb, 3)) && \
	 (_SHR_PBMP_WORD_GET(bma, 4) == _SHR_PBMP_WORD_GET(bmb, 4)))
#define _SHR_PBMP_BMOP(bma, bmb, op)	do { \
	_SHR_PBMP_WORD_GET(bma, 0) op _SHR_PBMP_WORD_GET(bmb, 0); \
	_SHR_PBMP_WORD_GET(bma, 1) op _SHR_PBMP_WORD_GET(bmb, 1); \
	_SHR_PBMP_WORD_GET(bma, 2) op _SHR_PBMP_WORD_GET(bmb, 2); \
	_SHR_PBMP_WORD_GET(bma, 3) op _SHR_PBMP_WORD_GET(bmb, 3); \
	_SHR_PBMP_WORD_GET(bma, 4) op _SHR_PBMP_WORD_GET(bmb, 4); \
	} while (0)
#define	_SHR_PBMP_COUNT(bm, count)	\
	(count = _shr_popcount(_SHR_PBMP_WORD_GET(bm, 0)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 1)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 2)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 3)) + \
		 _shr_popcount(_SHR_PBMP_WORD_GET(bm, 4)))

#else	/* _SHR_PBMP_WORD_MAX == 4 || > 5 */	/* 144 or > 160 bit maps */

/* For use by pbmp.c */
#define _SHR_DEFINE_PBMP_FUNCTIONS

extern int	_shr_pbmp_bmnull(_shr_pbmp_t *);
extern int	_shr_pbmp_bmeq(_shr_pbmp_t *, _shr_pbmp_t *);

#define	_SHR_PBMP_WENT(port)		((port)/_SHR_PBMP_WORD_WIDTH)
#define	_SHR_PBMP_WBIT(port)		(1U<<((port) % _SHR_PBMP_WORD_WIDTH))

#define _SHR_PBMP_BMCLEAR(bm)		do { \
		int	_w; \
		for (_w = 0; _w < _SHR_PBMP_WORD_MAX; _w++) { \
			_SHR_PBMP_WORD_GET(bm, _w) = 0; \
		} \
	} while (0)
#define _SHR_PBMP_BMNULL(bm)		(_shr_pbmp_bmnull(&bm))
#define _SHR_PBMP_BMEQ(bma, bmb)	(_shr_pbmp_bmeq(&bma, &bmb))
#define _SHR_PBMP_BMOP(bma, bmb, op)	do { \
		int	_w; \
		for (_w = 0; _w < _SHR_PBMP_WORD_MAX; _w++) { \
			_SHR_PBMP_WORD_GET(bma, _w) op _SHR_PBMP_WORD_GET(bmb, _w); \
		} \
	} while (0)
#define	_SHR_PBMP_COUNT(bm, count)	do { \
		int	_w; \
		count = 0; \
		for (_w = 0; _w < _SHR_PBMP_WORD_MAX; _w++) { \
			count += _shr_popcount(_SHR_PBMP_WORD_GET(bm, _w)); \
		} \
	} while(0)

#endif	/* _SHR_PBMP_WORD_MAX */

/* generics that use the previously defined helpers */
#define _SHR_PBMP_CLEAR(bm)		_SHR_PBMP_BMCLEAR(bm)
#define _SHR_PBMP_ITER(bm, port)	\
	for ((port) = 0; (port) < _SHR_PBMP_PORT_MAX; (port)++) \
		if (_SHR_PBMP_MEMBER((bm), (port)))

#define _SHR_PBMP_REVERSE_ITER(bm, port)	\
	for ((port) = _SHR_PBMP_PORT_MAX - 1; (port) > -1; (port)--) \
		if (_SHR_PBMP_MEMBER((bm), (port)))

#define _SHR_PBMP_IS_NULL(bm)		(_SHR_PBMP_BMNULL(bm))
#define _SHR_PBMP_NOT_NULL(bm)		(!_SHR_PBMP_BMNULL(bm))
#define _SHR_PBMP_EQ(bma, bmb)		(_SHR_PBMP_BMEQ(bma, bmb))
#define _SHR_PBMP_NEQ(bma, bmb)		(!_SHR_PBMP_BMEQ(bma, bmb))

/* Assignment operators */
#define _SHR_PBMP_ASSIGN(dst, src)	(dst) = (src)
#define _SHR_PBMP_AND(bma, bmb)		_SHR_PBMP_BMOP(bma, bmb, &=)
#define _SHR_PBMP_OR(bma, bmb)		_SHR_PBMP_BMOP(bma, bmb, |=)
#define _SHR_PBMP_XOR(bma, bmb)		_SHR_PBMP_BMOP(bma, bmb, ^=)
#define _SHR_PBMP_REMOVE(bma, bmb)	_SHR_PBMP_BMOP(bma, bmb, &= ~)
#define _SHR_PBMP_NEGATE(bma, bmb)	_SHR_PBMP_BMOP(bma, bmb, = ~)

/* Port PBMP operators */
#define	_SHR_PBMP_FIRST(bm, first_port)	\
    do {\
	    _SHR_PBMP_ITER(bm, first_port) {break;} \
        if (first_port == _SHR_PBMP_PORT_MAX) first_port = -1; \
    } while(0)

#define	_SHR_PBMP_LAST(bm, last_port)	\
    do {\
	    _SHR_PBMP_REVERSE_ITER(bm, last_port) {break;} \
    } while(0)

    
#define	_SHR_PBMP_ENTRY(bm, port)	\
	(_SHR_PBMP_WORD_GET(bm,_SHR_PBMP_WENT(port)))
#define _SHR_PBMP_MEMBER(bm, port)	\
	((_SHR_PBMP_ENTRY(bm, port) & _SHR_PBMP_WBIT(port)) != 0)
#define _SHR_PBMP_PORT_SET(bm, port)	do { \
		_SHR_PBMP_CLEAR(bm); \
		_SHR_PBMP_PORT_ADD(bm, port); \
	} while(0)
#define _SHR_PBMP_PORT_ADD(bm, port)	\
	(_SHR_PBMP_ENTRY(bm, port) |= _SHR_PBMP_WBIT(port))
#define _SHR_PBMP_PORT_REMOVE(bm, port)	\
	(_SHR_PBMP_ENTRY(bm, port) &= ~_SHR_PBMP_WBIT(port))
#define _SHR_PBMP_PORT_FLIP(bm, port)	\
	(_SHR_PBMP_ENTRY(bm, port) ^= _SHR_PBMP_WBIT(port))

#define _SHR_PBMP_PORTS_RANGE_ADD(bm, first_port, range) \
    do {\
        uint32 _mask_;\
        int _first_port_, _range_;\
        _first_port_ = first_port; _range_ = range;\
        while (_range_ > 0) {\
            _mask_ = ~0;\
            if (_range_ < _SHR_PBMP_WORD_WIDTH) _mask_ >>= (_SHR_PBMP_WORD_WIDTH - _range_);\
            _mask_ <<= (_first_port_ % _SHR_PBMP_WORD_WIDTH);\
            _SHR_PBMP_ENTRY(bm, _first_port_) |= _mask_; \
            _range_ += (_first_port_ % _SHR_PBMP_WORD_WIDTH) - _SHR_PBMP_WORD_WIDTH;\
            _first_port_ +=  _SHR_PBMP_WORD_WIDTH - (_first_port_ % _SHR_PBMP_WORD_WIDTH);\
        }\
    } while (0); 
        

#endif	/* _SHR_PBMP_WIDTH == 0 */

extern int _shr_pbmp_parse(char *s, _shr_pbmp_t *pbmp, uint32 *arr, uint8 *is_hex);
extern char		*_shr_pbmp_format(_shr_pbmp_t, char *);
char            *shr_pbmp_range_format(_shr_pbmp_t bmp, char *buf, int buf_size);
extern int		_shr_pbmp_decode(char *, _shr_pbmp_t *);

#define	_SHR_PBMP_FMT(bm, buf)		_shr_pbmp_format(bm, buf)
#define	_SHR_PBMP_FMT_LEN		((_SHR_PBMP_WORD_MAX*8)+3)

#define	_SHR_PBMP_PORT_VALID(p)		((p) >= 0 && (p) < _SHR_PBMP_PORT_MAX)

#endif	/* !_SHR_PBMP_H */
