#include"Client.h"
//客户端默认接收消息为1的消息
//argv参数包含客户端要连接服务器的ip，端口
//要监控的目录

int ProcessHandle(char info[3][128]) {

    std::shared_ptr<threadPool> pool = std::make_shared<threadPool>(8) ;
    std::vector<InfoNode> ls ;
 //   signal(SIGINT, SigHandle) ;
    std::shared_ptr<Fanotify> notify = Fanotify::GetNotify() ;
    int port = atoi(info[1]) ;
    //切换工作路径到监控目录
    //初始化端口地址和IP地址
    
    notify->InitFanotify(info[0], port) ;
    //设置监控路径
    notify->SetNotifyObject(info[2]) ;
    while(1) {
        //开始监听
        notify->StartListen(ls) ;
        int servFd = notify->GetServFd() ;
        //检测服务器是否存活
        //服务器事件
        int len = ls.size() ;
        for(int i=0; i<len; i++) {
            if(ls[i].type==-1) {
                pool->commit(RecvData, servFd) ;
            }
            else {
                //监控的路径
                //其他文件操作事件
                pool->commit(SendData, ls[i], servFd) ;  
            }
        }
    }
}

//连接服务器
int Connect(const char* ip, const int port) {
    
    int fd = socket(AF_INET, SOCK_STREAM, 0) ;
    if(fd < 0) {
        std::cout << __LINE__ <<"   " << __FILE__ << std::endl ;
        return 0 ;
    }
    
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(port) ;
    int ret = inet_aton(ip, &addr.sin_addr) ;
    if(ret < 0) {
        std::cout << __LINE__ <<"   " << __FILE__ << std::endl ;
        return -1 ;
    }
    ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr)) ;
    if(ret < 0) {
        std::cout << __LINE__ <<"   " << __FILE__ <<"    " << strerror(errno)<< std::endl ;
        return -1 ;
    }
    return fd ;
}


//向服务端发送请求

void SendData(struct InfoNode node, int servFd) {

    auto notify = Fanotify::GetNotify() ;
    //文件在监控目录下，并且存在
    if(node.type == CLOSE) {
        //close请求判断是否为最后一次close请求,是最后一次close请求的
        //话,才恢复原来文件内容,否则不会恢复原来文件内容
        int ret =  RecoverRequest(node, servFd) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return ;
        }
    }

    if(node.type == OPEN) {
        //备份文件发送
        int ret = SendFile(servFd, node) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return ;
        }
        //修改文件内容完成
        ModifyFile(node.fileFd) ;
        struct fanotify_event_metadata tmp ;
        tmp.fd = node.fileFd ;
        notify->HandlePerm(FAN_ALLOW, node.fanFd, &tmp) ;
        //关闭该文件事件
        close(node.fileFd) ;
        //设置监控关闭事件
        notify->DetectEvent(node.fanFd, FAN_CLOSE) ;
    }
    notify->ModifyServFd(EPOLLIN|EPOLLONESHOT) ;
}

void ModifyFile(int fd) {
    ftruncate(fd, 0) ;
    lseek(fd, 0, 0) ;
    const char* buf = "It's a secret!" ;
    write(fd, buf, strlen(buf)) ;
}
//发送文件内容
int  SendFile(int servFd, const struct InfoNode& node) {
    //判断是否为已经备份过的文件
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = OPEN ;
    int ret = GetMac(data.mac) ;
    if(ret < 0 ) {
        return -1 ;
    }
    auto notify = Fanotify::GetNotify() ;
    strcpy(data.pathName, node.path.c_str()) ;
    struct stat st ;
    fstat(node.fileFd, &st) ;
    char* p = (char*)mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, node.fileFd, 0) ;
    if(p == NULL) {
        std::cout << __FILE__ << "     " << __LINE__ << std::endl ;
        return -1;
    }
    int sum = 0 ;
    //打开文件
    long cur = 0 ; 
    int index = 0 ;
    int offset = 0;
    while(1) {
        index = 0 ;
        if(st.st_size-cur > 4096){
            for(int i=cur; i<st.st_size; i++) {
                if(index == BUF_SIZE-1) {
                    data.buf[index] = '\0' ;
                    offset = BUF_SIZE ;
                    break ;
                }
                data.buf[index] = p[i] ;
                index++ ;
            }
        }
        else {
            strncpy(data.buf, &p[cur], st.st_size-cur) ;
            offset = st.st_size-cur ;
        }
        sum+=offset ;
        data.left = cur ;
        cur += offset ;
        data.right = cur ;
        cout << data.left <<  "       " << data.right << endl ;
        //向服务器发送文件内容
        int res =  writen(servFd, &data, sizeof(data)) ;
        if(res < 0) {
            printError(__FILE__, __LINE__) ;
            return  -1 ;
        }

        if(cur >= st.st_size) {
            data.type = OPEN ;
            memset(data.buf, 0, sizeof(data.buf)) ;
            //通知发送文件结束
            strcpy(data.buf, "EOF") ;
            //向服务器发送结束标志
            int res = writen(servFd, &data, sizeof(data)) ;
            if(res < 0) {
                printError(__FILE__, __LINE__);
                return -1 ;
            }
            break ;
        }
        if(cur >= st.st_size) {
            break ;
        }
    }
    munmap(p, st.st_size) ;
    return 1 ;
}   

