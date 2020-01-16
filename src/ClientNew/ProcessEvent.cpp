#include "ProcessEvent.h"


void ProcessEvent:: GetNotify() {
    notify = Fanotify::GetNotify() ;
}

void ProcessEvent::StartListen(std::string path) {
    char buf[BUFSIZE] ;
    notify->SetNotifyObject(path.c_str()) ;
    fanFd = notify->GetNotifyFD() ;
    fd_set rfd ;
    FD_ZERO(&rfd) ;
    FD_SET(fanFd, &rfd) ;
    SelectEvent(&rfd) ;
    int len = 0 ;
    while((len = read(fanFd, buf, sizeof(buf))) > 0) {
        struct fanotify_event_metadata * metadata ;
        metadata = (fanotify_event_metadata*)buf ;
        int flag = notify->GetEvent(metadata, len) ;
        int ret = ProcessBaseFlag(flag, metadata) ;
        SelectEvent(&rfd) ;
    }
}

int ProcessEvent:: ConnectServer() {
    int servFd = Connect(ip.c_str(), port) ;
    if(servFd < 0) {
        return -1 ;
    }
    return servFd ;
}

void ProcessEvent::SetIpPortPath(std::string ip, int port, std::string path) {
    this->ip = ip ;
    this->port = port ; 
    this->path = path ;
}

//处理标志位
int ProcessEvent::ProcessBaseFlag(int flag, struct fanotify_event_metadata* metadata) {
    int ret = -1 ;
    switch(flag) {
    //打开前将文件发送给服务器
    case OPEN_PERMIT :
        if(servFd == -1) {
            ret = ConnectServer() ;
            if(ret < 0) {
                //不会给内核发消息，用户打不开文件
                return -1;
            }
            //可以连接服务器
        }
        else {
            int ret = IsConnect(servFd, ip.c_str(), port) ; 
            if(ret < 0) {
                return -1 ;
            }
        }
        BackUpToServer() ;      
        notify->HandlePerm(metadata) ;

    case OPEN :
        break ;
    case CLOSE :
        break ;
    }
}
//给服务器发送文件
void ProcessEvent::BackUpToServer(struct fanotify_event_metadata* metadata) {
       
}

int ProcessEvent::SelectEvent(fd_set* rfd) {
    while(select(fanFd+1, rfd, NULL, NULL, NULL) < 0) {
        if(errno != EINTR) {
            std ::cout << __LINE__ <<  "     " << std::endl ;
            exit(0) ;
        }
    }
    return 1 ;
}


