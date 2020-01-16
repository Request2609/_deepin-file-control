#include "Fanotify.h"

Fanotify :: Fanotify() {
}

Fanotify :: ~Fanotify() {
    close(fanFd) ;
}

void Fanotify:: detectOpenClose() {
    int ret = fanotify_mark(fanFd, FAN_MARK_ADD, FAN_CLOSE|FAN_OPEN, AT_FDCWD, paths.c_str()) ;
    if(ret < 0) {
        std:: cout << __FILE__ << "    " << __LINE__ << std:: endl ;
    }
}

//设置监控对象为目录下的子文件
void Fanotify::setNotifyObject(std::string path) {
    this->paths = path ;
    fanFd = fanotify_init(FAN_CLASS_CONTENT, O_RDWR) ;
    if(fanFd < 0) {
        std :: cout << __FILE__ << "   " << __LINE__ << "     " << strerror(errno)<< std :: endl ;
        return ;
    }
    std::cout << "初始化完成" << std::endl ;
    uint64_t fanMask = 0;
    fanMask |= FAN_OPEN_PERM ;
    fanMask |= FAN_ALL_PERM_EVENTS ;
    int ret = fanotify_mark(fanFd, FAN_MARK_ADD, fanMask|FAN_EVENT_ON_CHILD, 
                            AT_FDCWD, path.c_str()) ;
    if(ret < 0) {
        std::cout << __LINE__ <<"   " __FILE__ << "   " << strerror(errno) << std::endl ;
    }
    std::cout << "对象处理完成"<< std::endl ;
}

int Fanotify :: getNotifyFD() {
    return fanFd ;
}

void Fanotify:: startListen() {
    char buf[4096];
    int len = 0 ;
    fd_set rfd ;
    //使用select监听
    FD_ZERO(&rfd) ;
    FD_SET(fanFd, &rfd) ;
    std::cout << "开始监听" << std::endl ;
    selectEvent(&rfd) ;
    std:: cout << "发生事件" <<std::endl ;
    while((len = read(fanFd, buf, sizeof(buf))) > 0) {
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
            getEvent(metadata, len) ;
            std::cout << "检测完成" << std::endl ;
        }
        selectEvent(&rfd) ;
    }
    std::cout << strerror(errno) << std :: endl ;
}

int Fanotify:: selectEvent(fd_set* rfd) {
   std::cout << "阻塞" << std::endl ;
    while(select(fanFd+1, rfd, NULL, NULL, NULL) < 0) {
        if(errno != EINTR) {
            std ::cout << __LINE__ <<  "     " << std::endl ;
            exit(0) ;
        }
    }
    return 1 ;
}

int Fanotify::getEvent(const struct fanotify_event_metadata* metadata, int len) {
    std::string paths ;
    while(FAN_EVENT_OK(metadata, len)) {
        //处理matadata
        if(metadata->mask&FAN_OPEN) {
            std :: cout << "文件被打开" << std:: endl ;
            int fd = metadata->fd ;
            /*const char* buf = "hello,it's a secret!" ;
            write(fd, buf, strlen(buf)) ;*/
        }   
        if(metadata->mask&FAN_CLOSE) {
            if(metadata->mask&FAN_CLOSE_WRITE) {
                std:: cout << "写关闭" << std:: endl  ;
                return CLOSE_MODIFY ;
            }   
            if(metadata->mask&FAN_CLOSE_NOWRITE) {
                std :: cout << "关闭操作" <<std:: endl ;
            }
        }
        if(metadata->mask&FAN_MODIFY){
            std::cout << "修改" << std :: endl ;
        }
        if(metadata->mask&FAN_OPEN_PERM) {
            std::cout << "open perm" << std::endl  ;
        }
        if(metadata->mask&FAN_ALL_PERM_EVENTS) {
            std::cout << "给内核发送消息" << std::endl ;
            handlePerm(metadata) ;
            //操作文件
            operationFile(metadata->fd) ;
            detectOpenClose() ;
        }
        metadata = FAN_EVENT_NEXT(metadata, len) ;
    }   
    return 1 ;
}   

void Fanotify::operationFile(int fd) {
    const char* buf = "okokokokokokokok" ;
    int ret = write(fd, buf, strlen(buf)) ;
    if(ret < 0) {
        std::cout << "写失败" <<std:: endl ;
    }
    close(fd) ;
}

int Fanotify::handlePerm(const struct fanotify_event_metadata *metadata) {
    struct fanotify_response response_struct;
    int ret;
    response_struct.fd = metadata->fd;
    response_struct.response = FAN_ALLOW;

    ret = write(fanFd, &response_struct, sizeof(response_struct));
    if (ret < 0)
        return ret;

    return 0;
}