//接收服务端请求
int RecvData(int servFd) {
    auto notify = Fanotify::GetNotify() ;
    struct Data data ;
    int ret ,res ;
    while(1) {
        memset(&data, 0, sizeof(data)) ;
        ret = readn(servFd, &data, sizeof(data)) ;
        //服务器端关闭了
        if(ret == 0) {
            notify->RemoveServer() ;
            return 1 ;
        }
        //要是第一次open请求或者close请求，打开文件
        //其他情况下只写文件
        switch(data.type) {
        //表明服务器备份文件完成了
        case OPEN:
            if(!strcmp(data.buf, "OK")) {
                cout << "文件信息已经被隐藏!" << endl ;
            }
            return 0;
        case CLOSE :
            //文件完成
            struct ActiveNode* anode = Fanotify::GetHandleByPath(data.pathName) ;
            if(anode == NULL|anode->fileFd < 0) {
                cout << __LINE__ << "   " << __FILE__ << endl ;
            }
            res = RecoverFile(data, anode->fileFd) ;
            if(res == 1) {
                //close(anode->fileFd) ;
                anode->fileFd = -1 ;
                Fanotify::Remove(anode->fanFd) ;
            }
            notify->ModifyServFd(EPOLLIN|EPOLLONESHOT) ;
            break ;
        }
    }
    return 0;
}

//向hook程序发送消息
int SendHookMsg(struct Data data, int msgId, int& fd) {

    int ret = -1 ;
    ret = writen(fd, data.buf, strlen((char*)data.buf)) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    Msg msg ;
    //将客户端返回的结果返回给hook
    msg.buf.type = data.type ;
    msg.type = data.hookPid ;
    //给hook发消息
    if(IpcMsgSend(msgId, msg) < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    return 1 ;
}

//发送恢复文件请求
int RecoverRequest(const struct InfoNode& node, int servFd) {
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = CLOSE ;
    int ret = GetMac(data.mac) ;
    strcpy(data.pathName, node.path.c_str()) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    if((ret = writen(servFd, &data, sizeof(data))) < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    return 1 ;
}

int RecoverFile(struct Data data, int& fd) {
    if(!strcmp((char*)data.buf,"EOF")) {
        return 1 ;
    }
    lseek(fd, data.left, SEEK_SET) ;
    int ret = write(fd, data.buf, data.right-data.left) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        exit(1) ;
    }
    return 0 ;
}

//获取文件打开的fd
int GetFileFd(Data data) {
    int fd = 0;
    if(data.type == CLOSE) {
        fd = open(data.pathName, O_WRONLY) ;
        ftruncate(fd, 0) ;
        lseek(fd, 0, 0) ;
        if(fd < 0) {
            printError(__FILE__, __LINE__) ;
            return 0 ;
        }
        return fd ;
    }

    else if(data.type == OPEN) {
        fd = open(data.pathName, O_WRONLY) ;
        ftruncate(fd, 0) ;
        lseek(fd, 0, 0) ;
        if(fd < 0) {
            printError(__FILE__, __LINE__) ;
            return 0 ;
        }
        return fd ;
    }
    else {
        return 0 ;
    }
}

