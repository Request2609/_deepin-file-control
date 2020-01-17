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

struct InfoNode {
    int fileFd; 
    int fanFd ;
    int type ;
    string path ;
} ;

struct ActiveNode {
    string path ;
    int fanFd ;
    int fileFd ;
    //是否已经备份成功
    int isBackUp ;
    //引用计数
    int count = 0;
} ;

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
    //添加引用计数
    struct ActiveNode AddCountIfExist(int fanFd) ;
    void StartListen(vector<InfoNode>&) ;
    void DetectEvent(int fanFd, int mask) ;
    void DetectWrite() ;
    int ConnectServer() ;
    //在监控期间可以操作文件
    int GetEvent(int fanFd, const struct fanotify_event_metadata* metadata, int len) ;
    int HandlePerm(int, const struct fanotify_event_metadata* metadata) ;
    int RemoveServer() ;
    void InitFanotify() ;
    int ProcessBaseFlag( int flag) ;
    void SetIpPort(std::string ip, int port) ;
    void OperateFile(struct fanotify_event_metadata* metadata) ;
    int GetServFd() { return servFd ;}
    static int Modify(string path, struct fanotify_event_metadata* metadata) ;
    static ActiveNode* GetHandleByPath(string name) ;
    static void Remove(int) ;
    void ModifyServFd(int mask) ;
    //开始监听函数
private:
    int isConnect ;
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
    //第一个是正在备份的文件的绝对路径，
    static std::vector<ActiveNode> activeMap;
    shared_ptr<epOperation>ep ;
};

vector<ActiveNode> Fanotify::activeMap ;

