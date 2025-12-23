#include <string.h>

#include <stdio.h>
#include <stdlib.h>

//#define WIN32_LEAN_AND_MEAN
#include "webui.h"
#include <windows.h>
#include "wechat.h"
#include "win.h"
#include "mitm.h"

enum app_state gstate = APP_STATE_UNINIT;

/*
 fetch_video_url(current_idx)；
 response = new_idx, title, size, seed
*/
static char response_js_str[4096];
void fetch_video_url(webui_event_t* e)
{
	struct video_info *pvideo;
	long long idx = webui_get_int_at(e, 0);

	if (current_video_index > MAX_VIDEO_NUM) {
		sprintf(response_js_str, "%lld, video number is more than 32", idx);
		webui_return_string(e, response_js_str);
		return;
	}

	if (idx >= current_video_index) {
		//keep original
		idx = current_video_index;
		sprintf(response_js_str, "%lld", idx);
		webui_return_string(e, response_js_str);
		return;
	}

	if (gstate == APP_STATE_DOWNLOADING) {
		//keep original
		sprintf(response_js_str, "%lld, downloading, 0, 0", idx);
		webui_return_string(e, response_js_str);
		return;
	}
	
	//fetch next video
	printf("fetch new one \n");
	idx++;
	pvideo = &video_info[idx - 1];
	char* utf_str;
	int out_len;
	utf_str = to_utf(pvideo->title, &out_len);
	sprintf(response_js_str, "%lld, %s, %llu, %lld", idx, utf_str, pvideo->size, pvideo->seed);
	free(utf_str);
	printf("fetch idx=%lld, current_video_index = %d\n", idx, current_video_index);
	webui_return_string(e, response_js_str);
}

void reset_videos(char* log)
{
	current_video_index = 0;
	memset(video_info, 0, sizeof(struct video_info) * MAX_VIDEO_NUM);
	printf("clear all video itmes\n");
}

//download video
int start_download(long long id, char* log)
{
	if (id >= (current_video_index + 1)) {
		printf("no this video item id = %lld\n", id);
		return 1;
	}

	struct video_info *pvideo = &video_info[id - 1];
	if (pvideo->state == VIDEO_DOWNLAODED) {
		printf("already downloaded video: %s\n\n", pvideo->title);
		return 0;
	}

	gstate = APP_STATE_DOWNLOADING;

	printf("ongoing downloading video: id:%lld, title:%s\n", id, pvideo->title);
	download_video(pvideo);
	decode_video_with_seed(pvideo->seed, "enc.mp4", "dec.mp4");
	moveto_download_dir(pvideo->title);
	pvideo->state = VIDEO_DOWNLAODED;
	printf("OK to download video: %s\n\n", pvideo->title);
	gstate = APP_STATE_RUN;
	return 0;
}

void close_app(webui_event_t* e)
{
	printf("Exit.\n");

	if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0)) {
		printf("send CTRL+C failed: %lu\n", GetLastError());
	}
	// Close all opened windows
	webui_exit();
}

/*
	flag = 100: close app
	flag = 200: clear all video items
	flag = 300: open download dir
*/
void action_run(webui_event_t* e)
{
	long long flag = webui_get_int_at(e, 0);

	strcpy(response_js_str, "OK");
	if (flag == 100) {
		close_app(e);
	} else if (flag == 200) {
		//clear all video items
		reset_videos(response_js_str);
	} else if (flag == 300) {
		//open download dir
		ShellExecute(
			NULL,         
			L"open",
			L".\\videos",
			NULL,
			NULL,
			SW_SHOWNORMAL
		);
	} else {
		//download video
		int ret;
		ret = start_download(flag, response_js_str);
		if (ret) {
			strcpy(response_js_str, "failed");
		} else {
			strcpy(response_js_str, "OK");
		}
	}

	webui_return_string(e, response_js_str);
}

void webui_init(mitm_ctx* mitm)
{
	// Create a new window
	size_t MainWindow = webui_new_window();

	// Set the root folder for the UI
	//webui_set_root_folder(MainWindow, ".");

	// Bind HTML elements with the specified ID to C functions
	webui_bind(MainWindow, "fetch_video_url", fetch_video_url);
	webui_bind(MainWindow, "action_run", action_run);

	// Show the window, preferably in a chromium based browser
	if (!webui_show_browser(MainWindow, "GUI.html", AnyBrowser))
		webui_show(MainWindow, "GUI.html");

	mitm_run(mitm, NULL); //main loop

	// Wait until all windows get closed
	webui_wait();

	// Free all memory resources (Optional)
	webui_clean();
}

/*
* wechat: "qq.com"

fetch('https://www.httpbin.org/post', {
	method: 'POST',
	headers: {
		'Content-Type': 'application/json',
	},
*/
enum FILTER_RESULT cb_host_wechat(const char* host_name, void* arg)
{
	if (strstr(host_name, "qq.com")) {
		return FILTER_RESULT_SPLIT;
	}

	if (strstr(host_name, "httpbin")) {
		return FILTER_RESULT_SPLIT;
	}

	return FILTER_RESULT_PASS; //forward only
}

