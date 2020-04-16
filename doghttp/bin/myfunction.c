#include <string.h>


/*
* ----------------------------------------------------------------
* my custom funtions
* ----------------------------------------------------------------
*/


//find and return the position of a string first appear in another string
int my_strpos(char *string, char *find){
    int stringLength = strlen(string);
    char haystack[stringLength];
    strcpy(haystack, string);
    
    int findLength = strlen(find);
    char needle[findLength];
    strcpy(needle, find);
    
    int i;
    int j;
    int pos;
    for(i=0;i<stringLength;i++){
        if(haystack[i] == needle[0]){
            for(j=0;j<findLength;j++){
                if(haystack[i+j] != needle[j]){
                    break;
                }
            }
            if(j == findLength){
                break;
            }
        }
    }
    
    if(i != stringLength){
        pos = i;
    }else{
        pos = -1;
    }
    
    return pos;
}



