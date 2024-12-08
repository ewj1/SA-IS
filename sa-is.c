#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

const int alphabetSize = 256;

static const char* currentStr;

int compareSuffixes(const void* a, const void* b) {
    int idxA = *((int*)a);
    int idxB = *((int*)b);

    return strcmp(currentStr + idxA, currentStr + idxB);
}

void naiveSA(char* str, int* suffixArray){
    int len = strlen(str) + 1;

    for (int i = 0; i < len; i++) {
        suffixArray[i] = i;
    }
    currentStr = str;
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
void typeMap(char* str, int len, char* output){

    //empty suffix always S-type
    output[-1] = 'S';

    //empty string check
    if(len==0){
        return;
    }

    //character before empty suffix is always L-type
    output[-2] = 'L';

    //fill out types right to left
    for (int i = len - 2; i > 0; i--){
        if (str[i] > str[i + 1]){
            output[i] = 'L';
        }
        else if (str[i] < str[i + 1]){
            output[i] = 'S';
        }
        else {
            output[i] = output[i + 1];
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

int areLMSblocksEqual(char* str, int idxA, int idxB, char* typemap){

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

//alphabetSize will be 256 by default when we call the function
//output will be calloced to 0 for all 256 elements.
void findBucketSizes(char* str, int* output) {
    for (int i = 0; i < strlen(str); i++) {
        output[(int)str[i]]++;
    }
}

//len of bucketSizes will be alphabetSize
//output is calloced to 0
void findBucketHeads(int*bucketSizes, int* output) {
    int idx = 1;
    for (int i = 0; i < alphabetSize; i++) {
        if (bucketSizes[i] > 0) {
            output[i] = idx;
            idx += bucketSizes[i];
        }
        
    }
}

//len of bucketSizes will be alphabetSize
//output is calloced to 0
void findBucketTails(int*bucketSizes, int* output) {
    int idx = 1;
    for (int i = 0; i < alphabetSize; i++) {
        if (bucketSizes[i] > 0) {
            idx += bucketSizes[i];
            output[i] = idx - 1;
            
        }
        
    }
}

// Make a suffix array of LMS-substrings that is almost correct 
// -1 represents locations of suffixes that don't begin with an LMS character
void blockLMSsort(char* str, int* bucketSizes, char* typemap, int* output){
    int len = strlen(str) + 1;
    int* bucketTails = (int*)calloc(alphabetSize, sizeof(int));
    findBucketTails(bucketSizes, bucketTails);

    memset(output, -1, len * sizeof(int));

    for (int i = 0; i < strlen(str), i++;) {
        if (!isLMS(i, typemap)) {
            continue;
        }
        int bucketIndex = (int)str[i];
        output[bucketTails[bucketIndex]] = i;
        bucketTails[bucketIndex]--;
    }
    output[0] = strlen(str);
    free(bucketTails);
}

// Induce L-type suffixes
void induceSortL(char* str, int* SA, int* bucketSizes, char* typemap) {
    int len = strlen(str) + 1;
    int* bucketHeads = (int*)calloc(alphabetSize, sizeof(int));
    findBucketHeads(bucketSizes, bucketHeads);

    for (int i = 0; i < len; i++) {
        if (SA[i] > 0 && typemap[SA[i] - 1] == 'L') {
            int c = str[SA[i] - 1];
            SA[bucketHeads[c]] = SA[i] - 1;
            bucketHeads[c]++;
        }
    }
    free(bucketHeads);
}

// Induce S-type suffixes
void induceSortS(char* str, int* SA, int* bucketSizes, char* typemap) {
    int len = strlen(str) + 1;
    int* bucketTails = (int*)calloc(alphabetSize, sizeof(int));
    findBucketTails(bucketSizes, bucketTails);

    for (int i = len - 1; i >= 0; i--) {
        if (SA[i] > 0 && typemap[SA[i] - 1] == 'S') {
            int c = str[SA[i] - 1];
            SA[bucketTails[c]] = SA[i] - 1;
            bucketTails[c]--;
        }
    }
    free(bucketTails);
}

// Summarize LMS substrings into a smaller problem
void summarizeSuffixArray(char* str, int* SA, char* typemap, int* summaryString, int* summaryOffsets) {
    int len = strlen(str) + 1;
    int summaryIndex = 0;
    int lastLMS = -1;

    memset(summaryString, -1, len * sizeof(int));

    for (int i = 0; i < len; i++) {
        if (isLMS(SA[i], typemap)) {
            if (lastLMS == -1 || !areLMSblocksEqual((char*)str, lastLMS, SA[i], typemap)) {
                summaryIndex++;
            }
            summaryString[SA[i]] = summaryIndex;
            lastLMS = SA[i];
        }
    }

    // Create a dense representation of the summary string
    int offsetIndex = 0;
    for (int i = 0; i < len; i++) {
        if (summaryString[i] >= 0) {
            summaryOffsets[offsetIndex] = i;
            offsetIndex++;
        }
    }
}

// Construct a suffix array for the summary string
void makeSummarySuffixArray(int strlen, int* summaryString, int summaryLen, int* summarySA) {
    if (summaryLen == strlen) {
        memset(summarySA, -1, (summaryLen + 1) * sizeof(int));
        summarySA[0] = summaryLen;
        for (int i = 0; i < summaryLen; i++) {
            int c = summaryString[i];
            summarySA[c + 1] = i;
        }
    } else {
        makeSuffixArrayByInducedSorting((char*)summaryString, summarySA);
    }
}

// Sort LMS substrings accurately using the summary suffix array
void accurateLMSSort(char* str, int* bucketSizes, char* typemap, int* summarySA, int* summaryOffsets, int* SA) {
    int len = strlen(str) + 1;
    int* bucketTails = (int*)calloc(alphabetSize, sizeof(int));
    findBucketTails(bucketSizes, bucketTails);

    memset(SA, -1, len * sizeof(int));
    for (int i = len - 1; i >= 0; i--) {
        if (summarySA[i] > 0) {
            int c = (int)str[summaryOffsets[summarySA[i]]];
            SA[bucketTails[c]] = summaryOffsets[summarySA[i]];
            bucketTails[c]--;
        }
    }
    free(bucketTails);
}

// Main SA-IS algorithm to construct the suffix array
void makeSuffixArrayByInducedSorting(char* str, int* SA) {
    int len = strlen(str) + 1;
    char* typemap = (char*)calloc(len + 1, sizeof(char));
    typeMap(str, len - 1, typemap);

    int* bucketSizes = (int*)calloc(alphabetSize, sizeof(int));
    findBucketSizes(str, bucketSizes);

    // Step 1: Sort LMS substrings
    blockLMSsort(str, bucketSizes, typemap, SA);

    // Step 2: Induce L and S suffixes
    induceSortL(str, SA, bucketSizes, typemap);
    induceSortS(str, SA, bucketSizes, typemap);

    // Step 3: Summarize suffix array
    int* summaryString = (int*)calloc(len, sizeof(int));
    int* summaryOffsets = (int*)calloc(len, sizeof(int));
    summarizeSuffixArray(str, SA, typemap, summaryString, summaryOffsets);

    // Step 4: Calclulate true length of summary suffix array
    int summaryLen = 0;
    for (int i = 0; i < len; i++) {
        if (summaryString[i] != -1) {
            summaryLen++;
        }
    }

    int* summarySA = (int*)calloc(summaryLen, sizeof(int));
    makeSummarySuffixArray(len, summaryString, summaryLen, summarySA);

    // Step 5: Accurately sort LMS substrings
    accurateLMSSort(str, bucketSizes, typemap, summarySA, summaryOffsets, SA);

    // Step 6: Induce final L and S suffixes
    induceSortL(str, SA, bucketSizes, typemap);
    induceSortS(str, SA, bucketSizes, typemap);

    // Free allocated memory
    free(typemap);
    free(bucketSizes);
    free(summaryString);
    free(summaryOffsets);
    free(summarySA);
}


int main() {
    char* str = "banana";
    int len = strlen(str) + 1;
    int suffixArray[len];
    naiveSA(str, suffixArray);
    printSA(suffixArray, len);
    makeSuffixArrayByInducedSorting(str, suffixArray);
    printSA(suffixArray, len);
    return 0;
}

