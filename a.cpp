#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
using namespace std ;

int main() {
    char buf[1024] ;
    bzero(buf, sizeof(buf)) ;
    int fd = open("tests/1.txt", O_RDWR) ;
    cout << "文件打开完成！"<< endl ;
    int size = read(fd, buf, sizeof(buf)) ;
    cout << "数据: " << endl ; 
    cout<< buf << endl ;
    getchar() ;
    strcpy(buf, "It's a secret!") ;
    write(fd, buf, sizeof(buf)) ;
    getchar() ;
    close(fd) ;
    return 0;
}

