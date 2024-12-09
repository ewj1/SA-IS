#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

static const char* currentStr; //needs to be a global var for qsort comparison function

int* convertString(char* str, int len){
    int* convertedString = malloc(len*sizeof(int));
    for (int i = 0; i < len; i++){
        convertedString[i] = (int)(str[i]);
    }
    return convertedString;
}

int compareSuffixes(const void* a, const void* b) {
    int idxA = *((int*)a);
    int idxB = *((int*)b);

    return strcmp(currentStr + idxA, currentStr + idxB);
}

int* naiveSA(char* str, int len){
    int* suffixArray = malloc(len*sizeof(int));
    for (int i = 0; i < len; i++) {
        suffixArray[i] = i;
    }
    currentStr = str;
    qsort(suffixArray, len, sizeof(int), compareSuffixes);
    return suffixArray;
}

void printSA(int* SA, int len){
    printf("Suffix Array: ");
    for (int i = 0; i < len; i++) {
        printf("%d ", SA[i]);
    }
    printf("\n");
}


//------------------SA-IS algorithm----------------------------------------

//output is one character longer than length of string (for empty suffix)
char* createTypeMap(int* str, int len){
    char* typemap = (char*)calloc(len, sizeof(char));
    //empty suffix always S-type
    typemap[-1] = 'S';

    //empty string check
    if(len==1){
        return typemap;
    }

    //character before empty suffix is always L-type
    typemap[-2] = 'L';

    //fill out types right to left
    for (int i = len - 3; i > 0; i--){
        if (str[i] > str[i + 1]){
            typemap[i] = 'L';
        }
        else if (str[i] < str[i + 1]){
            typemap[i] = 'S';
        }
        else {
            typemap[i] = typemap[i + 1];
        }
    }
    return typemap;
}

int isLMS(int idx, char* typemap){
    if (idx > 0){
        return typemap[idx] == 'S' && typemap[idx-1] == 'L';
    }
    //first char
    return 0;
}

