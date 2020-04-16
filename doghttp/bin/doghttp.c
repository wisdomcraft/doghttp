#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "myfunction.c"
#include "initialization.c"


/*
* ----------------------------------------------------------------
* get config from conf cache files
* ----------------------------------------------------------------
*/

int conf_get_listen_port(){
    FILE *fstream;
    char *conf = "../temp_conf/listen";
    
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
        
        int listen = atoi(fileContent);
        return listen;
    }
}


/*
* ----------------------------------------------------------------
* request header and response data 
* ----------------------------------------------------------------
*/

//get http type (form method)
char *get_method(char *request){
    if(request[0]=='G' && request[1]=='E' && request[2]=='T'){
        return "GET";
    }else if(request[0]=='P' && request[1]=='O' && request[2]=='S' && request[3]=='T'){
        return "POST";
    }else{
        return "";
    }
}

//http type (form method) check
int method_check(char *request){
    char *method = get_method(request);
    if(strcmp(method,"GET")==0 || strcmp(method,"POST")==0){
        return 0;
    }else{
        return -1;
    }
}

//get host
char *request_get_host(char *request){
    int requestLength = strlen(request);
    int i;
    int j=-1;
    for(i=0;i<requestLength;i++){
        if(request[i] == '\n'){
            if(request[i+1]=='H'&&request[i+2]=='o'&&request[i+3]=='s'&&request[i+4]=='t'){
                j = i+6;
            }
        }
    }
    
    while(1){
        if(request[j]==' '){
            j++;
        }else{
            break;
        }
    }
    
    if(j<0){
        return "";
    }
    
    int hostLength = 0;
    int hostLengthCount = j;
    while(request[hostLengthCount] !='\n' && request[hostLengthCount] !=':' && request[hostLengthCount] !='\0'){
        hostLength++;
        hostLengthCount++;
    }
    
    if(hostLength < 1){
        return "";
    }
    
    char host[hostLength];
    int s = 0;
    while(request[j] !='\n' && request[j] !=':' && request[j] !='\0'){
        host[s] = request[j];
        s++;
        j++;
    }
    host[s] = '\0';
    
    char *host_address = (char*)malloc(strlen(host)+1);
    strcpy(host_address,host);
    return host_address;
}

//host check
int host_check(char *request){
    char *host = request_get_host(request);
    if(strlen(host) < 1){
        return -1;
    }
    
    char *conf_host_directory = (char*)malloc(strlen("../temp_conf/")+strlen(host)+1);
    strcpy(conf_host_directory, "../temp_conf/");
    strcat(conf_host_directory, host);
    
    if(access(conf_host_directory, 0) == 0){
        return 0;
    }else{
        return -1;
    }
}

char *get_root(char *request){
    char *host = request_get_host(request);
    char *rootFile = (char*)malloc(strlen("../temp_conf/")+strlen(host)+6);
    strcpy(rootFile,"../temp_conf/");
    strcat(rootFile,host);
    strcat(rootFile,"/root");
    
    FILE *fstream;
    
    fstream = fopen(rootFile,"r");
    if(fstream == NULL){
        return "";
    }else{
        int fileLength = 0;
        fseek(fstream, 0, SEEK_END);
        fileLength = ftell(fstream);
        rewind(fstream);

        char fileContent[fileLength+1];
        fread(fileContent, 1, fileLength, fstream);
        fileContent[fileLength+1] = '\0';
        
        fclose(fstream);
        
        char rootValue[fileLength+1];
        int i;
        for(i=0;i<strlen(fileContent);i++){
            if(fileContent[i] != '\n'){
                rootValue[i] = fileContent[i];
            }else{
                break;
            }
        }
        rootValue[i] = '\0';
        
        char *root = (char*)malloc(strlen(rootValue));
        strcpy(root,rootValue);
        return root;
    }
}


//get url
char *request_get_url(char *request){
    char *method = get_method(request);
    int i;
    if(strcmp(method,"GET") == 0){
        i = 4;
    }else if(strcmp(method,"POST") == 0){
        i = 5;
    }else{
        return "";
    }
    
    char url[200] = {0};
    int j=0;
    while(request[i] != ' '){
        url[j] = request[i];
        j++;
        i++;
    }
    char *result = (char*)malloc(strlen(url)+1);
    strcpy(result,url);
    return result;
}


