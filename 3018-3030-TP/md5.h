#include <string.h>

typedef unsigned long md5_uint32;

struct md5_ctx
{
  md5_uint32 A;
  md5_uint32 B;
  md5_uint32 C;
  md5_uint32 D;

  md5_uint32 total[2];
  md5_uint32 buflen;
  char buffer[128];// __attribute__ ((__aligned__ (__alignof__ (md5_uint32))));
	//char buffer[3072];
};

#undef __P
#if defined (__STDC__) && __STDC__
# define __P(x) x
#else
# define __P(x) ()
#endif

#ifdef WORDS_BIGENDIAN
# define SWAP(n)							\
    (((n) << 24) | (((n) & 0xff00) << 8) | (((n) >> 8) & 0xff00) | ((n) >> 24))
#else
# define SWAP(n) (n)
#endif

/* This array contains the bytes used to pad the buffer to the next
   64-byte boundary.  (RFC 1321, 3.1: Step 1)  */
static const unsigned char fillbuf[64] = { 0x80, 0 /* , 0, 0, ...  */ };

#define FF(b, c, d) (d ^ (b & (c ^ d)))
#define FG(b, c, d) FF (d, b, c)
#define FH(b, c, d) (b ^ c ^ d)
#define FI(b, c, d) (c ^ (b | ~d))

#define ENC_OK		0
#define ENC_PARAM	1
#define ENC_RESULT	2

void md5_init_ctx __P ((struct md5_ctx *ctx));

void md5_process_block __P ((const void *buffer, size_t len,
				    struct md5_ctx *ctx));

void md5_process_bytes __P ((const void *buffer, size_t len,
				    struct md5_ctx *ctx));

void *md5_finish_ctx __P ((struct md5_ctx *ctx, void *resbuf));

void *md5_read_ctx __P ((const struct md5_ctx *ctx, void *resbuf));

//int md5_stream __P ((FILE *stream, void *resblock));

//void *md5_buffer __P ((const char *buffer, size_t len, void *resblock));
unsigned char * md5_str(unsigned char *EncryptingChar, long in_len, unsigned char *EncryptedChar);
long VerifyPIN(unsigned char *UserInputPIN, unsigned char *UserInputEnPIN);
