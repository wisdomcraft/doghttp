
/*
* ----------------------------------------------------------------
* initialization and create temp files
* ----------------------------------------------------------------
*/


int init_filename_directory_check(char *filename){
    struct stat buf;
    stat(filename,&buf);
    if(S_ISDIR(buf.st_mode)){
        return 0;
    }else{
        return -1;
    }
}


int init_delete_directory(char *dirname){
    if(access(dirname, 0) == -1){
        return 0;
    }
    
    DIR *dirp;
    dirp = opendir(dirname);
    if(dirp == NULL){
        printf("open the directory failed\n");
        return -1;
    }
    
    struct dirent *dp;
    while((dp = readdir(dirp)) != NULL){
        if(strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0){
        
            int length = strlen(dirname)+strlen(dp->d_name)+2;
            char filename[length];
            strcpy(filename,dirname);
            strcat(filename,"/");
            strcat(filename,dp->d_name);
            
            if(init_filename_directory_check(filename) == 0){
                if(init_delete_directory(filename)!=0){
                    printf("one\n");
                    return -1;
                }
            }else{
                if(remove(filename) != 0){
                    printf("two\n");
                    return -1;
                }
            }
        }
    }
    closedir(dirp);
    
    remove(dirname);
    
    return 0;
}


int init_write_listen(void){
    FILE *fstream;
    char *conf = "../conf/http.conf";
    
    fstream = fopen(conf,"r");
    if(fstream == NULL){
        printf("open http.conf failed, even this file maybe is not exist\n");
        return -1;
    }

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
                j=i+7;
                break;
            }
        }
    }
    
    char port[7];
    
    if(j<0){
        strcpy(port,"80");
    }else{
        while(1){
            if(fileContent[j]==' '){
                j++;
            }else{
                break;
            }
        }
        
        int s=0;
        while(fileContent[j]>='0' && fileContent[j]<='9'){
            port[s] = fileContent[j];
            s++;
            j++;
        }
        
        if(s > 6){
            printf("error, listen port from http.conf, its max length must be less than or equal to 6\n");
            return -1;
        }
        
        if(s == 0){
            strcpy(port,"80");
        }else{
            port[s] = '\0';
        }
    }
    
    int listen = atoi(port);
    if(listen > 65535){
        printf("the tcp max port is 65535, your listen port cannot be greater than it\n");
        return -1;
    }else if(listen < 1){
        printf("the tcp min port is 1, your listen port cannot be zero and less than 1\n");
        return -1;
    }
    
    FILE *fileListen;
    fileListen = fopen("../temp_conf/listen","w");
    if(fileListen == NULL){
        printf("create or open \"../temp_conf/listen\" failed\n");
        return -1;
    }
    int writeResult = fwrite(port,1,strlen(port),fileListen);
    fclose(fileListen);
    
    if(writeResult < 1){
        printf("write listen port into \"../temp_conf/listem\" failed\n");
        return -1;
    }
    
    return 0;
}


int init_write_virtualhost_root(char *virtualhost, char *servername){
    int i = my_strpos(virtualhost,"root");
    if(i == -1){
        printf("error about root, there is not \"root\" word in http.conf\n");
        return -1;
    }
    
    int j = i+5;
    
    while(1){
        if(virtualhost[j]==' '){
            j++;
        }else{
            break;
        }
    }
    
    int k = j;
    int length = 1;
    while(1){
        if(virtualhost[k] != ';' && virtualhost[k] != '\n' && virtualhost[k] != ' '){
            k++;
            length++;
        }else{
            break;
        }
    }
    
    if(length == 1){
        printf("there is not root value in http.conf about server name \"%s\"\n", servername);
        return -1;
    }
    
    char root[length+1];
    int s;
    for(s=0;s<length;s++){
        if(virtualhost[j] != ';' && virtualhost[j] != '\n' && virtualhost[k] != ' '){
            root[s] = virtualhost[j];
            j++;
        }else{
            break;
        }
    }
    root[s] = '\0';
    
    char *rootPath = (char*)malloc(strlen("../temp_conf/")+strlen(servername)+6);
    strcpy(rootPath,"../temp_conf/");
    strcat(rootPath,servername);
    strcat(rootPath,"/root");
    
    FILE *fileRoot;
    fileRoot = fopen(rootPath,"w");
    if(fileRoot == NULL){
        printf("create or open \"%s\" failed\n", rootPath);
        return -1;
    }
    int writeResult = fwrite(root,1,strlen(root),fileRoot);
    fclose(fileRoot);
    
    if(writeResult < 1){
        printf("write root value into \"%s\" failed\n", rootPath);
        return -1;
    }
    
    return 0;
}