char *get_defaultindex(char *host, char *root){
    char *path = (char*)malloc(strlen("../temp_conf/")+strlen(host)+10);
    strcpy(path,"../temp_conf/");
    strcat(path,host);
    strcat(path,"/index");
    
    int indexTotal = 0;
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(path);
    while((dp = readdir(dirp)) != NULL){
        indexTotal++;
    }
    indexTotal = indexTotal-2;
    
    int i;
    int error = 0;
    char *defaultIndex;
    for(i=1;i<=indexTotal;i++){
        char indexFileName[2];
        sprintf(indexFileName,"%d",i);
        
        char *indexFilePath = (char*)malloc(strlen(path)+3);
        strcpy(indexFilePath,path);
        strcat(indexFilePath,"/");
        strcat(indexFilePath,indexFileName);
        
        FILE *fstream;
        fstream = fopen(indexFilePath,"r");
        if(fstream == NULL){
            if(i == indexTotal){
                error = -1;
                break;
            }
            continue;
        }else{
            int fileLength = 0;
            fseek(fstream, 0, SEEK_END);
            fileLength = ftell(fstream);
            rewind(fstream);

            char fileContent[fileLength];
            fread(fileContent, 1, fileLength, fstream);
            fileContent[fileLength] = '\0';
            
            fclose(fstream);
            
            char indexFile[fileLength+1];
            int s=0;
            for(s=0;s<strlen(fileContent);s++){
                if(fileContent[s] != '\n'){
                    indexFile[s] = fileContent[s];
                }else{
                    break;
                }
            }
            indexFile[s] = '\0';
            
            char *url = (char*)malloc(strlen(root)+strlen(indexFile)+2);
            strcpy(url,root);
            strcat(url,"/");
            strcat(url,indexFile);
            
            FILE *furlstream;
            furlstream = fopen(url,"r");
            if(furlstream == NULL){
                if(i == indexTotal){
                    error = -1;
                    break;
                }
                continue;
            }else{
                char *defaultIndexName = (char*)malloc(strlen(indexFile)+1);
                strcpy(defaultIndexName,indexFile);
                defaultIndex = defaultIndexName;
                break;
            }
        }
    }
    
    if(error == -1) return "";
    
    return defaultIndex;
}


char *get_response(char *request){
    if(method_check(request) == -1){
        return "HTTP/1.1 400 Bad Request\n\n400 Bad Request";
    }
    
    if(host_check(request) == -1){
        return "HTTP/1.1 400 Bad Request\n\n400 Bad Request";
    }
    
    char *url = request_get_url(request);
    if(strlen(url) < 1){
        return "HTTP/1.1 400 Bad Request\n\n400 Bad Request";
    }
    
    char *root = get_root(request);
    if(strlen(root) < 1){
        return "HTTP/1.1 500 Internal Server Error\n\n500 Internal Server Error";
    }
    
    char *path = (char*)malloc(strlen(root)+strlen(url)+999);
    strcpy(path,root);
    if(strcmp(url,"/")==0){
        char *host = request_get_host(request);
        char *index = get_defaultindex(host,root);
        if(strlen(index)<1){
            return "HTTP/1.1 404 Not Found\n\n404 Not Found";
        }
        
        strcat(path,"/");
        strcat(path,index);
    }else{
        strcat(path,url);
    }
    
    FILE *fstream;
    fstream = fopen(path,"r");
    
    if(fstream == NULL){
        return "HTTP/1.1 404 Not Found\n\n404 Not Found";
    }else{
        int fileLength = 0;
        fseek(fstream, 0, SEEK_END);
        fileLength = ftell(fstream);
        rewind(fstream);

        char fileContent[fileLength];
        fread(fileContent, 1, fileLength, fstream);
        fileContent[fileLength] = '\0';
        
        fclose(fstream);
    
        char *response = (char*)malloc(strlen("HTTP/1.1 200 OK\n\n")+strlen(fileContent)+1);
        strcpy(response,"HTTP/1.1 200 OK\n\n");
        strcat(response,fileContent);
        
        return response;
    }
}

//----------------------------------------------------------------


//main function
int main(void){
    //initialization and create temp files
    if(initialization() != 0){
        printf("initialization failed\n");
        return -1;
    }
    //return -1;  //it is for test
    
    //create socket
    int sServer;
    if((sServer = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("socket error!\n");
        return 0;
    }
    
    //bind ip and port
    int port = conf_get_listen_port();
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
    while(1){
        sClient = accept(sServer, (struct sockaddr*)&remoteAddr, &nAddrlen);
        if(sClient == -1){
            printf("receive error!\n");
            continue;
        }
        
        //receive data
        int receiveMaxLength = 99999;
        char receiveData[receiveMaxLength+1];
        int receiveSingleLength;
        
        char *receive = (char*)malloc(1);
        strcpy(receive,"");
        int realloc_error;
    
        while((receiveSingleLength=recv(sClient,receiveData,receiveMaxLength,0)) > 0){
            receiveData[receiveSingleLength] = 0x00;
            char *realloc_result = realloc(receive,strlen(receiveData)+strlen(receive));
            if(realloc_result == NULL){
                realloc_error = 1;
                break;
            }
            strcat(receive,receiveData);
            if(receiveSingleLength < receiveMaxLength){
                break;
            }else{
                memset(receiveData,0,sizeof(receiveData));
            }
        }
        
        //get response and send data
        if(realloc_error == 1){
            char *response = "HTTP/1.1 500 Internal Server Error\n\n500 Internal Server Error";
            
            char *sendData = response;
            send(sClient, sendData, strlen(sendData), 0);
        }else{
            //analyse request header from browser
            int receiveDataLength = strlen(receiveData);
            char revData[receiveDataLength];
            strcpy(revData,receiveData);
            char *response = get_response(revData);
            
            char *sendData = response;
            send(sClient, sendData, strlen(sendData), 0);
        }
        
        close(sClient);
    }

    close(sServer);
    return 0;
}





