#include <string.h>
#include <stdio.h>

#include "wechat.h"

/*
* ISAAC64
* product 131072 random number by seed
*
* in: seed
* out: keys[131072]
*/
void get_decryptor_array(uint64_t seed, uint8_t* keys)
{
	int encrypted_len = WXISAAC_LEN;
	struct isaac_state state;
	memset((void*)state.m, 0, ISAAC_WORDS * sizeof(isaac_word));
	state.m[0] = seed;
	isaac_seed(&state);

	int i, mod;
	int index;
	isaac_word result[ISAAC_WORDS];
	memset((void*)result, 0, ISAAC_WORDS * sizeof(isaac_word));

	for (i = 0; i < encrypted_len; i++) {
		mod = i % (ISAAC_WORDS * sizeof(isaac_word));
		if (mod == 0) {
			memset((void*)result, 0, ISAAC_WORDS * sizeof(isaac_word));
			isaac_refill(&state, result);
		}
		index = ISAAC_WORDS * sizeof(isaac_word) - 1 - mod;
		keys[i] = *((uint8_t*)result + index);
	}

	/*
	//write to file
	FILE* fp = fopen("key.mp4", "wb");
	fwrite(keys, 1, encrypted_len, fp);
	fclose(fp);
	*/
}

/*
* in: keys[131072]
* in: video with ISAAC encryption
* out: video with decryption
*/
int decode_video(uint8_t* keys, char* video_in, char* video_out)
{
	FILE* fp_in = fopen(video_in, "rb");
	if (!fp_in)
		return -1;

	FILE* fp_out = fopen(video_out, "wb");
	uint8_t buf[WXISAAC_LEN + 1];
	uint8_t out_buf[WXISAAC_LEN + 1];
	fread(buf, 1, WXISAAC_LEN, fp_in);

	//handle 128KB of video head
	int i;
	for (i = 0; i < WXISAAC_LEN; i++) {
		out_buf[i] = buf[i] ^ keys[i];
	}
	fwrite(out_buf, 1, WXISAAC_LEN, fp_out);

	//rest part of video
	size_t read_byes;
	while (1) {
		read_byes = fread(buf, 1, WXISAAC_LEN, fp_in);
		fwrite(buf, 1, read_byes, fp_out);
		if (read_byes != WXISAAC_LEN)
			break;
	}

	printf("Finished to decode %s\n", video_in);
	fclose(fp_in);
	fclose(fp_out);
	return 0;
}

/*
* in: seed
* in: video with ISAAC encryption
* out: video with decryption
*/
int decode_video_with_seed(uint64_t seed, char* video_in, char* video_out)
{
	uint8_t decode_key[WXISAAC_LEN];
	get_decryptor_array(seed, decode_key);
	return decode_video(decode_key, video_in, video_out);
}

