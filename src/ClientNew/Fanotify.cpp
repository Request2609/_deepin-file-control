#include "Fanotify.h"

std::shared_ptr<Fanotify>Fanotify::notify = nullptr ;

Fanotify :: Fanotify() {
}

Fanotify :: ~Fanotify() {
    for(auto s : fdAbsolutePathPair) {
        close(s.first) ;
    }
}

void Fanotify:: DetectEvent(int fanFd, int mask) {
    int ret = fanotify_mark(fanFd, FAN_MARK_ADD, mask, AT_FDCWD, paths.c_str()) ;
    if(ret < 0) {
        std:: cout << __FILE__ << "    " << __LINE__ << std:: endl ;
    }
}

void Fanotify::SetIpPort(std::string ip, int port) {
    this->ip = ip ;
    this->port = port ;
}

void Fanotify :: InitFanotify() {
    servFd = -1 ;
    ep = std::make_shared<epOperation>() ;
}

//设置监控对象为目录下的子文件
void Fanotify::SetNotifyObject(std::string path) {
    std::string tmp = path ;
    int fd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR) ;
    if(fd < 0) {
        std :: cout << __FILE__ << "   " << __LINE__ << "     " << strerror(errno)<< std :: endl ;
        return ;
    }
    uint64_t fanMask = 0;
    fanMask |= FAN_OPEN_PERM ;
    int ret = fanotify_mark(fd, FAN_MARK_ADD, fanMask|FAN_EVENT_ON_CHILD, 
                            AT_FDCWD, path.c_str()) ;
    if(ret < 0) {
        std::cout << __LINE__ <<"   " __FILE__ << "   " << strerror(errno) << std::endl ;
    }
    fdAbsolutePathPair.insert({fd, path}) ;
    //设置读事件
    ep->add(fd, POLLIN) ;
    //递归遍历目录
    DIR *dir ;
    struct dirent *ptr ;
    if((dir = opendir(path.c_str())) == NULL) {
        std::cout << __LINE__ << "   " << __FILE__ << std::endl; 
        exit(1) ;
    }   
    while((ptr = readdir(dir)) != NULL) {
        //是目录，讲目录加到
        if(ptr->d_type == 4) {
            if(!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) {
                continue ;
            }   
            path = tmp+'/'+ptr->d_name  ;
            SetNotifyObject(path) ;
        }   
        else {
            continue ;
        }   
    }  
}

void Fanotify:: StartListen(vector<InfoNode>&ls) {
    int len = 0 ;
    int num = 0 ;
    ls.clear() ;
    //使用select监听
    if((num = ep->wait(-1, fdList)) > 0) {
        for(int i=0; i<num; i++) {
            ActiveNode anode ;
            anode.isBackUp = -1 ;
            InfoNode in ;
            char buf[4096];
            //服务器事件
            if(fdList[i] == servFd) {
                in.fileFd = servFd ;
                in.type = -1 ;
                in.fanFd = fdList[i] ;
                return ;
            }
            len = read(fdList[i], buf, sizeof(buf)) ;
            struct fanotify_event_metadata* metadata ;
            char path[PATH_MAX] ;
            int pathLen ;
            metadata = (fanotify_event_metadata*)buf ;
            if(metadata->fd >= 0) {
                sprintf(path, "/proc/self/fd/%d", metadata->fd) ;
                pathLen = readlink(path, path, sizeof(path)-1) ;
                if(pathLen < 0) {
                    std :: cout << __LINE__ << "     " << __FILE__ << std::endl ;
                    exit(1) ;
                }
                path[pathLen] = '\0' ;
                int ret = GetEvent(fdList[i], metadata, len) ;
                if(ret < 0) {
                    cout << __LINE__ << "   " << __FILE__ << endl ;
                    return  ;
                }
                ret = ProcessBaseFlag(ret) ;     
                if(ret < 0) {
                    //关闭被监控描述符的文件描述符
                    close(metadata->fd) ;
                    continue ;
                }
                in.fanFd = fdList[i] ;
                in.fileFd = metadata->fd ;
                in.path = path ;
                if(ret == OPEN_PERMIT) {
                    anode = AddCountIfExist(fdList[i]) ;                   
                    if(anode.count != 0){ 
                        HandlePerm(fdList[i], metadata) ;
                        DetectEvent(fdList[i], FAN_CLOSE) ;
                        continue ;
                    }
                    anode.fanFd = fdList[i] ;
                    anode.path = path ;
                    anode.fileFd = in.fileFd ;
                    //将会被备份
                    anode.isBackUp = 0 ;
                    //为1为打开
                    in.type = 1 ;
                }
                //关闭文件
                else {  
                    int ret = Fanotify::Modify(path, metadata) ;
                    //引用计数不为0的话，继续减
                    if(ret != 0) {
                        continue ;
                    }
                    //是关闭的文件的事件
                    in.type = 0 ;
                }
                if(anode.isBackUp != -1) 
                    Fanotify::activeMap.push_back(anode) ;
                ls.push_back(in) ;
            }       
        }
        fdList.clear() ;
    }
}

