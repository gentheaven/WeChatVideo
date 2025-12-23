from mitmproxy import http
import re
import json

        
def response(flow: http.HTTPFlow) -> None:
    content_type = flow.response.headers.get("content-type", "").lower()
    
    # 检查是否是JS文件且URL中包含"virtual_svg"
    if (flow.response.headers.get("content-type", "").lower().startswith("application/javascript") and
        "virtual_svg-icons-register.publish" in flow.request.path.lower()):
        print(flow.request.path);
        
        # 获取原始JS内容
        original_js = flow.response.text
        
        # 使用正则表达式替换内容
        modified_js = re.sub(
            r'async finderGetCommentDetail\((\w+)\)\{return(.*?)\}async',  # 匹配
            r"async finderGetCommentDetail(\1){const feedResult=await\2;var data_object=feedResult.data.object;var media=data_object.objectDesc.media[0];var fetch_body={duration:media.spec[0].durationMs,title:data_object.objectDesc.description,url:media.url+media.urlToken,size:media.fileSize,key:media.decodeKey,id:data_object.id,nonce_id:data_object.objectNonceId,nickname:data_object.nickname,createtime:data_object.createtime,fileFormat:media.spec.map(o => o.fileFormat)};fetch('https://www.httpbin.org/post',{method:'POST',headers:{'Content-Type':'application/json',},body:JSON.stringify(fetch_body)}).then(response=>{console.log(response.ok,response.body)});return feedResult;}async",      # 替换
            original_js, flags=re.MULTILINE
        )
        
        # 更新响应
        flow.response.text = modified_js
        flow.response.headers.pop("Content-Length", None)
     
     
     
        
          # 处理JSON响应
    if "application/json" in content_type and flow.response.content:
        try:
            json_data = json.loads(flow.response.content)
            print("\n=== JSON内容 ===")
            print(json.dumps(json_data, indent=2))
        except json.JSONDecodeError:
            print("无效的JSON数据")
            print(flow.response.text)
        
   