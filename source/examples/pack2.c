#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>

// macros for packing floats and doubles:
#define pack754_16(f) (pack754((f), 16, 5))
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_16(i) (unpack754((i), 16, 5))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/ 
unsigned long long int pack754(long double f, unsigned bits, unsigned expbits)
{
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
long double unpack754(unsigned long long int i, unsigned bits, unsigned expbits)
{
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
void packi16(unsigned char *buf, unsigned int i)
{
	*buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/ 
void packi32(unsigned char *buf, unsigned long int i)
{
	*buf++ = i>>24; *buf++ = i>>16;
	*buf++ = i>>8;  *buf++ = i;
}

/*
** packi64() -- store a 64-bit int into a char buffer (like htonl())
*/ 
void packi64(unsigned char *buf, unsigned long long int i)
{
	*buf++ = i>>56; *buf++ = i>>48;
	*buf++ = i>>40; *buf++ = i>>32;
	*buf++ = i>>24; *buf++ = i>>16;
	*buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/ 
int unpacki16(unsigned char *buf)
{
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
unsigned int unpacku16(unsigned char *buf)
{
	return ((unsigned int)buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
long int unpacki32(unsigned char *buf)
{
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
unsigned long int unpacku32(unsigned char *buf)
{
	return ((unsigned long int)buf[0]<<24) |
	       ((unsigned long int)buf[1]<<16) |
	       ((unsigned long int)buf[2]<<8)  |
	       buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer (like ntohl())
*/ 
long long int unpacki64(unsigned char *buf)
{
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
unsigned long long int unpacku64(unsigned char *buf)
{
	return ((unsigned long long int)buf[0]<<56) |
	       ((unsigned long long int)buf[1]<<48) |
	       ((unsigned long long int)buf[2]<<40) |
	       ((unsigned long long int)buf[3]<<32) |
	       ((unsigned long long int)buf[4]<<24) |
	       ((unsigned long long int)buf[5]<<16) |
	       ((unsigned long long int)buf[6]<<8)  |
	       buf[7];
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C         
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (16-bit unsigned length is automatically prepended to strings)
*/ 

unsigned int pack(unsigned char *buf, char *format, ...)
{
	va_list ap;

	signed char c;              // 8-bit
	unsigned char C;

	int h;                      // 16-bit
	unsigned int H;

	long int l;                 // 32-bit
	unsigned long int L;

	long long int q;            // 64-bit
	unsigned long long int Q;

	float f;                    // floats
	double d;
	long double g;
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
			h = va_arg(ap, int);
			packi16(buf, h);
			buf += 2;
			break;

		case 'H': // 16-bit unsigned
			size += 2;
			H = va_arg(ap, unsigned int);
			packi16(buf, H);
			buf += 2;
			break;

		case 'l': // 32-bit
			size += 4;
			l = va_arg(ap, long int);
			packi32(buf, l);
			buf += 4;
			break;

		case 'L': // 32-bit unsigned
			size += 4;
			L = va_arg(ap, unsigned long int);
			packi32(buf, L);
			buf += 4;
			break;

		case 'q': // 64-bit
			size += 8;
			q = va_arg(ap, long long int);
			packi64(buf, q);
			buf += 8;
			break;

		case 'Q': // 64-bit unsigned
			size += 8;
			Q = va_arg(ap, unsigned long long int);
			packi64(buf, Q);
			buf += 8;
			break;

		case 'f': // float-16
			size += 2;
			f = (float)va_arg(ap, double); // promoted
			fhold = pack754_16(f); // convert to IEEE 754
			packi16(buf, fhold);
			buf += 2;
			break;

		case 'd': // float-32
			size += 4;
			d = va_arg(ap, double);
			fhold = pack754_32(d); // convert to IEEE 754
			packi32(buf, fhold);
			buf += 4;
			break;

		case 'g': // float-64
			size += 8;
			g = va_arg(ap, long double);
			fhold = pack754_64(g); // convert to IEEE 754
			packi64(buf, fhold);
			buf += 8;
			break;

		case 's': // string
			s = va_arg(ap, char*);
			len = strlen(s);
			size += len + 2;
			packi16(buf, len);
			buf += 2;
			memcpy(buf, s, len);
			buf += len;
			break;
		}
	}

	va_end(ap);

	return size;
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**   bits |signed   unsigned   float   string
**   -----+----------------------------------
**      8 |   c        C         
**     16 |   h        H         f
**     32 |   l        L         d
**     64 |   q        Q         g
**      - |                               s
**
**  (string is extracted based on its stored length, but 's' can be
**  prepended with a max length)
*/
void unpack(unsigned char *buf, char *format, ...)
{
	va_list ap;

	signed char *c;              // 8-bit
	unsigned char *C;

	int *h;                      // 16-bit
	unsigned int *H;

	long int *l;                 // 32-bit
	unsigned long int *L;

	long long int *q;            // 64-bit
	unsigned long long int *Q;

	float *f;                    // floats
	double *d;
	long double *g;
	unsigned long long int fhold;

	char *s;
	unsigned int len, maxstrlen=0, count;

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
			h = va_arg(ap, int*);
			*h = unpacki16(buf);
			buf += 2;
			break;

		case 'H': // 16-bit unsigned
			H = va_arg(ap, unsigned int*);
			*H = unpacku16(buf);
			buf += 2;
			break;

		case 'l': // 32-bit
			l = va_arg(ap, long int*);
			*l = unpacki32(buf);
			buf += 4;
			break;

		case 'L': // 32-bit unsigned
			L = va_arg(ap, unsigned long int*);
			*L = unpacku32(buf);
			buf += 4;
			break;

		case 'q': // 64-bit
			q = va_arg(ap, long long int*);
			*q = unpacki64(buf);
			buf += 8;
			break;

		case 'Q': // 64-bit unsigned
			Q = va_arg(ap, unsigned long long int*);
			*Q = unpacku64(buf);
			buf += 8;
			break;

		case 'f': // float
			f = va_arg(ap, float*);
			fhold = unpacku16(buf);
			*f = unpack754_16(fhold);
			buf += 2;
			break;

		case 'd': // float-32
			d = va_arg(ap, double*);
			fhold = unpacku32(buf);
			*d = unpack754_32(fhold);
			buf += 4;
			break;

		case 'g': // float-64
			g = va_arg(ap, long double*);
			fhold = unpacku64(buf);
			*g = unpack754_64(fhold);
			buf += 8;
			break;

		case 's': // string
			s = va_arg(ap, char*);
			len = unpacku16(buf);
			buf += 2;
			if (maxstrlen > 0 && len > maxstrlen) count = maxstrlen - 1;
			else count = len;
			memcpy(s, buf, count);
			s[count] = '\0';
			buf += len;
			break;

		default:
			if (isdigit(*format)) { // track max str len
				maxstrlen = maxstrlen * 10 + (*format-'0');
			}
		}

		if (!isdigit(*format)) maxstrlen = 0;
	}

	va_end(ap);
}

//#define DEBUG
#ifdef DEBUG
#include <limits.h>
#include <float.h>
#include <assert.h>
#endif

int main(void)
{
#ifndef DEBUG
	unsigned char buf[1024];
	unsigned char magic;
	int monkeycount;
	long altitude;
	double absurdityfactor;
	char *s = "Great unmitigated Zot!  You've found the Runestaff!";
	char s2[96];
	unsigned int packetsize, ps2;

	packetsize = pack(buf, "CHhlsd", 'B', 0, 37, -5, s, -3490.5);
	packi16(buf+1, packetsize); // store packet size in packet for kicks

	printf("packet is %u bytes\n", packetsize);

	unpack(buf, "CHhl96sd", &magic, &ps2, &monkeycount, &altitude, s2,
		&absurdityfactor);

	printf("'%c' %hhu %u %ld \"%s\" %f\n", magic, ps2, monkeycount,
			altitude, s2, absurdityfactor);

#else
	unsigned char buf[1024];

	int x;

	long long k, k2;
	long long test64[14] = { 0, -0, 1, 2, -1, -2, 0x7fffffffffffffffll>>1, 0x7ffffffffffffffell, 0x7fffffffffffffffll, -0x7fffffffffffffffll, -0x8000000000000000ll, 9007199254740991ll, 9007199254740992ll, 9007199254740993ll };

	unsigned long long K, K2;
	unsigned long long testu64[14] = { 0, 0, 1, 2, 0, 0, 0xffffffffffffffffll>>1, 0xfffffffffffffffell, 0xffffffffffffffffll, 0, 0, 9007199254740991ll, 9007199254740992ll, 9007199254740993ll };

	long i, i2;
	long test32[14] = { 0, -0, 1, 2, -1, -2, 0x7fffffffl>>1, 0x7ffffffel, 0x7fffffffl, -0x7fffffffl, -0x80000000l, 0, 0, 0 };

	unsigned long I, I2;
	unsigned long testu32[14] = { 0, 0, 1, 2, 0, 0, 0xffffffffl>>1, 0xfffffffel, 0xffffffffl, 0, 0, 0, 0, 0 };

	int j, j2;
	int test16[14] = { 0, -0, 1, 2, -1, -2, 0x7fff>>1, 0x7ffe, 0x7fff, -0x7fff, -0x8000, 0, 0, 0 };

	printf("char bytes: %zu\n", sizeof(char));
	printf("int bytes: %zu\n", sizeof(int));
	printf("long bytes: %zu\n", sizeof(long));
	printf("long long bytes: %zu\n", sizeof(long long));
	printf("float bytes: %zu\n", sizeof(float));
	printf("double bytes: %zu\n", sizeof(double));
	printf("long double bytes: %zu\n", sizeof(long double));

	for(x = 0; x < 14; x++) {
		k = test64[x];
		pack(buf, "q", k);
		unpack(buf, "q", &k2);

		if (k2 != k) {
			printf("64: %lld != %lld\n", k, k2);
			printf("  before: %016llx\n", k);
			printf("  after:  %016llx\n", k2);
			printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
				" %02hhx %02hhx %02hhx %02hhx\n", 
				buf[0], buf[1], buf[2], buf[3],
				buf[4], buf[5], buf[6], buf[7]);
		} else {
			//printf("64: OK: %lld == %lld\n", k, k2);
		}

		K = testu64[x];
		pack(buf, "Q", K);
		unpack(buf, "Q", &K2);

		if (K2 != K) {
			printf("64: %llu != %llu\n", K, K2);
		} else {
			//printf("64: OK: %llu == %llu\n", K, K2);
		}

		i = test32[x];
		pack(buf, "l", i);
		unpack(buf, "l", &i2);

		if (i2 != i) {
			printf("32(%d): %ld != %ld\n", x,i, i2);
			printf("  before: %08lx\n", i);
			printf("  after:  %08lx\n", i2);
			printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
				" %02hhx %02hhx %02hhx %02hhx\n", 
				buf[0], buf[1], buf[2], buf[3],
				buf[4], buf[5], buf[6], buf[7]);
		} else {
			//printf("32: OK: %ld == %ld\n", i, i2);
		}

		I = testu32[x];
		pack(buf, "L", I);
		unpack(buf, "L", &I2);

		if (I2 != I) {
			printf("32(%d): %lu != %lu\n", x,I, I2);
		} else {
			//printf("32: OK: %lu == %lu\n", I, I2);
		}

		j = test16[x];
		pack(buf, "h", j);
		unpack(buf, "h", &j2);

		if (j2 != j) {
			printf("16: %d != %d\n", j, j2);
		} else {
			//printf("16: OK: %d == %d\n", j, j2);
		}
	}

	if (1) {
		long double testf64[8] = { -3490.6677, 0.0, 1.0, -1.0, DBL_MIN*2, DBL_MAX/2, DBL_MIN, DBL_MAX };
		long double f,f2;

		for (i = 0; i < 8; i++) {
			f = testf64[i];
			pack(buf, "g", f);
			unpack(buf, "g", &f2);

			if (f2 != f) {
				printf("f64: %Lf != %Lf\n", f, f2);
				printf("  before: %016llx\n", *((long long*)&f));
				printf("  after:  %016llx\n", *((long long*)&f2));
				printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
					" %02hhx %02hhx %02hhx %02hhx\n", 
					buf[0], buf[1], buf[2], buf[3],
					buf[4], buf[5], buf[6], buf[7]);
			} else {
				//printf("f64: OK: %f == %f\n", f, f2);
			}
		}
	}
	if (1) {
		double testf32[7] = { 0.0, 1.0, -1.0, 10, -3.6677, 3.1875, -3.1875 };
		double f,f2;

		for (i = 0; i < 7; i++) {
			f = testf32[i];
			pack(buf, "d", f);
			unpack(buf, "d", &f2);

			if (f2 != f) {
				printf("f32: %.10f != %.10f\n", f, f2);
				printf("  before: %016llx\n", *((long long*)&f));
				printf("  after:  %016llx\n", *((long long*)&f2));
				printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
					" %02hhx %02hhx %02hhx %02hhx\n", 
					buf[0], buf[1], buf[2], buf[3],
					buf[4], buf[5], buf[6], buf[7]);
			} else {
				//printf("f32: OK: %f == %f\n", f, f2);
			}
		}
	}
	if (1) {
		float testf16[7] = { 0.0, 1.0, -1.0, 10, -10, 3.1875, -3.1875 };
		float f,f2;

		for (i = 0; i < 7; i++) {
			f = testf16[i];
			pack(buf, "f", f);
			unpack(buf, "f", &f2);

			if (f2 != f) {
				printf("f16: %f != %f\n", f, f2);
				printf("  before: %08x\n", *((int*)&f));
				printf("  after:  %08x\n", *((int*)&f2));
				printf("  buffer: %02hhx %02hhx %02hhx %02hhx "
					" %02hhx %02hhx %02hhx %02hhx\n", 
					buf[0], buf[1], buf[2], buf[3],
					buf[4], buf[5], buf[6], buf[7]);
			} else {
				//printf("f16: OK: %f == %f\n", f, f2);
			}
		}
	}
#endif

	return 0;
}

