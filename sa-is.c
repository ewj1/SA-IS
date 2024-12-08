#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

//global input string
const char* str;
int compareSuffixes(const void *a, const void *b) {
    int idxA = *((int*)a);
    int idxB = *((int*)b);

    return strcmp(str + idxA, str + idxB);
}

void naiveSA(const char* str, int* suffixArray){
    int len = strlen(str);

    for (int i = 0; i < len; i++) {
        suffixArray[i] = i;
    }

    qsort(suffixArray, len, sizeof(int), compareSuffixes);
}

void printSA(int* SA, int len){
    printf("Suffix Array: ");
    for (int i = 0; i < len; i++) {
        printf("%d ", SA[i]);
    }
    printf("\n");
}


//SA-IS algorithm

//output is one character longer than length of string (for empty suffix)
void typeMap(const char* str, int len, char* output){

    //empty suffix always S-type
    output[-1] = 'S';

    //empty string check
    if(len==0){
        return;
    }

    //character before empty suffix is always L-type
    output[-2] = 'L';

    //fill out types right to left
    for (int i = len-2; i>0; i--){
        if (str[i] > str[i+1]){
            output[i] = 'L';
        }
        else if (str[i] < str[i+1]){
            output[i] = 'S';
        }
        else {
            output[i] = output[i+1];
        }
    }
}

int isLMS(int idx, char* typemap){
    if (idx > 0){
        return typemap[idx] == 'S' && typemap[idx-1] == 'L';
    }
    //first char
    return 0;
}

int areLMSblocksEqual(char* str, int len, int idxA, int idxB, char* typemap){

    int i = 0;
    while(1){
        int aIsLMS = isLMS(idxA, typemap);
        int bIsLMS = isLMS(idxB, typemap);

        if (i > 0 && aIsLMS && bIsLMS){
            return 1;
        }
        else if (aIsLMS != bIsLMS){
            return 0;
        }
        else if (str[idxA+i] != str[idxB+i]){
            return 0;
        }

        i++;
    }
}



void blockLMSsort(char* str, int len, int* bucketSizes, char* typemap, int* output){
    int outputLen = len+1;
    for (int i = 0; i < outputLen; i++){
        output[i] = -1;
    }

    int* bucketTails = 
}


int main() {
    str = "banana";
    int len = strlen(str);
    int suffixArray[len];
    naiveSA(str, suffixArray);
    printSA(suffixArray, len);
    return 0;
}

