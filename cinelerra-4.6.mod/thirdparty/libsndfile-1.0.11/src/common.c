/*
** Copyright (C) 1999-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include	<stdarg.h>
#include	<string.h>
#include	<ctype.h>
#include	<math.h>
#include	<time.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"

/*-----------------------------------------------------------------------------------------------
** psf_log_printf allows libsndfile internal functions to print to an internal logbuffer which
** can later be displayed.
** The format specifiers are as for printf but without the field width and other modifiers.
** Printing is performed to the logbuffer char array of the SF_PRIVATE struct.
** Printing is done in such a way as to guarantee that the log never overflows the end of the
** logbuffer array.
*/

#define LOG_PUTCHAR(a,b)									\
		{	if ((a)->logindex < SIGNED_SIZEOF ((a)->logbuffer) - 1) \
			{	(a)->logbuffer [(a)->logindex++] = (b) ;	\
				(a)->logbuffer [(a)->logindex] = 0 ;		\
				}											\
			}

void
psf_log_printf (SF_PRIVATE *psf, const char *format, ...)
{	va_list			ap ;
	unsigned int	u ;
	int				d, tens, shift, width, width_specifier, left_align ;
	char			c, *strptr, istr [5], lead_char, sign_char ;

	va_start (ap, format) ;

	while ((c = *format++))
	{	if (c != '%')
		{	LOG_PUTCHAR (psf, c) ;
			continue ;
			} ;

		if (format [0] == '%') /* Handle %% */
		{ 	LOG_PUTCHAR (psf, '%') ;
			format ++ ;
			continue ;
			} ;

		sign_char = 0 ;
		left_align = SF_FALSE ;
		while (1)
		{	switch (format [0])
			{	case ' ' :
				case '+' :
					sign_char = format [0] ;
					format ++ ;
					continue ;

				case '-' :
					left_align = SF_TRUE ;
					format ++ ;
					continue ;

				default : break ;
				} ;

			break ;
			} ;

		if (format [0] == 0)
			break ;

		lead_char = ' ' ;
		if (format [0] == '0')
			lead_char = '0' ;

		width_specifier = 0 ;
		while ((c = *format++) && isdigit (c))
			width_specifier = width_specifier * 10 + (c - '0') ;

		switch (c)
		{	case 0 : /* NULL character. */
					va_end (ap) ;
					return ;

			case 's': /* string */
					strptr = va_arg (ap, char *) ;
					if (strptr == NULL)
						break ;
					width_specifier -= strlen (strptr) ;
					if (left_align == SF_FALSE)
						while (width_specifier -- > 0)
							LOG_PUTCHAR (psf, ' ') ;
					while (*strptr)
						LOG_PUTCHAR (psf, *strptr++) ;
					while (width_specifier -- > 0)
						LOG_PUTCHAR (psf, ' ') ;
					break ;

			case 'd': /* int */
					d = va_arg (ap, int) ;

					if (d < 0)
					{	d = -d ;
						sign_char = '-' ;
						if (lead_char != '0' && left_align == SF_FALSE)
							width_specifier -- ;
						} ;

					tens = 1 ;
					width = 1 ;
					while (d / tens >= 10)
					{	tens *= 10 ;
						width ++ ;
						} ;

					width_specifier -= width ;

					if (sign_char == ' ')
					{	LOG_PUTCHAR (psf, ' ') ;
						width_specifier -- ;
						} ;

					if (left_align == SF_FALSE && lead_char != '0')
					{	if (sign_char == '+')
							width_specifier -- ;

						while (width_specifier -- > 0)
							LOG_PUTCHAR (psf, lead_char) ;
						} ;

					if (sign_char == '+' || sign_char == '-')
					{	LOG_PUTCHAR (psf, sign_char) ;
						width_specifier -- ;
						} ;

					if (left_align == SF_FALSE)
						while (width_specifier -- > 0)
							LOG_PUTCHAR (psf, lead_char) ;

					while (tens > 0)
					{	LOG_PUTCHAR (psf, '0' + d / tens) ;
						d %= tens ;
						tens /= 10 ;
						} ;

					while (width_specifier -- > 0)
						LOG_PUTCHAR (psf, lead_char) ;
					break ;

			case 'D': /* sf_count_t */
					{	sf_count_t		D, Tens ;

						D = va_arg (ap, sf_count_t) ;

						if (D == 0)
						{	while (-- width_specifier > 0)
								LOG_PUTCHAR (psf, lead_char) ;
							LOG_PUTCHAR (psf, '0') ;
							break ;
							}
						if (D < 0)
						{	LOG_PUTCHAR (psf, '-') ;
							D = -D ;
							} ;
						Tens = 1 ;
						width = 1 ;
						while (D / Tens >= 10)
						{	Tens *= 10 ;
							width ++ ;
							} ;

						while (width_specifier > width)
						{	LOG_PUTCHAR (psf, lead_char) ;
							width_specifier-- ;
							} ;

						while (Tens > 0)
						{	LOG_PUTCHAR (psf, '0' + D / Tens) ;
							D %= Tens ;
							Tens /= 10 ;
							} ;
						} ;
					break ;

			case 'u': /* unsigned int */
					u = va_arg (ap, unsigned int) ;

					tens = 1 ;
					width = 1 ;
					while (u / tens >= 10)
					{	tens *= 10 ;
						width ++ ;
						} ;

					width_specifier -= width ;

					if (sign_char == ' ')
					{	LOG_PUTCHAR (psf, ' ') ;
						width_specifier -- ;
						} ;

					if (left_align == SF_FALSE && lead_char != '0')
					{	if (sign_char == '+')
							width_specifier -- ;

						while (width_specifier -- > 0)
							LOG_PUTCHAR (psf, lead_char) ;
						} ;

					if (sign_char == '+' || sign_char == '-')
					{	LOG_PUTCHAR (psf, sign_char) ;
						width_specifier -- ;
						} ;

					if (left_align == SF_FALSE)
						while (width_specifier -- > 0)
							LOG_PUTCHAR (psf, lead_char) ;

					while (tens > 0)
					{	LOG_PUTCHAR (psf, '0' + u / tens) ;
						u %= tens ;
						tens /= 10 ;
						} ;

					while (width_specifier -- > 0)
						LOG_PUTCHAR (psf, lead_char) ;
					break ;

			case 'c': /* char */
					c = va_arg (ap, int) & 0xFF ;
					LOG_PUTCHAR (psf, c) ;
					break ;

			case 'x': /* hex */
			case 'X': /* hex */
					d = va_arg (ap, int) ;

					if (d == 0)
					{	while (--width_specifier > 0)
							LOG_PUTCHAR (psf, lead_char) ;
						LOG_PUTCHAR (psf, '0') ;
						break ;
						} ;
					shift = 28 ;
					width = (width_specifier < 8) ? 8 : width_specifier ;
					while (! ((0xF << shift) & d))
					{	shift -= 4 ;
						width -- ;
						} ;

					while (width > 0 && width_specifier > width)
					{	LOG_PUTCHAR (psf, lead_char) ;
						width_specifier-- ;
						} ;

					while (shift >= 0)
					{	c = (d >> shift) & 0xF ;
						LOG_PUTCHAR (psf, (c > 9) ? c + 'A' - 10 : c + '0') ;
						shift -= 4 ;
						} ;
					break ;

			case 'M': /* int2str */
					d = va_arg (ap, int) ;
					if (CPU_IS_LITTLE_ENDIAN)
					{	istr [0] = d & 0xFF ;
						istr [1] = (d >> 8) & 0xFF ;
						istr [2] = (d >> 16) & 0xFF ;
						istr [3] = (d >> 24) & 0xFF ;
						}
					else
					{	istr [3] = d & 0xFF ;
						istr [2] = (d >> 8) & 0xFF ;
						istr [1] = (d >> 16) & 0xFF ;
						istr [0] = (d >> 24) & 0xFF ;
						} ;
					istr [4] = 0 ;
					strptr = istr ;
					while (*strptr)
					{	c = *strptr++ ;
						LOG_PUTCHAR (psf, c) ;
						} ;
					break ;

			default :
					LOG_PUTCHAR (psf, '*') ;
					LOG_PUTCHAR (psf, c) ;
					LOG_PUTCHAR (psf, '*') ;
					break ;
			} /* switch */
		} /* while */

	va_end (ap) ;
	return ;
} /* psf_log_printf */