int init_write_virtualhost_index_do(char *servername, char *index, int sort){
    if(sort >= 10){
        printf("number of default index cannot more then ten\n");
        return -1;
    }
    char number[2];
    sprintf(number,"%d",sort);
    
    char *indexFilePath = (char*)malloc(strlen("../temp_conf/")+strlen(servername)+strlen(number)+8);
    strcpy(indexFilePath,"../temp_conf/");
    strcat(indexFilePath,servername);
    strcat(indexFilePath,"/index/");
    strcat(indexFilePath,number);
    
    FILE *fileIndex;
    fileIndex = fopen(indexFilePath,"w");
    if(fileIndex == NULL){
        printf("create or open \"%s\" failed\n", indexFilePath);
        return -1;
    }
    int writeResult = fwrite(index,1,strlen(index),fileIndex);
    fclose(fileIndex);
    
    if(writeResult < 1){
        printf("write index value into \"%s\" failed\n", indexFilePath);
        return -1;
    }
    
    return 0;
}

int init_write_virtualhost_index(char *virtualhost, char *servername){
    char *indexPath = (char*)malloc(strlen("../temp_conf/")+strlen(servername)+7);
    strcpy(indexPath,"../temp_conf/");
    strcat(indexPath,servername);
    strcat(indexPath,"/index");
    
    if(mkdir(indexPath,0700) != 0){
        printf("create path \"%s\" failed\n", indexPath);
        return -1;
    }
    
    int i = my_strpos(virtualhost,"index");
    int j = i + 6;
    
    while(1){
        if(virtualhost[j] != ' '){
            break;
        }else{
            j++;
        }
    }
    
    int k = j;
    int length = 1;
    while(1){
        if(virtualhost[k] != ';' && virtualhost[k] != '\n'){
            k++;
            length++;
        }else{
            break;
        }
    }
    
    if(length == 1){
        printf("there is not root value in http.conf\n");
        return -1;
    }
    
    char indexCharArray[length+1];
    int s;
    for(s=0;s<length;s++){
        if(virtualhost[j] != ';' && virtualhost[j] != '\n'){
            indexCharArray[s] = virtualhost[j];
            j++;
        }else{
            break;
        }
    }
    indexCharArray[s] = '\0';
    
    int     x;
    char    index[length+1];
    int     y = 0;
    int     indexCharArrayLength = strlen(indexCharArray);
    int     error = 0;
    int     sort = 0;
    for(x=0;x<indexCharArrayLength;x++){
        if(indexCharArray[x] != ' ' && indexCharArray[x] != '\0'){
            index[y] = indexCharArray[x];
            y++;
            if(x == (indexCharArrayLength-1)){
                sort++;
                index[y] = '\0';
                error = init_write_virtualhost_index_do(servername, index, sort);
                if(error == -1){
                    break;
                }
            }
        }else{
            sort++;
            index[y] = '\0';
            error = init_write_virtualhost_index_do(servername, index, sort);
            if(error == -1){
                break;
            }
            
            memset(index,0,sizeof(index));
            y=0;
        }
    }
    
    if(error == -1) return -1;
    
    return 0;
}


int init_write_virtualhost_servername(char *virtualhost, char *servername){
    char *servernamePath = (char*)malloc(strlen("../temp_conf/")+strlen(servername)+1);
    strcpy(servernamePath,"../temp_conf/");
    strcat(servernamePath,servername);
    
    if(access(servernamePath, 0) == 0){
        printf("servername \"%s\" is repeated in http.conf\n", servername);
        return -1;
    }
    
    if(mkdir(servernamePath,0700) != 0){
        printf("create path \"%s\" failed\n", servernamePath);
        return -1;
    }
    
    if(init_write_virtualhost_root(virtualhost, servername) == -1){
        return -1;
    }
    
    init_write_virtualhost_index(virtualhost, servername);
    
    return 0;
}


