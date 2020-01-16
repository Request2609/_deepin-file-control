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

const int OPENING = 1 ;
const int MODIFY = 2 ;
const int CLOSE_MODIFY = 3;
const int CLOSED = 4 ;
const int OPEN_PERMIT = 5 ;
const int FAN_ALL_PERM = 6 ;

class Fanotify {
public:
    ~Fanotify() ;

    static std::shared_ptr<Fanotify>GetNotify() {
        if(notify == nullptr) {
            notify = std::make_shared<Fanotify>() ;
        }
        return notify ;
    } 
    //选择检测对象
    void SetNotifyObject(std::string path) ;
    //获取句柄
    int GetNotifyFD() ;
    void StartListen() ;
    void DetectOpenClose(int) ;
    void DetectWrite() ;
    int ConnectServer() ;
    //在监控期间可以操作文件
    int GetEvent(int fanFd, const struct fanotify_event_metadata* metadata, int len) ;
    int HandlePerm(int, const struct fanotify_event_metadata* metadata) ;
    void InitFanotify() ;
    int ProcessBaseFlag(int flag, struct fanotify_event_metadata* metadata) ;
    void SetIpPort(std::string ip, int port) ;
    //开始监听函数
private:
    std::string ip ;
    int port ;
    int servFd ;
    Fanotify() ;
    vector<int>fdList ;
    //设置检测对象的
    std::string paths ;
    //使用单实例模式
    static std::shared_ptr<Fanotify>notify ;
    std::map<int, std::string>fdAbsolutePathPair ;
    shared_ptr<epOperation>ep ;
    int fanFd ;
};