#ifndef PSF_LOG_PRINTF_ONLY
/*-----------------------------------------------------------------------------------------------
**  ASCII header printf functions.
**  Some formats (ie NIST) use ascii text in their headers.
**  Format specifiers are the same as the standard printf specifiers (uses vsnprintf).
**  If this generates a compile error on any system, the author should be notified
**  so an alternative vsnprintf can be provided.
*/

void
psf_asciiheader_printf (SF_PRIVATE *psf, const char *format, ...)
{	va_list	argptr ;
	int		maxlen ;
	char	*start ;

	maxlen = strlen ((char*) psf->header) ;
	start	= ((char*) psf->header) + maxlen ;
	maxlen	= sizeof (psf->header) - maxlen ;

	va_start (argptr, format) ;
	LSF_VSNPRINTF (start, maxlen, format, argptr) ;
	va_end (argptr) ;

	/* Make sure the string is properly terminated. */
	start [maxlen - 1] = 0 ;

	psf->headindex = strlen ((char*) psf->header) ;

	return ;
} /* psf_asciiheader_printf */

/*-----------------------------------------------------------------------------------------------
**  Binary header writing functions. Returns number of bytes written.
**
**  Format specifiers for psf_binheader_writef are as follows
**		m	- marker - four bytes - no endian manipulation
**
**		e   - all following numerical values will be little endian
**		E   - all following numerical values will be big endian
**
**		t   - all following O types will be truncated to 4 bytes
**		T   - switch off truncation of all following O types
**
**		1	- single byte value
**		2	- two byte value
**		3	- three byte value
**		4	- four byte value
**		8	- eight byte value (sometimes written as 4 bytes)
**
**		s   - string preceded by a four byte length
**		S   - string including null terminator
**		f	- floating point data
**		d	- double precision floating point data
**		h	- 16 binary bytes value
**
**		b	- binary data (see below)
**		z   - zero bytes (ses below)
**		j	- jump forwards or backwards
**
**	To write a word followed by an int (both little endian) use:
**		psf_binheader_writef ("e24", wordval, longval) ;
**
**	To write binary data use:
**		psf_binheader_writef ("b", &bindata, sizeof (bindata)) ;
**
**	To write N zero bytes use:
**			NOTE: due to platform issues (ie x86-64) you should cast the
**			argument to size_t or ensure the variable type is size_t.
**		psf_binheader_writef ("z", N) ;
*/

