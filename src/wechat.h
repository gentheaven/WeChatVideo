#ifndef _WECHAT_H_
#define _WECHAT_H_

#include "rand-isaac.h"
#include "mitm.h"

//video decriptio
//128KB of video HEAD is cryptographic with ISACC
#define WXISAAC_LEN 131072

enum video_state {
	VIDEO_URL_READY = 0, //got url, not download
	VIDEO_DOWNLOADING, //downloading
	VIDEO_DOWNLAODED, //downloaded
};

enum app_state {
	APP_STATE_UNINIT = 0,
	APP_STATE_INITED,
	APP_STATE_RUN,
	APP_STATE_DOWNLOADING,
};

struct video_info {
	char url[4096];
	char title[1024]; //GB2312
	int duration; //seconds
	size_t size; //bytes
	uint64_t seed;

	enum video_state state;
};

#define MAX_SHOW_TITLE_LEN 64
#define MAX_VIDEO_NUM 32
extern int current_video_index; //read thread for mitmdump, update this index
extern struct video_info video_info[MAX_VIDEO_NUM];

/*
* ISAAC64
* product 131072 random number by seed
*
* in: seed
* out: keys[131072]
*/
extern void get_decryptor_array(uint64_t seed, uint8_t* keys);

/*
* in: keys[131072]
* in: video with ISAAC encryption
* out: video with decryption
*/
extern int decode_video(uint8_t* keys, char* video_in, char* video_out);

/*
* in: seed
* in: video with ISAAC encryption
* out: video with decryption
*/
extern int decode_video_with_seed(uint64_t seed, char* video_in, char* video_out);

extern void handle_json_data(char* json, size_t json_len, struct video_info* video);


#endif
