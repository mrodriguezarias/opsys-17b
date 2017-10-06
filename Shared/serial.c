/*
 * Módulo de serialización extraído de la guía Beej.
 * Ver ejemplos de uso al final de la página.
 * http://beej.us/guide/bgnet/examples/pack2.c
 */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "serial.h"

#define SERIAL_MAX 1024

// macros for packing floats and doubles:
#define pack754_16(f) (pack754((f), 16, 5))
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_16(i) (unpack754((i), 16, 5))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

static unsigned long long int pack754(long double f, unsigned bits, unsigned expbits);
static long double unpack754(unsigned long long int i, unsigned bits, unsigned expbits);
static void packi16(unsigned char *buf, unsigned int i);
static void packi32(unsigned char *buf, unsigned long int i);
static void packi64(unsigned char *buf, unsigned long long int i);
static int unpacki16(unsigned char *buf);
static unsigned int unpacku16(unsigned char *buf);
static long int unpacki32(unsigned char *buf);
static unsigned long int unpacku32(unsigned char *buf);
static long long int unpacki64(unsigned char *buf);
static unsigned long long int unpacku64(unsigned char *buf);

// ========== Funciones públicas ==========

t_serial *serial_create(void *data, size_t size) {
	t_serial *serial = malloc(sizeof(t_serial));
	serial->data = data;
	serial->size = size;
	return serial;
}

void serial_destroy(t_serial *serial) {
	free(serial->data);
	free(serial);
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**  (16-bit unsigned length is automatically prepended to strings)
*/

t_serial *serial_pack(const char *format, ...) {
	char buffer[SERIAL_MAX];
	char *buf = buffer;
	va_list ap;

	signed char c;              // 8-bit
	unsigned char C;

	short h;                      // 16-bit
	unsigned short H;

	int l;                 // 32-bit
	unsigned int L;

	long long int q;            // 64-bit
	unsigned long long int Q;

	float d;                    // floats
	double g;
	unsigned long long int fhold;

	char *s;                    // strings
	unsigned int len;

	unsigned int size = 0;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'c': // 8-bit
			size += 1;
			c = (signed char)va_arg(ap, int); // promoted
			*buf++ = c;
			break;

		case 'C': // 8-bit unsigned
			size += 1;
			C = (unsigned char)va_arg(ap, unsigned int); // promoted
			*buf++ = C;
			break;

		case 'h': // 16-bit
			size += 2;
			h = (short)va_arg(ap, int);
			packi16((unsigned char *)buf, h);
			buf += 2;
			break;

		case 'H': // 16-bit unsigned
			size += 2;
			H = (unsigned short)va_arg(ap, unsigned int);
			packi16((unsigned char *)buf, H);
			buf += 2;
			break;

		case 'i': // 32-bit
			size += 4;
			l = va_arg(ap, int);
			packi32((unsigned char *)buf, l);
			buf += 4;
			break;

		case 'I': // 32-bit unsigned
			size += 4;
			L = va_arg(ap, unsigned int);
			packi32((unsigned char *)buf, L);
			buf += 4;
			break;

		case 'l': // 64-bit
			size += 8;
			q = va_arg(ap, long long int);
			packi64((unsigned char *)buf, q);
			buf += 8;
			break;

		case 'L': // 64-bit unsigned
			size += 8;
			Q = va_arg(ap, unsigned long long int);
			packi64((unsigned char *)buf, Q);
			buf += 8;
			break;

		case 'f': // float-32
			size += 4;
			d = (float)va_arg(ap, double);
			fhold = pack754_32(d); // convert to IEEE 754
			packi32((unsigned char *)buf, fhold);
			buf += 4;
			break;

		case 'd': // float-64
			size += 8;
			g = va_arg(ap, double);
			fhold = pack754_64(g); // convert to IEEE 754
			packi64((unsigned char *)buf, fhold);
			buf += 8;
			break;

		case 's': // string
			s = va_arg(ap, char*);
			len = sprintf(buf, "%s", s) + 1;
			size += len;
			buf += len;
			break;
		}
	}

	va_end(ap);

	char *data = malloc(size);
	memcpy(data, buffer, size);

	return serial_create(data, size);
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
void serial_unpack(t_serial *serial, const char *format, ...) {
	const char *buf = serial->data;
	va_list ap;

	signed char *c;              // 8-bit
	unsigned char *C;

	short *h;                      // 16-bit
	unsigned short *H;

	int *l;                 // 32-bit
	unsigned int *L;

	long long int *q;            // 64-bit
	unsigned long long int *Q;

	float *d;                    // floats
	double *g;
	unsigned long long int fhold;

	char **s;
	unsigned int len;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'c': // 8-bit
			c = va_arg(ap, signed char*);
			if (*buf <= 0x7f) { *c = *buf;} // re-sign
			else { *c = -1 - (unsigned char)(0xffu - *buf); }
			buf++;
			break;

		case 'C': // 8-bit unsigned
			C = va_arg(ap, unsigned char*);
			*C = *buf++;
			break;

		case 'h': // 16-bit
			h = va_arg(ap, short*);
			*h = unpacki16((unsigned char *)buf);
			buf += 2;
			break;

		case 'H': // 16-bit unsigned
			H = va_arg(ap, unsigned short*);
			*H = unpacku16((unsigned char *)buf);
			buf += 2;
			break;

		case 'i': // 32-bit
			l = va_arg(ap, int*);
			*l = unpacki32((unsigned char *)buf);
			buf += 4;
			break;

		case 'I': // 32-bit unsigned
			L = va_arg(ap, unsigned int*);
			*L = unpacku32((unsigned char *)buf);
			buf += 4;
			break;

		case 'l': // 64-bit
			q = va_arg(ap, long long int*);
			*q = unpacki64((unsigned char *)buf);
			buf += 8;
			break;

		case 'L': // 64-bit unsigned
			Q = va_arg(ap, unsigned long long int*);
			*Q = unpacku64((unsigned char *)buf);
			buf += 8;
			break;

		case 'f': // float-32
			d = va_arg(ap, float*);
			fhold = unpacku32((unsigned char *)buf);
			*d = unpack754_32(fhold);
			buf += 4;
			break;

		case 'd': // float-64
			g = va_arg(ap, double*);
			fhold = unpacku64((unsigned char *)buf);
			*g = unpack754_64(fhold);
			buf += 8;
			break;

		case 's': // string
			s = va_arg(ap, char**);
			char *str = strdup(buf);
			len = strlen(str) + 1;
			*s = str;
			buf += len;
			break;

		default:
			break;
		}
	}

	va_end(ap);
}