/* These macros may seem a bit messy but do prevent problems with processors which
** seg. fault when asked to write an int or short to a non-int/short aligned address.
*/

#define	PUT_BYTE(psf,x)		if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 1)	\
							{ (psf)->header [(psf)->headindex++] = (x) ; }

#if (CPU_IS_BIG_ENDIAN == 1)
#define	PUT_MARKER(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 4)	\
							{	(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ;	}

#elif (CPU_IS_LITTLE_ENDIAN == 1)
#define	PUT_MARKER(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 4)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;	}

#else
#	error "Cannot determine endian-ness of processor."
#endif


#define	PUT_BE_SHORT(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 2)	\
							{	(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ; 		}

#define	PUT_LE_SHORT(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 2)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;	}

#define	PUT_BE_3BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 3)	\
							{	(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ;		}

#define	PUT_LE_3BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 3)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;	}

#define	PUT_BE_INT(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 4)	\
							{	(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ;		}

#define	PUT_LE_INT(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 4)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;	}

#if (SIZEOF_SF_COUNT_T == 4)
#define	PUT_BE_8BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 8)	\
							{	(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ;		}

#define	PUT_LE_8BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 8)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;				\
								(psf)->header [(psf)->headindex++] = 0 ;	}

#elif (SIZEOF_SF_COUNT_T == 8)
#define	PUT_BE_8BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 8)	\
							{	(psf)->header [(psf)->headindex++] = ((x) >> 56) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 48) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 40) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 32) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = (x) ;		}

#define	PUT_LE_8BYTE(psf,x)	if ((psf)->headindex < SIGNED_SIZEOF ((psf)->header) - 8)	\
							{	(psf)->header [(psf)->headindex++] = (x) ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 32) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 40) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 48) ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 56) ;	}
#else
#error "SIZEOF_SF_COUNT_T is not defined."
#endif

