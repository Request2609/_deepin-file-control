

### 【管控客户端第2版】

<div class="markdown-body">
          <h2>
<a id="user-content-项目概况" class="anchor" href="#%E9%A1%B9%E7%9B%AE%E6%A6%82%E5%86%B5" aria-hidden="true"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z"></path></svg></a>项目概况</h2>
<h3>
<a id="user-content-背景" class="anchor" href="#%E8%83%8C%E6%99%AF" aria-hidden="true"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z"></path></svg></a>背景</h3>
<ul>
<li>项目来源：<a href="https://www.deepin.org/devcon-2019/topic" rel="nofollow">2019 深度软件开发大赛</a>
</li>
<li>项目名称：文件管控客户端</li>
</ul>
<h3>
<a id="user-content-运行环境" class="anchor" href="#%E8%BF%90%E8%A1%8C%E7%8E%AF%E5%A2%83" aria-hidden="true"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z"></path></svg></a>运行环境</h3>
<ul>
<li>deepin Linux x86_64 系统，理论上也兼容其他 x86_64 Linux 系统</li>
</ul>
<h3>
<a id="user-content-条件与限制" class="anchor" href="#%E6%9D%A1%E4%BB%B6%E4%B8%8E%E9%99%90%E5%88%B6" aria-hidden="true"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z"></path></svg></a>条件与限制</h3>
<ul>
<li>区分服务端与客户端，一般运行在不同的机器上</li>
<li>正式的运行环境是N(N&gt;=2)台计算机节点，通过有线或者无线互联，且运行服务端机器有客户端机器能直连的IP地址</li>
</ul>
<h3>
<a id="user-content-业务功能需求" class="anchor" href="#%E4%B8%9A%E5%8A%A1%E5%8A%9F%E8%83%BD%E9%9C%80%E6%B1%82" aria-hidden="true"><svg class="octicon octicon-link" viewBox="0 0 16 16" version="1.1" width="16" height="16" aria-hidden="true"><path fill-rule="evenodd" d="M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z"></path></svg></a>业务功能需求</h3>
<ul>
<li>监视和记录指定目录下文件的打开和关闭动作（文件系统事件），并上报服务端（文件路径、文件句柄、操作方式等）</li>
<li>配置了指定的监视服务器情况下，可接受服务端下发的文件操作指令，包括不限于读取、删除、重写等</li>
<li>当监视服务器关闭或者网络不通时，客户端拒绝任何操作，不允许任何人打开文件（拒绝监视目录的一切文件操作）</li>
<li>监视服务器不存在或者未配置时，记录相关的操作到日志文件，监视服务器可以查询历史操作日志</li>
</ul>
<h3>


### 客户端详细设计

##### 根据主成功场景实现

- 客户端开始在配置文件中设置连接远程服务器需要的端口，IP地址，以及要监控的文件的绝对路径
- 程序开始是从配置文件中获取信息，检查信息
- 检查完成，根据用户设置的路径名，是目录的话，递归遍历目录，设置fd为fanoitify打开权限检查事件，并将获取的句柄加到epoll中
- 当有用户打开监控目录下的任意文件时，管控客户端epoll会检测到可读事件，在其打开之前就将其拦截，客户端检查和服务器连接是否正常，要是连接不上服务器，即拒绝本次打开文件操作；要是能连上服务器，先将文件中的内容备份到服务器，并修改文件内容，然后通知内核允许本次文件打开操作，并注册修改监听事件类型。然后用户打开文件看到的并非真实的文件内容。用户所做的修改文件操作都是无效的
- 当检测到文件关闭操作时，管控客户端给服务器发送消息，通知恢复文件，服务器返回保存的原文件数据。当客户端不工作的时候，用户可以正常访问文件的真实内容
- 其中向服务器发送消息由线程池负责


### 文件说明

| 目录名称 |          用途          |
| :------: | :--------------------: |
|   conf   |        配置文件        |
|   src    |       源代码文件       |
|  tests   |        测试文件        |
|   \*.md   |        说明文件        |
| start.sh | 开始运行客户端脚本文件 |
|  images  |   运行图片或视频文件   |


### 运行

- 用户需要在conf/info文件中设置相关信息
- 执行./start.sh
- 运行服务器程序：./Server PORT IP

### 运行截图

- 和服务器正常连接视频演示

[![image-20200119190036639](images/2.png)](https://github.com/Request2609/_deepin-file-control/blob/master/images/connected.mp4?raw=true)

- 连不上服务器视频演示

[![image-20200119190326073](images/1.png)](https://github.com/Request2609/_deepin-file-control/blob/master/images/unconnect.mp4?raw=true)



### 总结

这是第二版本改近的客户端程序，第一版使用hook和消息队列实现的！要是感兴趣欢迎移步到[这里](https://github.com/xiyou-linuxer/_deepin-file-control)查看，详细说明在[wiki](https://github.com/xiyou-linuxer/_deepin-file-control/wiki)中！
