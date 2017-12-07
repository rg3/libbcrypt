/*
 * Simple base64 encoding implementation, with explicit index calculation.
 */
#include <assert.h>
#include <string.h>
#include "base64.h"

static char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static void base64_block(const unsigned char *in, size_t len, char *out);

static void base64_block(const unsigned char *in, size_t len, char out[4])
{
	unsigned char padded_in[3] = { '\0', '\0', '\0' };

	assert(len >= 1 && len <= 3);
	memmove(padded_in, in, len);

	/* 3 bytes are grouped as 4 6-bit blocks and interpreted as indexes. */
	unsigned int index[4];
	index[0] = ((padded_in[0] >> 2) & 0x3f);
	index[1] = (((padded_in[0] & 0x3) << 4) | ((padded_in[1] >> 4) & 0xf));
	index[2] = (((padded_in[1] & 0xf) << 2) | ((padded_in[2] >> 6) & 0x3));
	index[3] = (padded_in[2] & 0x3f);

	out[0] = base64_table[index[0]];
	out[1] = base64_table[index[1]];
	/* Equal signs are used to indicate padding was applied. */
	out[2] = ((len == 1)?'=':base64_table[index[2]]);
	out[3] = ((len <= 2)?'=':base64_table[index[3]]);
}

void bcrypt_base64_calc(const unsigned char *in, size_t len, char *out)
{
	size_t i, used, output_used, remaining;

	/* The code could have an integer overflow bug if len was very big, when
	 * calculating used and output_used. In our code, len should always be 64 (a
	 * 512-bit message digest). */
	for (i = 0; ; ++i)
	{
		used = i*3;
		output_used = i*4;
		if (used >= len)
		{
			out[output_used] = '\0';
			break;
		}
		remaining = len - used;
		base64_block(in + used, ((remaining < 3)?remaining:3), out + output_used);
	}
}