int
psf_binheader_writef (SF_PRIVATE *psf, const char *format, ...)
{	va_list	argptr ;
	sf_count_t 		countdata ;
	unsigned long 	longdata ;
	unsigned int 	data ;
	float			floatdata ;
	double			doubledata ;
	void			*bindata ;
	size_t			size ;
	char			c, *strptr ;
	int				count = 0, trunc_8to4 ;

	trunc_8to4 = SF_FALSE ;

	va_start (argptr, format) ;

	while ((c = *format++))
	{	switch (c)
		{	case 'e' : /* All conversions are now from LE to host. */
					psf->rwf_endian = SF_ENDIAN_LITTLE ;
					break ;

			case 'E' : /* All conversions are now from BE to host. */
					psf->rwf_endian = SF_ENDIAN_BIG ;
					break ;

			case 't' : /* All 8 byte values now get written as 4 bytes. */
					trunc_8to4 = SF_TRUE ;
					break ;

			case 'T' : /* All 8 byte values now get written as 8 bytes. */
					trunc_8to4 = SF_FALSE ;
					break ;

			case 'm' :
					data = va_arg (argptr, unsigned int) ;
					PUT_MARKER (psf, data) ;
					count += 4 ;
					break ;

			case '1' :
					data = va_arg (argptr, unsigned int) ;
					PUT_BYTE (psf, data) ;
					count += 1 ;
					break ;

			case '2' :
					data = va_arg (argptr, unsigned int) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
					{	PUT_BE_SHORT (psf, data) ;
						}
					else
					{	PUT_LE_SHORT (psf, data) ;
						} ;
					count += 2 ;
					break ;

			case '3' : /* tribyte */
					data = va_arg (argptr, unsigned int) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
					{	PUT_BE_3BYTE (psf, data) ;
						}
					else
					{	PUT_LE_3BYTE (psf, data) ;
						} ;
					count += 3 ;
					break ;

			case '4' :
					data = va_arg (argptr, unsigned int) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
					{	PUT_BE_INT (psf, data) ;
						}
					else
					{	PUT_LE_INT (psf, data) ;
						} ;
					count += 4 ;
					break ;

			case '8' :
					countdata = va_arg (argptr, sf_count_t) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG && trunc_8to4 == SF_FALSE)
					{	PUT_BE_8BYTE (psf, countdata) ;
						count += 8 ;
						}
					else if (psf->rwf_endian == SF_ENDIAN_LITTLE && trunc_8to4 == SF_FALSE)
					{	PUT_LE_8BYTE (psf, countdata) ;
						count += 8 ;
						}
					else if (psf->rwf_endian == SF_ENDIAN_BIG && trunc_8to4 == SF_TRUE)
					{	longdata = countdata & 0xFFFFFFFF ;
						PUT_BE_INT (psf, longdata) ;
						count += 4 ;
						}
					else if (psf->rwf_endian == SF_ENDIAN_LITTLE && trunc_8to4 == SF_TRUE)
					{	longdata = countdata & 0xFFFFFFFF ;
						PUT_LE_INT (psf, longdata) ;
						count += 4 ;
						}
					break ;

			case 'f' :
					/* Floats are passed as doubles. Is this always true? */
					floatdata = (float) va_arg (argptr, double) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						float32_be_write (floatdata, psf->header + psf->headindex) ;
					else
						float32_le_write (floatdata, psf->header + psf->headindex) ;
					psf->headindex += 4 ;
					count += 4 ;
					break ;

			case 'd' :
					doubledata = va_arg (argptr, double) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						double64_be_write (doubledata, psf->header + psf->headindex) ;
					else
						double64_le_write (doubledata, psf->header + psf->headindex) ;
					psf->headindex += 8 ;
					count += 8 ;
					break ;

			case 's' :
					strptr = va_arg (argptr, char *) ;
					size = strlen (strptr) + 1 ;
					size += (size & 1) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
					{	PUT_BE_INT (psf, size) ;
						}
					else
					{	PUT_LE_INT (psf, size) ;
						} ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					psf->header [psf->headindex - 1] = 0 ;
					count += 4 + size ;
					break ;

			case 'S' :
					strptr	= va_arg (argptr, char *) ;
					size	= strlen (strptr) + 1 ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					count += size ;
					break ;

			case 'b' :
					bindata	= va_arg (argptr, void *) ;
					size	= va_arg (argptr, size_t) ;
					memcpy (&(psf->header [psf->headindex]), bindata, size) ;
					psf->headindex += size ;
					count += size ;
					break ;

			case 'z' :
					size = va_arg (argptr, size_t) ;
					count += size ;
					while (size)
					{	psf->header [psf->headindex] = 0 ;
						psf->headindex ++ ;
						size -- ;
						} ;
					break ;

			case 'h' :
					bindata = va_arg (argptr, void *) ;
					memcpy (&(psf->header [psf->headindex]), bindata, 16) ;
					psf->headindex += 16 ;
					count += 16 ;
					break ;

			case 'j' :
					size = va_arg (argptr, int) ;
					psf->headindex += size ;
					count = size ;
					break ;

			default :
				psf_log_printf (psf, "*** Invalid format specifier `%c'\n", c) ;
				psf->error = SFE_INTERNAL ;
				break ;
			} ;
		} ;

	va_end (argptr) ;
	return count ;
} /* psf_binheader_writef */

