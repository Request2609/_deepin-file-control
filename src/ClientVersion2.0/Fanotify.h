#pragma once
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <map>
#include <linux/limits.h>
#include <iostream>
#include <signal.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <linux/fanotify.h>
#include "FanotifySyscallLib.h"
#include "Epoll.h"
#include "Client.h"

class epOperation ;
const int OPENING = 1 ;
const int MODIFY = 2 ;
const int CLOSE_MODIFY = 3;
const int CLOSED = 4 ;
const int OPEN_PERMIT = 5 ;
const int FAN_ALL_PERM = 6 ;

struct InfoNode {
    int fileFd; 
    int fanFd ;
    int type ;
    std::string path ;
} ;

struct ActiveNode {
    std::string path ;
    int fanFd ;
    int fileFd ;
    //是否已经备份成功
    int isBackUp ;
    //引用计数
    int count = 0;
} ;

class Fanotify {
    Fanotify();
public:
    ~Fanotify() ;
    //选择检测对象
    void SetNotifyObject(std::string path) ;
    //获取句柄
    int GetNotifyFD() ;
    //添加引用计数
    struct ActiveNode AddCountIfExist(int fanFd) ;
    void StartListen(std::vector<InfoNode>&) ;
    void DetectEvent(int fanFd, int mask) ;
    void DetectWrite() ;
    int ConnectServer() ;
    //在监控期间可以操作文件
    int GetEvent(int fanFd, const struct fanotify_event_metadata* metadata, int len) ;
    int HandlePerm(int, int, const struct fanotify_event_metadata* metadata) ;
    int RemoveServer() ;
    void InitFanotify(std::string ip, int port) ;
    int ProcessBaseFlag(int flag, struct fanotify_event_metadata* metadata) ;
    void SetIpPort(std::string ip, int port) ;
    void OperateFile(struct fanotify_event_metadata* metadata) ;
    void ClearFile(int fd) ;
    int GetServFd() { return servFd ;}
    static std::shared_ptr<Fanotify> GetNotify() ;
    static int Modify(std::string path, struct fanotify_event_metadata* metadata) ;
    static ActiveNode* GetHandleByPath(std::string name) ;
    static void Remove(int) ;
    void ModifyServFd(int mask) ;
    //开始监听函数
private:
    int isConnect ;
    std::string ip ;
    int port ;
    int servFd ;
    std::vector<int>fdList ;
    //使用单实例模式
    static std::shared_ptr<Fanotify>notify ;
    std::map<int, std::string>fdAbsolutePathPair ;
    //第一个是正在备份的文件的绝对路径，
    static std::vector<ActiveNode> activeMap;
    std::shared_ptr<epOperation>ep ;
};


