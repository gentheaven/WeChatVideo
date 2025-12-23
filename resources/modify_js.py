import re

def process_file_with_regex(input_file, output_file, pattern, replacement):
    """
    读取文件，使用正则表达式修改内容，保存为新文件
    
    参数:
    input_file: 输入文件路径
    output_file: 输出文件路径
    pattern: 正则表达式模式
    replacement: 替换内容
    """
    try:
        # 读取文件内容
        with open(input_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 使用正则表达式替换
        modified_content = re.sub(pattern, replacement, content, flags=re.MULTILINE)
        
        # 写入新文件
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(modified_content)
        
        print(f"文件处理完成！已保存到: {output_file}")
        return True
    
    except FileNotFoundError:
        print(f"错误: 文件 '{input_file}' 不存在")
        return False
    except Exception as e:
        print(f"错误: {e}")
        return False

# 使用示例
if __name__ == "__main__":
    process_file_with_regex(
        input_file="ori.js",
        
        output_file="python_chg.js",
        
        pattern=r'async finderGetCommentDetail\((\w+)\)\{return(.*?)\}async',
        
        replacement=r"async finderGetCommentDetail(\1){const feedResult=await\2;var data_object=feedResult.data.object;var media=data_object.objectDesc.media[0];var fetch_body={duration:media.spec[0].durationMs,title:data_object.objectDesc.description,url:media.url+media.urlToken,size:media.fileSize,key:media.decodeKey,id:data_object.id,nonce_id:data_object.objectNonceId,nickname:data_object.nickname,createtime:data_object.createtime,fileFormat:media.spec.map(o => o.fileFormat)};fetch('https://www.httpbin.org/post',{method:'POST',headers:{'Content-Type':'application/json',},body:JSON.stringify(fetch_body)}).then(response=>{console.log(response.ok,response.body)});return feedResult;}async",
    )
    