/*-----------------------------------------------------------------------------------------------
**  Binary header reading functions. Returns number of bytes read.
**
**	Format specifiers are the same as for header write function above with the following
**	additions:
**
**		p   - jump a given number of position from start of file.
**
**	If format is NULL, psf_binheader_readf returns the current offset.
*/

#if (CPU_IS_BIG_ENDIAN == 1)
#define	GET_MARKER(ptr)	(	((ptr) [0] << 24)	| ((ptr) [1] << 16) |	\
							((ptr) [2] << 8)	| ((ptr) [3]) )

#elif (CPU_IS_LITTLE_ENDIAN == 1)
#define	GET_MARKER(ptr)	(	((ptr) [0])			| ((ptr) [1] << 8) |	\
							((ptr) [2] << 16)	| ((ptr) [3] << 24) )

#else
#	error "Cannot determine endian-ness of processor."
#endif

#define	GET_LE_SHORT(ptr)	( ((ptr) [1] << 8) | ((ptr) [0]) )
#define	GET_BE_SHORT(ptr)	( ((ptr) [0] << 8) | ((ptr) [1]) )

#define	GET_LE_3BYTE(ptr)	( 	((ptr) [2] << 16) | ((ptr) [1] << 8) | ((ptr) [0]) )
#define	GET_BE_3BYTE(ptr)	( 	((ptr) [0] << 16) | ((ptr) [1] << 8) | ((ptr) [2]) )

#define	GET_LE_INT(ptr)		( 	((ptr) [3] << 24)	| ((ptr) [2] << 16) |	\
								((ptr) [1] << 8)	| ((ptr) [0]) )

#define	GET_BE_INT(ptr)		( 	((ptr) [0] << 24)	| ((ptr) [1] << 16) |	\
							 	((ptr) [2] << 8)	| ((ptr) [3]) )

#if (SIZEOF_LONG == 4)
#define	GET_LE_8BYTE(ptr)	( 	((ptr) [3] << 24)	| ((ptr) [2] << 16) |	\
							 	((ptr) [1] << 8)	| ((ptr) [0]) )

#define	GET_BE_8BYTE(ptr)	( 	((ptr) [4] << 24)	| ((ptr) [5] << 16) |	\
								((ptr) [6] << 8)	| ((ptr) [7]) )
#else
#define	GET_LE_8BYTE(ptr)	( 	(((ptr) [7] * 1L) << 56) | (((ptr) [6] * 1L) << 48) |	\
							 	(((ptr) [5] * 1L) << 40) | (((ptr) [4] * 1L) << 32) |	\
							 	(((ptr) [3] * 1L) << 24) | (((ptr) [2] * 1L) << 16) |	\
							 	(((ptr) [1] * 1L) << 8 ) | ((ptr) [0]))

#define	GET_BE_8BYTE(ptr)	( 	(((ptr) [0] * 1L) << 56) | (((ptr) [1] * 1L) << 48) |	\
							 	(((ptr) [2] * 1L) << 40) | (((ptr) [3] * 1L) << 32) |	\
							 	(((ptr) [4] * 1L) << 24) | (((ptr) [5] * 1L) << 16) |	\
							 	(((ptr) [6] * 1L) << 8 ) | ((ptr) [7]))

#endif

static int
header_read (SF_PRIVATE *psf, void *ptr, int bytes)
{	int count = 0 ;

	if (psf->headindex + bytes > SIGNED_SIZEOF (psf->header))
	{	if (psf->headend < SIGNED_SIZEOF (psf->header))
			psf_log_printf (psf, "Warning : Further header read would overflow buffer.\n") ;
		psf->headend = SIGNED_SIZEOF (psf->header) ;

		/* This is the best that we can do. */
		return psf_fread (ptr, 1, bytes, psf) ;
		} ;

	if (psf->headindex + bytes > psf->headend)
	{	count = psf_fread (psf->header + psf->headend, 1, bytes - (psf->headend - psf->headindex), psf) ;
		if (count != bytes - (int) (psf->headend - psf->headindex))
		{	psf_log_printf (psf, "Error : psf_fread returned short count.\n") ;
			return 0 ;
			} ;
		psf->headend += count ;
		} ;

	memcpy (ptr, psf->header + psf->headindex, bytes) ;
	psf->headindex += bytes ;

	return bytes ;
} /* header_read */