int init_write_virtualhost_do(char *virtualhost){
    int i = my_strpos(virtualhost,"server_name");
    if(i == -1){
        printf("error about server_name, there is not \"server_name\" word in http.conf\n");
        return -1;
    }
    
    int j;
    j = i+12;

    while(1){
        if(virtualhost[j] != ' '){
            break;
        }else{
            j++;
        }
    }
    
    int k = j;
    int length = 1;
    while(1){
        if(virtualhost[k] != ';' && virtualhost[k] != '\n'){
            k++;
            length++;
        }else{
            break;
        }
    }
    
    if(length == 1){
        printf("server_name value of http.conf cannot be empty\n");
        return -1;
    }
    
    char serverNameCharArray[length+1];
    int s;
    for(s=0;s<length;s++){
        if(virtualhost[j] != ';' && virtualhost[j] != '\n'){
            serverNameCharArray[s] = virtualhost[j];
            j++;
        }else{
            break;
        }
    }
    serverNameCharArray[s] = '\0';
    
    int x;
    char serverName[length+1];
    int y=0;
    int serverNameCharArrayLength = strlen(serverNameCharArray);
    int error = 0;
    for(x=0;x<serverNameCharArrayLength;x++){
        if(serverNameCharArray[x] != ' ' && serverNameCharArray[x] != '\0'){
            serverName[y] = serverNameCharArray[x];
            y++;
            if(x == (serverNameCharArrayLength-1)){
                serverName[y] = '\0';
                error = init_write_virtualhost_servername(virtualhost,serverName);
                if(error == -1) break;
            }
        }else{
            serverName[y] = '\0';
            error = init_write_virtualhost_servername(virtualhost,serverName);
            if(error == -1) break;
            
            memset(serverName,0,sizeof(serverName));
            y=0;
        }
    }
    
    if(error == -1){
        return -1;
    }
    
    return 0;
}


int init_write_virtualhost(void){
    FILE *fstream;
    char *conf = "../conf/http.conf";
    
    fstream = fopen(conf,"r");
    if(fstream == NULL){
        printf("open http.conf failed, even this file maybe is not exist\n");
        return -1;
    }

    int fileLength = 0;
    fseek(fstream, 0, SEEK_END);
    fileLength = ftell(fstream);
    rewind(fstream);

    char fileContent[fileLength];
    fread(fileContent, 1, fileLength, fstream);
    fileContent[fileLength] = '\0';
    
    fclose(fstream);
    
    int i;
    int errorCode = 0;
    for(i=0;i<strlen(fileContent);i++){
        if(fileContent[i]=='s' && fileContent[i+1]=='e' && fileContent[i+2]=='r' && fileContent[i+3]=='v' && fileContent[i+4]=='e' && fileContent[i+5]=='r' && fileContent[i+6]=='{'){
            int j;
            int s;
            for(j=i+7;j<strlen(fileContent);j++){
                if(fileContent[j] == '}'){
                    int length = (j-i)-5;
                    char virtualhost[length];
                    int k=0;
                    for(s=i+7;s<j;s++){
                        virtualhost[k] = fileContent[s];
                        k++;
                    }
                    virtualhost[k] = '\0';
                    
                    errorCode = init_write_virtualhost_do(virtualhost);
                    if(errorCode == -1) break;
                    
                    j = strlen(fileContent);
                }
            }
        }
        if(errorCode == -1) break;
    }
    
    if(errorCode == -1) return -1;
    
    return 0;
}


int initialization(void){
    char *temp_conf = "../temp_conf";
    if(init_delete_directory(temp_conf) != 0){
        printf("delete directory temp_conf failed\n");
        return -1;
    }
    
    if(mkdir(temp_conf,0700) != 0){
        printf("create directory temp_conf failed\n");
        return -1;
    }
    
    if(init_write_listen() != 0){
        printf("write conf temp file of listen port failed\n");
        return -1;
    }
    
    if(init_write_virtualhost() != 0){
        printf("write VirtualHost temp conf file failed\n");
        return -1;
    }
    
    return 0;
}


