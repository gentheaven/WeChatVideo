# 微信短视频下载



![](cover.jpg)

## 简介

下载微信短视频



## 使用

第一次运行时，**安装证书**。（只需安装一次，今后不再需要安装）



运行本程序；再登录PC微信客户端。

就可以抓取并保存微信短视频了。

点击本程序的“关闭程序”按钮，就可以关闭了。



之后，如果PC 重启，则需要先运行本程序；再登录PC微信客户端。

否则，两者启动的顺序无所谓。



## ISAAC 解密视频

采用开源代码 Coreutils（以下两个链接内容一样，不过“码云”访问快一些）

[coreutils/coreutils: github](https://github.com/coreutils/coreutils)

[coreutils: gitee](https://gitee.com/mirrors_coreutils/coreutils)

代码在 coreutils-master\gl\lib\ 目录下

rand-isaac.c rand-isaac.h

这两个文件可以单独拿出来，加入项目。

------



# 软件架构

运行平台： windows 10 专业版 （版本 22H2）

开发工具：VS2022 社区版

开发语言：C 语言

| 开源代码   | 版本   | 集成方式   | 备注                             |
| ---------- | ------ | ---------- | -------------------------------- |
| openssl    | 3.5.4  | 动态链接库 | SSL 协议，网站下载编译好的SDK    |
| cJSON      | 1.7.18 | 源代码     | JSON 解析                        |
| Isaac      | NA     | 源代码     | 视频解密                         |
| webui      | 2.4.2  | 动态链接库 | UI 界面                          |
| cmitmproxy | NA     | 动态链接库 | 本人开发的 MITM 代理，破解 https |
| curl       | 8.12.1 | 可执行程序 | 下载微信视频                     |



| Open Source | Version | integrated mode                       |
| ----------- | ------- | ------------------------------------- |
| openssl     | 3.5.4   | libssl-3-x64.dll, libcrypto-3-x64.dll |
| cJSON       | 1.7.18  | source code                           |
| Isaac       | NA      | source code                           |
| webui       | 2.4.2   | webui-2.dll                           |
| cmitmproxy  | NA      | cmitmproxy.dll                        |
| curl        | 8.12.1  | libcurl-x64.dll, curl.exe             |



------

# 文章列表



文章1 ： **实现思路和程序框架**

[下载微信短视频 - 知乎](https://zhuanlan.zhihu.com/p/1934960304086840554)



文章2： **mitmproxy 代替 nghttp2**

[下载微信短视频2 - nghttp2 - 知乎](https://zhuanlan.zhihu.com/p/1936375408720340203)

本来想用 nghttp2 做代理，解密 https。

后来发现 nghttp2 不合适，最终选择用 mitmproxy 做代理。



文章3： **加密种子**

修改 JavaScript 文件内容，得到文章标题和加密种子

2025/8/8 知道了如何修改，何时修改 JS 文件内容。

并且得到了 httpbin 回应的 JSON 数据。

[下载微信短视频3 - 视频加密种子 - 知乎](https://zhuanlan.zhihu.com/p/1937476531355092940)



文章4： **解密视频**

解密算法一探。

2025/8/9 知道如何根据加密种子计算 128KB 密钥。

输入：64位（8字节）加密种子

输出： 128KB 长度的密钥

这个密钥和加密后的视频的前 128KB 做异或操作，就解密视频了。

[下载微信短视频4 - 解密视频 - 知乎](https://zhuanlan.zhihu.com/p/1937802991915759515)



文章5： **总结**

用到了这些技术：

**正则表达式替换**：替换 JavaScript 文件内容，用于得到文章标题和加密种子

**httpbin**/go httpbin（可选）： post请求：https://httpbin.org/post  ，得到文章标题和加密种子

**MITM** （Man-in-the-Middle Attack）  ： 解密 https 通讯，可执行程序和相关库

**WebUI**： GUI库，用于开发图形界面，动态链接库

**Curl**： 用于下载视频，可执行程序

[下载微信短视频5 - 总结 - 知乎](https://zhuanlan.zhihu.com/p/1938127782413578887)



------



# 开发日志

2025/8/1 开始开发

2025/8/23 基本完成。



## V0.4

2025/12/9 开始基于 cmitmproxy 代理开发。

之前基于 mitmproxy 开发，这是 python 库；

2025/9 - 2025/12，花了 3个月时间，自己开发了一个 c语言的 MITM 库。

所以，基于 V0.4，重新开发。



V0.4 之前：基于 mitmproxy 开发。包含 V0.4 版本。VS2019 社区版开发。

V0.4 之后：基于 cmitmproxy 开发。VS2022 社区版开发。



## V0.5

2025/12/23 基本完成。

基于 cmitmproxy V0.5开发。