static void
header_seek (SF_PRIVATE *psf, sf_count_t position, int whence)
{

	switch (whence)
	{	case SEEK_SET :
			if (position > SIGNED_SIZEOF (psf->header))
			{	/* Too much header to cache so just seek instead. */
				psf_fseek (psf, position, whence) ;
				return ;
				} ;
			if (position > psf->headend)
				psf->headend += psf_fread (psf->header + psf->headend, 1, position - psf->headend, psf) ;
			psf->headindex = position ;
			break ;

		case SEEK_CUR :
			if (psf->headindex + position < 0)
				break ;

			if (psf->headindex >= SIGNED_SIZEOF (psf->header))
			{	psf_fseek (psf, position, whence) ;
				return ;
				} ;

			if (psf->headindex + position <= psf->headend)
			{	psf->headindex += position ;
				break ;
				} ;

			if (psf->headindex + position > SIGNED_SIZEOF (psf->header))
			{	/* Need to jump this without caching it. */
				psf->headindex = psf->headend ;
				psf_fseek (psf, position, SEEK_CUR) ;
				break ;
				} ;

			psf->headend += psf_fread (psf->header + psf->headend, 1, position - (psf->headend - psf->headindex), psf) ;
			psf->headindex = psf->headend ;
			break ;

		case SEEK_END :
		default :
			psf_log_printf (psf, "Bad whence param in header_seek().\n") ;
			break ;
		} ;

	return ;
} /* header_seek */

static int
header_gets (SF_PRIVATE *psf, char *ptr, int bufsize)
{
	int		k ;

	for (k = 0 ; k < bufsize - 1 ; k++)
	{	if (psf->headindex < psf->headend)
		{	ptr [k] = psf->header [psf->headindex] ;
			psf->headindex ++ ;
			}
		else
		{	psf->headend += psf_fread (psf->header + psf->headend, 1, 1, psf) ;
			ptr [k] = psf->header [psf->headindex] ;
			psf->headindex = psf->headend ;
			} ;

		if (ptr [k] == '\n')
			break ;
		} ;

	ptr [k] = 0 ;

	return k ;
} /* header_gets */

