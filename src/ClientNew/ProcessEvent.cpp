#include "ProcessEvent.h"


void ProcessEvent:: GetNotify() {
    notify = Fanotify::GetNotify() ;
}

void ProcessEvent::StartListen(std::string path) {
    char buf[BUFSIZE] ;
    //切换工作目录
    chdir(path.c_str()) ;
    //监控目录下的所有目录子文件
    notify->InitFanotify() ;
    notify->SetNotifyObject(path.c_str()) ;
    
    int len = 0 ;
    while((len = read(fanFd, buf, sizeof(buf))) > 0) {
        struct fanotify_event_metadata * metadata ;
        metadata = (fanotify_event_metadata*)buf ;
        int flag = notify->GetEvent(metadata, len) ;
        int ret = ProcessBaseFlag(flag, metadata) ;
        
    }
}
void ProcessEvent::SetIpPortPath(std::string ip, int port, std::string path) {
    this->ip = ip ;
    this->port = port ; 
    this->path = path ;
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