//#define DEBUG_EXPORT_FILE
char* replace_js(char* ori, size_t ori_len, size_t* chg_len)
{
	char* regex_match_str;
	char* regex_replace_str;

	size_t len = ori_len + 1024;
	char* chg_content = malloc(len);
	*chg_len = len;

	regex_match_str = "async finderGetCommentDetail\\((\\w+)\\)\\{return(.*?)\\}async";
	regex_replace_str = "async finderGetCommentDetail(\\1){const feedResult=await\\2;var data_object=feedResult.data.object;var media=data_object.objectDesc.media[0];var fetch_body={duration:media.spec[0].durationMs,title:data_object.objectDesc.description,url:media.url+media.urlToken,size:media.fileSize,key:media.decodeKey,id:data_object.id,nonce_id:data_object.objectNonceId,nickname:data_object.nickname,createtime:data_object.createtime,fileFormat:media.spec.map(o => o.fileFormat)};fetch('https://www.httpbin.org/post',{method:'POST',headers:{'Content-Type':'application/json',},body:JSON.stringify(fetch_body)}).then(response=>{console.log(response.ok,response.body)});return feedResult;}async";

	int ret = regex_replace(ori, (unsigned int)ori_len,
		regex_match_str, regex_replace_str, chg_content, chg_len, 1);

	if (ret > 0) {
		printf("match str: %s\n", regex_match_str);
		printf("match %d times\n", ret);
		return chg_content;
	}

	//match failed
	free(chg_content);
	return NULL;
}

/*
* now only support peek http request, can't change it
* 
* if "application/json" in content_type
*	and send to www.httpbin.org
* then handle it
*/
int http_request(http_info_t* http_ctx)
{
	if (!http_ctx->http_host || !http_ctx->http_content_type
		|| !http_ctx->http_method)
		return 0;

	//printf("app: %s %s %s len=%zu\n", http_ctx->http_method, http_ctx->http_host, 
	//	http_ctx->http_content_type, http_ctx->http_content_length);

	//POST
	if(_strnicmp(http_ctx->http_method, "POST", 4))
		return 0;

	//Content-Type: application/json\r\n
	if (_strnicmp(http_ctx->http_content_type, "application/json", strlen("application/json")))
		return 0;

	//Host: www.httpbin.org\r\n
	if (_strnicmp(http_ctx->http_host, "www.httpbin.org", strlen("www.httpbin.org")))
		return 0;

	struct video_info* video = &video_info[current_video_index];
	handle_json_data(http_ctx->http_content, http_ctx->http_content_length, video);
	return 0;
}

int http_response(void* arg, http_info_t* http_ctx,
	char** out, size_t* out_len, FreeFunc* cb_free)
{
	if (!http_ctx->http_content_length)
		return 0;

	if (!http_ctx->response) {
		//handle http request
		http_request(http_ctx);
		return 0;
	}

	//here, http response
	printf("http_response: %s len is %lld\n", http_ctx->http_uri, http_ctx->http_content_length);
	char* chg;
	chg = replace_js(http_ctx->http_content, http_ctx->http_content_length, out_len);
	if (!chg)
		return 0;
	*out = chg;
	printf("replace OK, len is %lld\n\n", *out_len);

#ifdef DEBUG_EXPORT_FILE
	FILE* fp_ori = fopen("ori.js", "wb");
	fwrite(http_ctx->http_content, 1, http_ctx->http_content_length, fp_ori);
	fclose(fp_ori);

	FILE* fp_chg = fopen("chg.js", "wb");
	fwrite(*out, 1, *out_len, fp_chg);
	fclose(fp_chg);
#endif

	*cb_free = free;
	return 1;
}

char* get_file_name_from_uri(char* uri)
{
	char* last = NULL;
	char* cur = uri;
	cur = strchr(uri, '/');
	while (cur) {
		last = cur + 1;
		cur = strchr(last, '/');
	}
	return last;
}

const char js_type_str[] = "application/javascript";
/*
* Request URI: /t/wx_fed/finder/web/web-finder/res/js/virtual_svg-icons-register.publishDJmRcesj.js

 if (flow.response.headers.get("content-type", "").lower().startswith("application/javascript") and
		"virtual_svg-icons-register.publish" in flow.request.path.lower()):
*/
const char virtual_svg_str[] = "virtual_svg-icons-register.publish";
int is_cared_info(http_info_t* phi, void* arg)
{
	//not find Content-Type in http header
	if (!phi->http_content_type || !phi->http_uri)
		return 0;

	if (_strnicmp(js_type_str, phi->http_content_type, strlen(js_type_str))) {
		return 0;
	}
	//here, Content-Type: application/javascript\r\n
	//start point to file name
	char* start = get_file_name_from_uri(phi->http_uri);
	if (!start)
		return 0;
	if (_strnicmp(start, virtual_svg_str, strlen(virtual_svg_str))) {
		return 0;
	}

	//virtual_svg-icons-register.publish...
	printf("cared cb: %s, len is %lld\n\n", phi->http_uri, phi->http_content_length);
	return 1;
}

int main(int argc, char** argv)
{
	int ret = -1;
	gstate = APP_STATE_UNINIT;

	mitm_ctx* mitm = mitm_init(DEFAULT_PROXY_ADDR, DEFAULT_PROXY_PORT);
	if (!mitm)
		goto fail;
	gstate = APP_STATE_INITED;
	//define filter system
	register_filter_cb_host(mitm, cb_host_wechat);
	register_filter_cb_cared(mitm, is_cared_info);
	register_action_cb_http(mitm, http_response);

	gstate = APP_STATE_RUN;
	//start UI
	webui_init(mitm);
	ret = 0;

fail:
	mitm_exit(mitm);
	return ret;

}