struct ActiveNode Fanotify::AddCountIfExist(int fanFd) {
    struct ActiveNode node ;
    int len = Fanotify::activeMap.size() ;
    for(int i=0; i<len; i++) {
        if(Fanotify::activeMap[i].fanFd == fanFd) {
            //引用计数加1
            Fanotify::activeMap[i].count++ ;
            return Fanotify::activeMap[i] ;
        }
    } 
    return node ;
}

void Fanotify::Remove(int fanFd) {
    for(auto s=activeMap.begin(); s != activeMap.end(); s++) {
        if(s->fanFd == fanFd) {
            activeMap.erase(s) ;
        }
    }
}

int Fanotify::Modify(string path, struct fanotify_event_metadata* metadata) {
    int count = 0 ;
    int len = activeMap.size() ;
    for(int i=0; i<len; i++) {
        if(activeMap[i].path == path) {
            activeMap[i].fileFd = metadata->fd ;           
            activeMap[i].count-- ;
            count = activeMap[i].count ;
        }
    }
    return count ;
}

int Fanotify:: ConnectServer() {
    servFd = Connect(ip.c_str(), port) ;
    if(servFd < 0) {
        return -1 ;
    }
    isConnect = 1 ;
    ep->add(servFd, EPOLLIN|EPOLLONESHOT) ;
    return servFd ;
}


int Fanotify::RemoveServer() {
    ep->del(servFd) ;
    isConnect = -1 ;
    return 1 ;
}

void Fanotify :: ModifyServFd(int mask) {
    ep->change(servFd, mask) ;
}

ActiveNode* Fanotify::GetHandleByPath(string name) {
    struct ActiveNode an ;
    int len = activeMap.size() ;
    for(int i=0; i<len; i++) {
        if(activeMap[i].path == name) {
            return &activeMap[i];
        }
    }
    //没有找到备份的文件
    an.isBackUp = -5 ;
    return NULL ;
}

//操作文件
void Fanotify:: OperateFile(struct fanotify_event_metadata* metadata) {
    int fd = metadata->fd ;
    ftruncate(fd, 0) ;
    lseek(fd, 0, 0) ;
    const char* buf="It's a secret!" ;
    write(fd, buf, strlen(buf)) ;
    //关闭fd
    close(fd) ;
}

//处理标志位
int Fanotify::ProcessBaseFlag(int flag) {
    int ret = -1 ;
    switch(flag) {
    //打开前将文件发送给服务器
    case OPEN_PERMIT :
        if(servFd == -1 || isConnect == 0) {
            //还没建立过链接
            servFd= ConnectServer() ;
            if(ret < 0) {
                //不会给内核发消息，用户打不开文件
                return -1;
            }
        }
        else {
            if(ret < 0) {
                return -1 ;
            }
        }
        ////////////////////////////////
        ///向内核发送消息，设置允许打开标志位，并设置监控事件open close
      //  HandlePerm(fanFd, metadata) ;
        //修改文件内容
     //   OperateFile(metadata) ;
        return OPEN_PERMIT ;
        //监控关闭事件
        //DetectClose(fanFd) ;
        //设置关闭事件
        //准备使用inotify进行管理
        //fanotify在这一方面功能较少
    case CLOSE :
        return CLOSE ;
    }
}


int Fanotify::GetEvent(int fanFd, const struct fanotify_event_metadata* metadata, int len) {
    std::string paths ;
    int flag = 1 ;
    while(FAN_EVENT_OK(metadata, len)) {
        //处理matadata
        if(metadata->mask&FAN_OPEN) {
            flag = OPENING ;
        }   
        if(metadata->mask&FAN_CLOSE) {
            if(metadata->mask&FAN_CLOSE_WRITE) {
                flag = CLOSE_MODIFY ;
            }   
            if(metadata->mask&FAN_CLOSE_NOWRITE) {
                flag =  CLOSED ;
            }
        }
        if(metadata->mask&FAN_MODIFY){
            flag = MODIFY ;
        }
        if(metadata->mask&FAN_OPEN_PERM) {
            flag = OPEN_PERMIT ;
        //    HandlePerm(fanFd, metadata) ;
            //操作文件
       //     DetectOpenClose(fanFd) ;
        }
        metadata = FAN_EVENT_NEXT(metadata, len) ;
    }   
    return flag ;
}   

int Fanotify::HandlePerm(int fanFd, const struct fanotify_event_metadata *metadata) {
    struct fanotify_response response_struct;
    int ret;
    response_struct.fd = metadata->fd;
    response_struct.response = FAN_ALLOW;
    ret = write(fanFd, &response_struct, sizeof(response_struct));
    if (ret < 0)
        return ret;

    return 0;
}

