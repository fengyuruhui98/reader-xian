/* sha1.h
*
*/
#ifndef NETTLE_SHA1_H_INCLUDED
#define NETTLE_SHA1_H_INCLUDED


typedef unsigned char UINT8;
typedef unsigned long UINT32;

#define SHA1_DIGEST_SIZE 20
#define SHA1_DATA_SIZE 64

/* Digest is kept internally as 4 32-bit words. */
#define _SHA1_DIGEST_LENGTH 5

struct sha1_ctx
{
	UINT32 digest[_SHA1_DIGEST_LENGTH]; 	/* Message digest */
	UINT32 count_low, count_high; 			/* 64-bit block count */
	UINT8 block[SHA1_DATA_SIZE]; 			/* SHA1 data buffer */
	unsigned int index;				 		/* index into buffer */
};


void sha1_init(struct sha1_ctx *ctx);
void sha1_update(struct sha1_ctx *ctx, unsigned length, const UINT8 *data);
void sha1_final(struct sha1_ctx *ctx);
void sha1_digest(const struct sha1_ctx *ctx, unsigned length, UINT8 *digest);

#endif /* NETTLE_SHA1_H_INCLUDED */