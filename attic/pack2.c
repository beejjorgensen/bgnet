#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

// macros for packing floats and doubles:
#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/ 
long long pack754(long double f, unsigned bits, unsigned expbits)
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
long double unpack754(long long i, unsigned bits, unsigned expbits)
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
void packi32(unsigned char *buf, unsigned long i)
{
	*buf++ = i>>24; *buf++ = i>>16;
	*buf++ = i>>8;  *buf++ = i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/ 
unsigned int unpacki16(unsigned char *buf)
{
	return (buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
unsigned long unpacki32(unsigned char *buf)
{
	return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**  h - 16-bit              l - 32-bit
**  c - 8-bit char          f - float, 32-bit
**  s - string (16-bit length is automatically prepended)
*/ 
size_t pack(unsigned char *buf, char *format, ...)
{
	va_list ap;
	int h;
	int l;
	char c;
	float f;
	char *s;
	size_t size = 0, len;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'h': // 16-bit
			size += 2;
			h = va_arg(ap, int); // promoted
			packi16(buf, h);
			buf += 2;
			break;

		case 'l': // 32-bit
			size += 4;
			l = va_arg(ap, int);
			packi32(buf, l);
			buf += 4;
			break;

		case 'c': // 8-bit
			size += 1;
			c = va_arg(ap, int); // promoted
			*buf++ = (c>>0)&0xff;
			break;

		case 'f': // float
			size += 4;
			f = va_arg(ap, double); // promoted
			l = pack754_32(f); // convert to IEEE 754
			packi32(buf, l);
			buf += 4;
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
*/
void unpack(unsigned char *buf, char *format, ...)
{
	va_list ap;
	short *h;
	int *l;
	int pf;
	char *c;
	float *f;
	char *s;
	size_t len, count, maxstrlen=0;

	va_start(ap, format);

	for(; *format != '\0'; format++) {
		switch(*format) {
		case 'h': // 16-bit
			h = va_arg(ap, short*);
			*h = unpacki16(buf);
			buf += 2;
			break;

		case 'l': // 32-bit
			l = va_arg(ap, int*);
			*l = unpacki32(buf);
			buf += 4;
			break;

		case 'c': // 8-bit
			c = va_arg(ap, char*);
			*c = *buf++;
			break;

		case 'f': // float
			f = va_arg(ap, float*);
			pf = unpacki32(buf);
			buf += 4;
			*f = unpack754_32(pf);
			break;

		case 's': // string
			s = va_arg(ap, char*);
			len = unpacki16(buf);
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

int main(void)
{
	unsigned char buf[1024];
	char magic;
	short monkeycount;
	long altitude;
	float absurdityfactor;
	char *s = "Great unmitigated Zot!  You've found the Runestaff!";
	char s2[96];
	size_t packetsize, ps2;

	packetsize = pack(buf, "chhlsf", 'B', 0, 37, -5, s, -3490.6677);
	packi16(buf+1, packetsize); // store packet size in packet for kicks

	printf("packet is %d bytes\n", packetsize);

	unpack(buf, "chhl96sf", &magic, &ps2, &monkeycount, &altitude, s2,
		&absurdityfactor);

	printf("'%c' %d %d %ld \"%s\" %f\n", magic, ps2, monkeycount, altitude,
		s2, absurdityfactor);

	return 0;
}