int
psf_binheader_readf (SF_PRIVATE *psf, char const *format, ...)
{	va_list			argptr ;
	sf_count_t		*countptr, countdata ;
	unsigned char	*ucptr, sixteen_bytes [16] ;
	unsigned int 	*intptr, intdata ;
	unsigned short	*shortptr ;
	char			*charptr ;
	float			*floatptr ;
	double			*doubleptr ;
	char			c ;
	int				byte_count = 0, count ;

	if (! format)
		return psf_ftell (psf) ;

	va_start (argptr, format) ;

	while ((c = *format++))
	{	switch (c)
		{	case 'e' : /* All conversions are now from LE to host. */
					psf->rwf_endian = SF_ENDIAN_LITTLE ;
					break ;

			case 'E' : /* All conversions are now from BE to host. */
					psf->rwf_endian = SF_ENDIAN_BIG ;
					break ;

			case 'm' :
					intptr = va_arg (argptr, unsigned int*) ;
					ucptr = (unsigned char*) intptr ;
					byte_count += header_read (psf, ucptr, sizeof (int)) ;
					*intptr = GET_MARKER (ucptr) ;
					break ;

			case 'h' :
					intptr = va_arg (argptr, unsigned int*) ;
					ucptr = (unsigned char*) intptr ;
					byte_count += header_read (psf, sixteen_bytes, sizeof (sixteen_bytes)) ;
					{	int k ;
						intdata = 0 ;
						for (k = 0 ; k < 16 ; k++)
							intdata ^= sixteen_bytes [k] << k ;
						}
					*intptr = intdata ;
					break ;

			case '1' :
					charptr = va_arg (argptr, char*) ;
					byte_count += header_read (psf, charptr, sizeof (char)) ;
					break ;

			case '2' :
					shortptr = va_arg (argptr, unsigned short*) ;
					ucptr = (unsigned char*) shortptr ;
					byte_count += header_read (psf, ucptr, sizeof (short)) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						*shortptr = GET_BE_SHORT (ucptr) ;
					else
						*shortptr = GET_LE_SHORT (ucptr) ;
					break ;

			case '3' :
					intptr = va_arg (argptr, unsigned int*) ;
					byte_count += header_read (psf, sixteen_bytes, 3) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						*intptr = GET_BE_3BYTE (sixteen_bytes) ;
					else
						*intptr = GET_LE_3BYTE (sixteen_bytes) ;
					break ;

			case '4' :
					intptr = va_arg (argptr, unsigned int*) ;
					ucptr = (unsigned char*) intptr ;
					byte_count += header_read (psf, ucptr, sizeof (int)) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						*intptr = GET_BE_INT (ucptr) ;
					else
						*intptr = GET_LE_INT (ucptr) ;
					break ;

			case '8' :
					countptr = va_arg (argptr, sf_count_t*) ;
					byte_count += header_read (psf, sixteen_bytes, 8) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						countdata = GET_BE_8BYTE (sixteen_bytes) ;
					else
						countdata = GET_LE_8BYTE (sixteen_bytes) ;
					*countptr = countdata ;
					break ;

			case 'f' : /* Float conversion */
					floatptr = va_arg (argptr, float *) ;
					*floatptr = 0.0 ;
					byte_count += header_read (psf, floatptr, sizeof (float)) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						*floatptr = float32_be_read ((unsigned char*) floatptr) ;
					else
						*floatptr = float32_le_read ((unsigned char*) floatptr) ;
					break ;

			case 'd' : /* double conversion */
					doubleptr = va_arg (argptr, double *) ;
					*doubleptr = 0.0 ;
					byte_count += header_read (psf, doubleptr, sizeof (double)) ;
					if (psf->rwf_endian == SF_ENDIAN_BIG)
						*doubleptr = double64_be_read ((unsigned char*) doubleptr) ;
					else
						*doubleptr = double64_le_read ((unsigned char*) doubleptr) ;
					break ;

			case 's' :
					psf_log_printf (psf, "Format conversion 's' not implemented yet.\n") ;
					/*
					strptr = va_arg (argptr, char *) ;
					size   = strlen (strptr) + 1 ;
					size  += (size & 1) ;
					longdata = H2LE_INT (size) ;
					get_int (psf, longdata) ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					*/
					break ;

			case 'b' :
					charptr = va_arg (argptr, char*) ;
					count = va_arg (argptr, int) ;
					if (count > 0)
						byte_count += header_read (psf, charptr, count) ;
					break ;

			case 'G' :
					charptr = va_arg (argptr, char*) ;
					count = va_arg (argptr, int) ;
					if (count > 0)
						byte_count += header_gets (psf, charptr, count) ;
					break ;

			case 'z' :
					psf_log_printf (psf, "Format conversion 'z' not implemented yet.\n") ;
					/*
					size    = va_arg (argptr, size_t) ;
					while (size)
					{	psf->header [psf->headindex] = 0 ;
						psf->headindex ++ ;
						size -- ;
						} ;
					*/
					break ;

			case 'p' :
					/* Get the seek position first. */
					count = va_arg (argptr, int) ;
					header_seek (psf, count, SEEK_SET) ;
					byte_count = count ;
					break ;

			case 'j' :
					/* Get the seek position first. */
					count = va_arg (argptr, int) ;
					header_seek (psf, count, SEEK_CUR) ;
					byte_count += count ;
					break ;

			default :
				psf_log_printf (psf, "*** Invalid format specifier `%c'\n", c) ;
				psf->error = SFE_INTERNAL ;
				break ;
			} ;
		} ;

	va_end (argptr) ;

	return byte_count ;
} /* psf_binheader_readf */

/*-----------------------------------------------------------------------------------------------
*/

sf_count_t
psf_default_seek (SF_PRIVATE *psf, int mode, sf_count_t samples_from_start)
{	sf_count_t position, retval ;

	if (! (psf->blockwidth && psf->dataoffset >= 0))
	{	psf->error = SFE_BAD_SEEK ;
		return	((sf_count_t) -1) ;
		} ;

	if (! psf->sf.seekable)
	{	psf->error = SFE_NOT_SEEKABLE ;
		return	((sf_count_t) -1) ;
		} ;

	position = psf->dataoffset + psf->blockwidth * samples_from_start ;

	if ((retval = psf_fseek (psf, position, SEEK_SET)) != position)
	{	psf->error = SFE_SEEK_FAILED ;
		return ((sf_count_t) -1) ;
		} ;

	mode = mode ;

	return samples_from_start ;
} /* psf_default_seek */

/*-----------------------------------------------------------------------------------------------
*/

