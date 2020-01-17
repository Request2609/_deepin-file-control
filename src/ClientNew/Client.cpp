#include"Client.h"
//客户端默认接收消息为1的消息
//argv参数包含客户端要连接服务器的ip，端口
//要监控的目录

int ProcessHandle(char info[3][128]) {

    std::shared_ptr<threadPool> pool = std::make_shared<threadPool>(8) ;
    vector<InfoNode> ls ;
    signal(SIGINT, SigHandle) ;
    std::shared_ptr<Fanotify> notify = Fanotify::GetNotify() ;
    int port = atoi(info[1]) ;
    notify->SetIpPort(info[0], port); 
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
            if(ls[i].type == -1&&ls[i].fanFd==ls[i].fileFd) {
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
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    
    struct sockaddr_in addr ;
    addr.sin_family = AF_INET ;
    addr.sin_port = htons(port) ;
    int ret = inet_aton(ip, &addr.sin_addr) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    
    ret = connect(fd, (struct sockaddr*)&addr, sizeof(addr)) ;
    if(ret < 0) {
        printError(__FILE__, __LINE__) ;
        return 0 ;
    }
    return fd ;
}


//向服务端发送请求
    
void SendData(struct InfoNode node, int servFd) {
    
    auto notify = Fanotify::GetNotify() ;
    //文件在监控目录下，并且存在
    if(node.type == 0) {
        //close请求判断是否为最后一次close请求,是最后一次close请求的
        //话,才恢复原来文件内容,否则不会恢复原来文件内容
        int ret =  RecoverRequest(servFd) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return ;
        }
    }

    if(node.type == OPEN) {
        int ret = SendFile(servFd, node.path.c_str()) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return ;
        }
    }
    notify->ModifyServFd(EPOLLIN|EPOLLONESHOT) ;
}

//发送文件内容
int  SendFile(int servFd,const string& name) {
    //判断是否为已经备份过的文件
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = OPEN ;
    int ret = GetMac(data.mac) ;
    if(ret < 0 ) {
        return -1 ;
    }
    strcpy(data.pathName, name.c_str()) ;
    //打开文件
    int fd = open(name.c_str(), O_RDWR) ;
    if(fd < 0) {
        printError(__FILE__, __LINE__) ;
        return -1 ;
    }
    //客户端向服务器至少发送两次消息
    int counts = 0;
    long cur = 0 ; 
    while(1) {
        //读文件内容
        int ret = read(fd, data.buf, sizeof(data.buf)) ;
        if(ret < 0) {
            printError(__FILE__, __LINE__) ;
            return -1 ;
        }

        //如果将文件读完了
        if(ret ==0&& counts) {
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
            close(fd) ;
            break ;
        }
        //设置偏移量
        data.left = cur ;
        cur += ret ;
        data.right = cur ;
        //向服务器发送文件内容
        int res =  writen(servFd, &data, sizeof(data)) ;
        if(res < 0) {
            printError(__FILE__, __LINE__) ;
            return  -1 ;
        }
    }
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
            cout << "客户端与服务器断开连接" << endl ;
            return 1 ;
        }
        //要是第一次open请求或者close请求，打开文件
        //其他情况下只写文件
        struct ActiveNode* node ;
        struct fanotify_event_metadata metadata ;
        switch(data.type) {
        //表明服务器备份文件完成了
        case OPEN :
            //用户可以访问文件了
            //根据路径获取fanotifyFd
            node = Fanotify::GetHandleByPath(data.pathName) ;
            if(node == NULL) {
                cout << "在列表中没找到 "<< "    " <<__LINE__ << "  " << __FILE__ << endl ;
            }
            //备份的文件的未关闭的文件描述符
            metadata.fd = node->fileFd ;
            notify->HandlePerm(node->fanFd, &metadata) ;
            notify->DetectEvent(node->fanFd, FAN_CLOSE) ;
            close(metadata.fd) ;
            node->fileFd = -1 ;
            notify->ModifyServFd(EPOLLIN|EPOLLONESHOT) ;
            return 0;
        case CLOSE :
            //文件完成
            struct ActiveNode* anode = Fanotify::GetHandleByPath(data.pathName) ;
            if(anode == NULL|anode->fileFd < 0) {
                cout << __LINE__ << "   " << __FILE__ << endl ;
            }
            res = RecoverFile(data, anode->fileFd) ;
            if(res == 1) {
                close(anode->fileFd) ;
            }
            Fanotify::Remove(anode->fanFd) ;
            anode->fileFd = -1 ;
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
int RecoverRequest(int servFd) {
    
    Data data ;
    memset(&data, 0, sizeof(data)) ;
    data.type = CLOSE ;
    int ret = GetMac(data.mac) ;
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
        close(fd) ;
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

//接收到异常信号
void SigHandle(int signo) {

    if(signo == SIGINT) {
        close(FreeInfo::servFd) ;
        msgctl(FreeInfo::msgId, IPC_RMID, 0) ;
        return ;
    }
}

