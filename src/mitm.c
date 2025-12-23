#include <string.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "wechat.h"
#include "win.h"

int current_video_index = 0;
struct video_info video_info[MAX_VIDEO_NUM];

/*
first run: only do once
1. install certificate
certificate not installed, installed it;

2. delete cache
io.appData("..\Roaming\Tencent\WeChat\radium\web\profiles")
e.g. C:\Users\xxx\AppData\Roaming\Tencent\WeChat\radium\web\profiles
*/

char comma_mark[] = u8"，";
char full_stop[] = u8"。";
char question_stop[] = u8"？";
char exclamation_mark[] = u8"！";

char unknow_title[] = u8"未知标题";

/*
* cut the title
#title: title, ignore the first #
title#: title, stop at #
*/
void handle_title_end(char* buf)
{
	char* tail;
	//multi-line title, only care first line
	tail = strchr(buf, '\n');
	if (tail)
		*tail = 0;
	tail = strchr(buf, '#');
	if (tail)
		*tail = 0;

	tail = strstr(buf, comma_mark);
	if (tail)
		*tail = 0;

	tail = strstr(buf, full_stop);
	if (tail)
		*tail = 0;

	tail = strstr(buf, question_stop);
	if (tail)
		*tail = 0;

	tail = strstr(buf, exclamation_mark);
	if (tail)
		*tail = 0;
}

void handle_json_title(char* buf, char* title)
{
	char* head = buf;
	char* cur;
	int len = 0;

	*title = 0;
	//skip first '#'
	if (*head == '#')
		head++;

	if (*head == 0) {
		cur = u8_to_gb2312(unknow_title, &len);
	} else {
		handle_title_end(head);
		cur = u8_to_gb2312(head, &len);
	}
	if (!cur)
		return;

	char* tail;
	len >>= 1;
	if (len >= MAX_SHOW_TITLE_LEN) {
		//cut the title
		tail = cur + MAX_SHOW_TITLE_LEN;
		*tail = 0;
	}
	strcpy(title, cur);
	free(cur);
}

//according video title, check if video repeat
//return 1 if repeat
int check_repeat_video(struct video_info* videos, struct video_info* cur)
{
	int total = current_video_index;
	int i;
	for (i = 0; i < total; i++) {
		if (!strcmp(cur->title, videos[i].title))
			return 1;
	}
	return 0;
}

/*
"json": {
	"createtime": 1754709642,
	"duration": 15066,
	"fileFormat": [
	  "xWT111",
	  "xWT112",
	  "xWT156",
	  "xWT157",
	  "xWT113",
	  "xWT158"
	],
	"id": "14719571344689859092",
	"key": "944906605",
	"nickname": "xx",
	"nonce_id": "15326867668207159310_0_141_0_0",
	"size": 3058675,
	"title": "xx",
	"url": xxx
  },

  "duration": 15066, 15066ms
  "key": "944906605", seed
  "size": 3058675, Bytes
  "title": UTF8

  Member: url
	String value […]: https://finder.video.qq.com/251/20302/stodownload?encfilekey=Cvvj5I...
*/
void handle_json_data(char *json, size_t json_len, struct video_info* video)
{
	cJSON* root = cJSON_ParseWithLength(json, json_len);
	if (!root)
		return;

	static struct video_info current_video;
	struct video_info* pvideo = &current_video;

	cJSON* duration = cJSON_GetObjectItem(root, "duration");
	cJSON* key = cJSON_GetObjectItem(root, "key");
	cJSON* size = cJSON_GetObjectItem(root, "size");
	cJSON* title = cJSON_GetObjectItem(root, "title");
	cJSON* url = cJSON_GetObjectItem(root, "url");

	if (cJSON_IsNumber(duration)) {
		pvideo->duration = duration->valueint;
		pvideo->duration = pvideo->duration / 1000;
	}

	if (cJSON_IsString(key)) {
		pvideo->seed = atol(key->valuestring);
	}

	if (cJSON_IsNumber(size)) {
		pvideo->size = size->valueint; //bytes
		pvideo->size = pvideo->size >> 20; //MB
	}

	if (cJSON_IsString(title)) {
		handle_json_title(title->valuestring, pvideo->title);
	}

	if (cJSON_IsString(url)) {
		strcpy(pvideo->url, url->valuestring);
		if (check_repeat_video(video_info, pvideo)) {
			printf("repeat video: %s\n\n", pvideo->title);
			return;
		}

		memcpy(video, pvideo, sizeof(struct video_info));
		current_video_index++;
		if (current_video_index > MAX_VIDEO_NUM) {
			printf("video number is more than %d\n", MAX_VIDEO_NUM);
		}
	}
	cJSON_Delete(root);

	printf("video num is %d\n", current_video_index);
	printf("video duration: %d seconds \n", video->duration);
	printf("video seed: %llu \n", video->seed);
	printf("video size: %llu MB\n", video->size);
	printf("video title: %s\n\n", video->title);
}