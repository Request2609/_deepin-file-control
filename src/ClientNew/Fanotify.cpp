#include "Fanotify.h"

std::shared_ptr<Fanotify>Fanotify::notify = nullptr ;

Fanotify :: Fanotify() {
}

Fanotify :: ~Fanotify() {
    close(fanFd) ;
}

void Fanotify:: DetectOpenClose(int fanFd) {
    int ret = fanotify_mark(fanFd, FAN_MARK_ADD, FAN_CLOSE|FAN_OPEN, AT_FDCWD, paths.c_str()) ;
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
    if(fanFd < 0) {
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

int Fanotify :: GetNotifyFD() {
    return fanFd ;
}

void Fanotify:: StartListen() {
    int len = 0 ;
    int num = 0 ;
    //使用select监听
    if((num = ep->wait(-1, fdList)) > 0) {
        for(int i=0; i<num; i++) {
            char buf[4096];
            len = read(fanFd, buf, sizeof(buf)) ;
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
                      
            }
        }
    }
    std::cout << strerror(errno) << std :: endl ;
}

int Fanotify:: ConnectServer() {
    int servFd = Connect(ip.c_str(), port) ;
    if(servFd < 0) {
        return -1 ;
    }
    return servFd ;
}


//处理标志位
int Fanotify::ProcessBaseFlag(int flag, struct fanotify_event_metadata* metadata) {
    int ret = -1 ;
    switch(flag) {
    //打开前将文件发送给服务器
    case OPEN_PERMIT :
        if(servFd == -1) {
            //还没建立过链接
            ret = ConnectServer() ;
            if(ret < 0) {
                //不会给内核发消息，用户打不开文件
                return -1;
            }
            //可以连接服务器
        }
        else {
            if(ret < 0) {
                return -1 ;
            }
        }
        ////////////////////////////////
        ///向内核发送消息，设置允许打开标志位，并设置监控事件open close
    case OPEN :
        break ;
    case CLOSE :
        break ;
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
            HandlePerm(fanFd, metadata) ;
            //操作文件
            DetectOpenClose(fanFd) ;
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

