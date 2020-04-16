#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>


//----------------------------------------------------------------
//get config from conf
int get_listen_port(){
    FILE *fstream;
    char *conf = "./http.conf";
    
    fstream = fopen(conf,"r");
    if(fstream == NULL){
        return 80;
    }else{
        int fileLength = 0;
        fseek(fstream, 0, SEEK_END);
        fileLength = ftell(fstream);
        rewind(fstream);

        char fileContent[fileLength];
        fread(fileContent, 1, fileLength, fstream);
        fileContent[fileLength] = '\0';
        
        fclose(fstream);
        
        int i;
        int j=-1;
        for(i=0;i<fileLength;i++){
            if(fileContent[i] == '\n'){
                if(fileContent[i+1]=='l'&&fileContent[i+2]=='i'&&fileContent[i+3]=='s'&&fileContent[i+4]=='t'&&fileContent[i+5]=='e'&&fileContent[i+6]=='n'){
                    j=i+8;
                    break;
                }
            }
        }
        
        if(j<0){
            return 80;
        }else{
            while(1){
                if(fileContent[j]==' '){
                    j++;
                }else{
                    break;
                }
            }
            
            char port[6];
            int s=0;
            while(fileContent[j]>='0' && fileContent[j]<='9'){
                port[s] = fileContent[j];
                s++;
                j++;
            }
            
            if(s == 0){
                return 80;
            }else{
                port[s] = '\0';
                int listen = atoi(port);
                return listen;
            }
        }
    }
}


//----------------------------------------------------------------


//----------------------------------------------------------------
//request header and response data 
char *get_response(char *request){
    //create socket
    int sClient;
    sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sClient == -1){
        printf("socket error!\n");
        close(sClient);
        return "HTTP/1.1 500 Internal Server Error\n\n500 Internal Server Error";
    }
    
    //bind ip and port
    struct sockaddr_in serAddr;
    serAddr.sin_family      = AF_INET;
    serAddr.sin_port        = htons(88);
    serAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(sClient, (struct sockaddr*)&serAddr, sizeof(struct sockaddr)) == -1){
        printf("connect error!\n");
        close(sClient);
        return "HTTP/1.1 500 Internal Server Error\n\n500 Internal Server Error";
    }
    
    //send data
    send(sClient, request, strlen(request), 0);
    
    //receive data
    int length = 999;
    char receiveData[length];
    int revVal;
    
    char *receive = (char*)malloc(1);
    strcpy(receive,"");
    int realloc_error;
    while((revVal = recv(sClient, receiveData, length, 0)) > 0){
        receiveData[revVal] = 0x00;
        char *realloc_result = realloc(receive,strlen(receiveData)+strlen(receive));
        if(realloc_result == NULL){
            realloc_error = 1;
            break;
        }
        strcat(receive,receiveData);
        if(revVal < length){
            break;
        }else{
            memset(receiveData,0,sizeof(receiveData));
        }
    }
    if(realloc_error == 1){
        return "HTTP/1.1 500 Internal Server Error\n\n500 Internal Server Error";
    }
    
    return receive;
}

//----------------------------------------------------------------


//main function
int main(void){
    //create socket
    int sServer;
    if((sServer = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket error!\n");
        return 0;
    }
    
    //bind ip and port
    int port = get_listen_port();
    struct sockaddr_in sin;
    sin.sin_family      = AF_INET;
    sin.sin_port        = htons(port);
    sin.sin_addr.s_addr = INADDR_ANY;
    if(bind(sServer, (struct sockaddr*)&sin, sizeof(struct sockaddr)) == -1){
        printf("bind error!\n");
        close(sServer);
        return 0;
    }
    
    //strart listen
    if(listen(sServer, 5) == -1){
        printf("listen error\n");
        close(sServer);
        return 0;
    }
    
    //while to receive data
    int sClient;
    struct sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    char revData[9999];
    while(1){
        sClient = accept(sServer, (struct sockaddr*)&remoteAddr, &nAddrlen);
        if(sClient == -1){
            printf("receive error!\n");
            continue;
        }
        
        //receive data
        int ret = recv(sClient, revData, 9999, 0);
        if(ret > 0){
            revData[ret] = 0x00;
        }
        
        //analyse request header from browser
        char *response = get_response(revData);

        //send data
        char *sendData = response;
        send(sClient, sendData, strlen(sendData), 0);
        close(sClient);
    }

    close(sServer);
    return 0;
}