int areLMSblocksEqual(int* str, int idxA, int idxB, char* typemap){

    int i = 0;
    while(1){
        int aIsLMS = isLMS(idxA+i, typemap);
        int bIsLMS = isLMS(idxB+i, typemap);

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

//output will be calloced to 0 and length of alphabet.
int* findBucketSizes(int* str, int len, int alphabetSize) {
    int* bucketSizes = calloc(alphabetSize, sizeof(int));
    for (int i = 0; i < len; i++) {
        bucketSizes[(int)str[i]]++;
    }
    return bucketSizes;
}

//len of bucketSizes will be alphabetSize
//output is calloced to 0
int* findBucketHeads(int* bucketSizes, int alphabetSize) {
    int* output = calloc(alphabetSize, sizeof(int));
    int idx = 1; //empty suffix at start
    for (int i = 0; i < alphabetSize; i++) {
        output[i] = idx;
        idx += bucketSizes[i];   
    }
    return output;
}

//len of bucketSizes will be alphabetSize
//output is calloced to 0
int* findBucketTails(int* bucketSizes, int alphabetSize) {
    int* output = calloc(alphabetSize, sizeof(int));
    int idx = 1;
    for (int i = 0; i < alphabetSize; i++) {
        idx += bucketSizes[i];
        output[i] = idx - 1;
    }
    return output;
}

//LMSs are LMS suffixes or blocks, sorted or unsorted depending on context of call
int* inducedSort(int* str, int len, int alphabetSize, int* LMSs, int numLMS, int* bucketSizes, char* typemap){
    int* SA = malloc(len * sizeof(int));

    // Place LMS suffixes
    int* bucketTails = findBucketTails(bucketSizes, alphabetSize);
    memset(SA, -1, len * sizeof(int));

    for (int i = 0; i < numLMS, i++;) {
            int c = str[LMSs[i]];
            SA[bucketTails[c]] = i;
            bucketTails[c]--;
    }
    SA[0] = len; //empty suffix at front
    free(bucketTails);

    // Place L-type suffixes
    int* bucketHeads = findBucketHeads(bucketSizes, alphabetSize);

    for (int i = 0; i < len; i++) {
        if (SA[i] > 0 && typemap[SA[i] - 1] == 'L') {
            int c = str[SA[i] - 1];
            SA[bucketHeads[c]] = SA[i] - 1;
            bucketHeads[c]++;
        }
    }
    free(bucketHeads);

    // Place S-type suffixes
    bucketTails = findBucketTails(bucketSizes, alphabetSize);

    for (int i = len - 1; i >= 0; i--) {
        if (SA[i] > 0 && typemap[SA[i] - 1] == 'S') {
            int c = str[SA[i] - 1];
            SA[bucketTails[c]] = SA[i] - 1;
            bucketTails[c]--;
        }
    }
    free(bucketTails);

    return SA;
}

// extract just the LMS block indices
int* extractSortedLMSBlocks(int* SA, int len, int numLMS, char* typemap){
    int* sortedLMSBlocks = malloc(numLMS*sizeof(int));
    int j = 0;
    for (int i = 0; i < len; i++){
        if (isLMS(SA[i], typemap)){
            sortedLMSBlocks[j] = SA[i];
            j++;
        }
    }
    return sortedLMSBlocks;
}

// Takes sorted LMS blocks and assign each block a character
// Reorders these characters to the order the blocks appeared in the original string
int* createShorterString(int* str, int len, int* sortedLMSBlocks, int numLMS, char* typemap, int* numBlockIDs) {
    int blockAssignments[len];
    int blockID = 0; 
    memset(blockAssignments, -1, len*sizeof(int));
    blockAssignments[0] = 0;

    for (int i = 1; i < numLMS; i++){
        if (!areLMSblocksEqual(str, sortedLMSBlocks[i-1], sortedLMSBlocks[i], typemap)){
            blockID++;
        }
        blockAssignments[sortedLMSBlocks[i]] = blockID;
    }
    *numBlockIDs = blockID;

    int* shorterString = malloc(numLMS * sizeof(int));
    int j = 0;
    for (int i = 0; i < len; i++){
        if (blockAssignments[i] > -1){
            shorterString[j] = blockAssignments[i];
            j++;
        }
    }
    return shorterString;
}

// Main SA-IS algorithm to construct the suffix array
int* SAIS(int* str, int len, int alphabetSize, int** output) {

    char* typemap = createTypeMap(str, len);
    int* bucketSizes = findBucketSizes(str, len, alphabetSize);
    
    //find LMS suffixes
    int* LMSSuffixes = calloc(len, sizeof(int));
    int numLMS = 0;
    for (int i = 1; i < len; i++){
        if (isLMS(i, typemap)){
            LMSSuffixes[numLMS] = i;
            numLMS++;
        }
    }
    //preliminary sort to get LMS blocks in sorted order
    int* SA = inducedSort(str, len, alphabetSize, LMSSuffixes, numLMS, bucketSizes, typemap);
    int* sortedLMSBlocks = extractSortedLMSBlocks(SA, len, numLMS, typemap);

    int numBlockIDs;
    int* shorterString = createShorterString(str, len, sortedLMSBlocks, numLMS, typemap, &numBlockIDs);

    int* SAofShorterString = malloc(numLMS * sizeof(int));
    if (numBlockIDs == numLMS-1) { //base case, all unique
        for (int i = 0; i < numLMS; i++) {
            SAofShorterString[shorterString[i]] = i;
        }
    } else {
        SAIS(shorterString, numLMS, numBlockIDs, &SAofShorterString);
    }

    // at this point, SAofShorterString is the sorted order of LMS suffixes
    int sortedLMSSuffixes[numLMS];
    for (int i = 0; i < numLMS; i++){
        sortedLMSSuffixes[i] = LMSSuffixes[SAofShorterString[i]];
    }

    //finally we perform an induced sort to obtain the suffix array of the original string
    int* finalSA = inducedSort(str, len, alphabetSize, sortedLMSSuffixes, numLMS, bucketSizes, typemap);

    // Free allocated memory
    // free(typemap);
    // free(bucketSizes);
    // free(LMSSuffixes);
    // free(SA);
    // free(sortedLMSBlocks);
    // free(shorterString);
    printf("recursion");
    printSA(finalSA, len);
    *output = finalSA;
}


int main() {
    char* str = "banana";
    int len = strlen(str) + 1;
    // int* suffixArray = naiveSA(str, len);
    // printSA(suffixArray, len);
    int* suffixArray = malloc(len*sizeof(int));
    int* convertedString = convertString(str, len);
    printSA(convertedString, len);
    SAIS(convertedString, len, 256, &suffixArray); //assumes 256 alphabet initially
    printSA(suffixArray, len);
    return 0;
}

// // Place sorted LMS suffixes in character buckets
// void inducedSortLMS(char* str, int len, int alphabetSize, int* sortedLMS, int numLMS, int* bucketSizes, char* typemap, int* output){

//     int* bucketTails = (int*)calloc(alphabetSize, sizeof(int));
//     findBucketTails(bucketSizes, alphabetSize, bucketTails);

//     memset(output, -1, len * sizeof(int));

//     for (int i = 0; i < numLMS, i++;) {
//             int c = str[sortedLMS[i]];
//             output[bucketTails[c]] = i;
//             bucketTails[c]--;
//     }
//     output[0] = len; //empty suffix at front
//     free(bucketTails);
// }

// // Place L-type suffixes
// void inducedSortL(char* str, int len, int alphabetSize, int* SA, int* bucketSizes, char* typemap) {
//     int* bucketHeads = (int*)calloc(alphabetSize, sizeof(int));
//     findBucketHeads(bucketSizes, alphabetSize, bucketHeads);

//     for (int i = 0; i < len; i++) {
//         if (SA[i] > 0 && typemap[SA[i] - 1] == 'L') {
//             int c = str[SA[i] - 1];
//             SA[bucketHeads[c]] = SA[i] - 1;
//             bucketHeads[c]++;
//         }
//     }
//     free(bucketHeads);
// }

// // Place S-type suffixes
// void inducedSortS(char* str, int len, int alphabetSize, int* SA, int* bucketSizes, char* typemap) {
//     int* bucketTails = (int*)calloc(alphabetSize, sizeof(int));
//     findBucketTails(bucketSizes, alphabetSize, bucketTails);

//     for (int i = len - 1; i >= 0; i--) {
//         if (SA[i] > 0 && typemap[SA[i] - 1] == 'S') {
//             int c = str[SA[i] - 1];
//             SA[bucketTails[c]] = SA[i] - 1;
//             bucketTails[c]--;
//         }
//     }
//     free(bucketTails);
// }