// ========== Funciones privadas ==========

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/
static unsigned long long int pack754(long double f, unsigned bits, unsigned expbits) {
	long double fnorm;
	int shift;
	long long sign, exp, significand;
	unsigned significandbits = bits - expbits - 1; // -1 for sign bit

	if (f == 0.0) return 0; // get this special case out of the way

	// check sign and begin normalization
	if (f < 0) { sign = 1; fnorm = -f; }
	else { sign = 0; fnorm = f; }

	// get the normalized form of f and track the exponent
	shift = 0;
	while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
	while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
	fnorm = fnorm - 1.0;

	// calculate the binary form (non-float) of the significand data
	significand = fnorm * ((1LL<<significandbits) + 0.5f);

	// get the biased exponent
	exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

	// return the final answer
	return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

/*
** unpack754() -- unpack a floating point number from IEEE-754 format
*/
static long double unpack754(unsigned long long int i, unsigned bits, unsigned expbits) {
	long double result;
	long long shift;
	unsigned bias;
	unsigned significandbits = bits - expbits - 1; // -1 for sign bit

	if (i == 0) return 0.0;

	// pull the significand
	result = (i&((1LL<<significandbits)-1)); // mask
	result /= (1LL<<significandbits); // convert back to float
	result += 1.0f; // add the one back on

	// deal with the exponent
	bias = (1<<(expbits-1)) - 1;
	shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
	while(shift > 0) { result *= 2.0; shift--; }
	while(shift < 0) { result /= 2.0; shift++; }

	// sign it
	result *= (i>>(bits-1))&1? -1.0: 1.0;

	return result;
}

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/
static void packi16(unsigned char *buf, unsigned int i) {
	*buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
static void packi32(unsigned char *buf, unsigned long int i) {
	*buf++ = i>>24; *buf++ = i>>16;
	*buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/
static void packi64(unsigned char *buf, unsigned long long int i) {
	*buf++ = i>>56; *buf++ = i>>48;
	*buf++ = i>>40; *buf++ = i>>32;
	*buf++ = i>>24; *buf++ = i>>16;
	*buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/
static int unpacki16(unsigned char *buf) {
	unsigned int i2 = ((unsigned int)buf[0]<<8) | buf[1];
	int i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffu) { i = i2; }
	else { i = -1 - (unsigned int)(0xffffu - i2); }

	return i;
}

/*
** unpacku16() -- unpack a 16-bit unsigned from a char buffer (like ntohs())
*/
static unsigned int unpacku16(unsigned char *buf) {
	return ((unsigned int)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/
static long int unpacki32(unsigned char *buf) {
	unsigned long int i2 = ((unsigned long int)buf[0]<<24) |
	                       ((unsigned long int)buf[1]<<16) |
	                       ((unsigned long int)buf[2]<<8)  |
	                       buf[3];
	long int i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffffffu) { i = i2; }
	else { i = -1 - (long int)(0xffffffffu - i2); }

	return i;
}

/*
** unpacku32() -- unpack a 32-bit unsigned from a char buffer (like ntohl())
*/
static unsigned long int unpacku32(unsigned char *buf) {
	return ((unsigned long int)buf[0]<<24) |
	       ((unsigned long int)buf[1]<<16) |
	       ((unsigned long int)buf[2]<<8)  |
	       buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/
static long long int unpacki64(unsigned char *buf) {
	unsigned long long int i2 = ((unsigned long long int)buf[0]<<56) |
	                            ((unsigned long long int)buf[1]<<48) |
	                            ((unsigned long long int)buf[2]<<40) |
	                            ((unsigned long long int)buf[3]<<32) |
	                            ((unsigned long long int)buf[4]<<24) |
	                            ((unsigned long long int)buf[5]<<16) |
	                            ((unsigned long long int)buf[6]<<8)  |
	                            buf[7];
	long long int i;

	// change unsigned numbers to signed
	if (i2 <= 0x7fffffffffffffffu) { i = i2; }
	else { i = -1 -(long long int)(0xffffffffffffffffu - i2); }

	return i;
}

/*
** unpacku64() -- unpack a 64-bit unsigned from a char buffer (like ntohl())
*/
static unsigned long long int unpacku64(unsigned char *buf) {
	return ((unsigned long long int)buf[0]<<56) |
	       ((unsigned long long int)buf[1]<<48) |
	       ((unsigned long long int)buf[2]<<40) |
	       ((unsigned long long int)buf[3]<<32) |
	       ((unsigned long long int)buf[4]<<24) |
	       ((unsigned long long int)buf[5]<<16) |
	       ((unsigned long long int)buf[6]<<8)  |
	       buf[7];
}