void
psf_hexdump (void *ptr, int len)
{	char	ascii [17], *data ;
	int		k, m ;

	if ((data = ptr) == NULL)
		return ;
	if (len <= 0)
		return ;

	puts ("") ;
	for (k = 0 ; k < len ; k += 16)
	{	memset (ascii, ' ', sizeof (ascii)) ;

		printf ("%08X: ", k) ;
		for (m = 0 ; m < 16 && k + m < len ; m++)
		{	printf (m == 8 ? " %02X " : "%02X ", data [k + m] & 0xFF) ;
			ascii [m] = isprint (data [k + m]) ? data [k + m] : '.' ;
			} ;

		if (m <= 8) printf (" ") ;
		for ( ; m < 16 ; m++) printf ("   ") ;

		ascii [16] = 0 ;
		printf (" %s\n", ascii) ;
		} ;

	puts ("") ;
} /* psf_hexdump */

void
psf_log_SF_INFO (SF_PRIVATE *psf)
{	psf_log_printf (psf, "---------------------------------\n") ;

	psf_log_printf (psf, " Sample rate :   %d\n", psf->sf.samplerate) ;
	psf_log_printf (psf, " Frames      :   %C\n", psf->sf.frames) ;
	psf_log_printf (psf, " Channels    :   %d\n", psf->sf.channels) ;

	psf_log_printf (psf, " Format      :   0x%X\n", psf->sf.format) ;
	psf_log_printf (psf, " Sections    :   %d\n", psf->sf.sections) ;
	psf_log_printf (psf, " Seekable    :   %s\n", psf->sf.seekable ? "TRUE" : "FALSE") ;

	psf_log_printf (psf, "---------------------------------\n") ;
} /* psf_dump_SFINFO */

/*========================================================================================
*/

void*
psf_memset (void *s, int c, sf_count_t len)
{	char	*ptr ;
	int 	setcount ;

	ptr = (char *) s ;

	while (len > 0)
	{	setcount = (len > 0x10000000) ? 0x10000000 : (int) len ;

		memset (ptr, c, setcount) ;

		ptr += setcount ;
		len -= setcount ;
		} ;

	return s ;
} /* psf_memset */

void psf_get_date_str (char *str, int maxlen)
{	time_t		current ;
	struct tm	timedata, *tmptr ;

	time (&current) ;

#if defined (HAVE_GMTIME_R)
	/* If the re-entrant version is available, use it. */
	tmptr = gmtime_r (&current, &timedata) ;
#elif defined (HAVE_GMTIME)
	/* Otherwise use the standard one and copy the data to local storage. */
	tmptr = gmtime (&current) ;
	memcpy (&timedata, tmptr, sizeof (timedata)) ;
#else
	tmptr = NULL ;
#endif

	if (tmptr)
		LSF_SNPRINTF (str, maxlen, "%4d-%02d-%02d %02d:%02d:%02d UTC",
			1900 + timedata.tm_year, timedata.tm_mon, timedata.tm_mday,
			timedata.tm_hour, timedata.tm_min, timedata.tm_sec) ;
	else
		LSF_SNPRINTF (str, maxlen, "Unknown date") ;

	return ;
} /* psf_get_date_str */

int
subformat_to_bytewidth (int format)
{
	switch (format)
	{	case SF_FORMAT_PCM_U8 :
		case SF_FORMAT_PCM_S8 :
				return 1 ;
		case SF_FORMAT_PCM_16 :
				return 2 ;
		case SF_FORMAT_PCM_24 :
				return 3 ;
		case SF_FORMAT_PCM_32 :
		case SF_FORMAT_FLOAT :
				return 4 ;
		case SF_FORMAT_DOUBLE :
				return 8 ;
		} ;

	return 0 ;
} /* subformat_to_bytewidth */

int
s_bitwidth_to_subformat (int bits)
{	static int array [] =
	{	SF_FORMAT_PCM_S8, SF_FORMAT_PCM_16, SF_FORMAT_PCM_24, SF_FORMAT_PCM_32
		} ;

	if (bits < 8 || bits > 32)
		return 0 ;

	return array [((bits + 7) / 8) - 1] ;
} /* bitwidth_to_subformat */

int
u_bitwidth_to_subformat (int bits)
{	static int array [] =
	{	SF_FORMAT_PCM_U8, SF_FORMAT_PCM_16, SF_FORMAT_PCM_24, SF_FORMAT_PCM_32
		} ;

	if (bits < 8 || bits > 32)
		return 0 ;

	return array [((bits + 7) / 8) - 1] ;
} /* bitwidth_to_subformat */

#endif /* PSF_LOG_PRINTF_ONLY */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 33e9795e-f717-461a-9feb-65d083a56395
*/
