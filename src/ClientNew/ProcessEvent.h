#pragma once
#include <iostream>
#include <map>
#include "Fanotify.h"
#include "Judge.h"

const int BUFSIZE = 4096 ;
class ProcessEvent {
public:
    ProcessEvent() {
        servFd = -1 ;
    }
    ~ProcessEvent() {}
    void GetNotify() ;
    void SetIpPortPath(std::string ip, int port, std::string path) ;
    void StartListen(std::string path) ;
    int SelectEvent(fd_set* rfd) ;
    void BackUpToServer(struct fanotify_event_metadata* metadata) ;
private:    
    std::shared_ptr<Fanotify>notify ;
    int fanFd ;
    std::string ip ;
    int port ;
    std::string path ;
    int servFd ;
};

