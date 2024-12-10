#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static const char* currentStr; //needs to be a global var for qsort comparison function

//convert file text to string
char* read_file(FILE* file) {
    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* str = malloc(sizeof(char)*(len + 1));
    char c;
    int i = 0;
    while ( (c = fgetc(file)) != EOF) {
        str[i] = c;
        i++;
    }
    str[i] = '\0';
    return str;

}

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
    currentStr = str; //global var for compareSuffixes
    qsort(suffixArray, len, sizeof(int), compareSuffixes);
    return suffixArray;
}

void printSA(int* SA, int len){
    printf("Array: ");
    for (int i = 0; i < len; i++) {
        printf("%d ", SA[i]);
    }
    printf("\n");
}

void printTypemap(char* typemap, int len){
    printf("Typemap: ");
    for (int i = 0; i < len; i++) {
        printf("%c ", typemap[i]);
    }
    printf("\n");
}
//------------------SA-IS algorithm----------------------------------------

//output is one character longer than length of string (for empty suffix)
char* createTypeMap(int* str, int len){
    char* typemap = malloc(len*sizeof(char));
    //empty suffix always S-type
    typemap[len-1] = 'S';

    //empty string check
    if(len==1){
        return typemap;
    }

    //character before empty suffix is always L-type
    typemap[len-2] = 'L';

    //fill out types right to left
    for (int i = len - 3; i >= 0; i--){
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
    int idx = 0;
    for (int i = 0; i < alphabetSize; i++) {
        if(bucketSizes[i] > 0){
            output[i] = idx;
            idx += bucketSizes[i];
        }
    }
    return output;
}

//len of bucketSizes will be alphabetSize
//output is calloced to 0
int* findBucketTails(int* bucketSizes, int alphabetSize) {
    int* output = calloc(alphabetSize, sizeof(int));
    int idx = 0;
    for (int i = 0; i < alphabetSize; i++) {
        if(bucketSizes[i] > 0){
            idx += bucketSizes[i];
            output[i] = idx - 1;
        }
    }
    return output;
}

//LMSs are LMS suffixes or blocks, sorted or unsorted depending on context of call
int* inducedSort(int* str, int len, int alphabetSize, int* LMSs, int numLMS, int* bucketSizes, char* typemap){
    int* SA = malloc(len * sizeof(int));

    // Place LMS suffixes
    int* bucketTails = findBucketTails(bucketSizes, alphabetSize);
    memset(SA, -1, len * sizeof(int));
    // printf("numLMS: %d \n", numLMS);
    for (int i = numLMS-1; i >= 0; i--) {
            int c = str[LMSs[i]];
            // printf("c: %d \n", c);
            SA[bucketTails[c]] = LMSs[i];
            bucketTails[c]--;
    }
    SA[0] = len-1; //empty suffix at front
    free(bucketTails);

    // Place L-type suffixes
    int* bucketHeads = findBucketHeads(bucketSizes, alphabetSize);
    // printf("bucket heads:");
    // printSA(bucketHeads, alphabetSize);
    for (int i = 0; i < len; i++) {
        if (SA[i] > 0 && typemap[SA[i] - 1] == 'L') {
            int prevChar = str[SA[i] - 1];
            SA[bucketHeads[prevChar]] = SA[i] - 1;
            bucketHeads[prevChar]++;
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
    int* blockAssignments = malloc(len*sizeof(int));
    int blockID = 0; 
    memset(blockAssignments, -1, len*sizeof(int));
    blockAssignments[len-1] = 0;

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
    free(blockAssignments);
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
        SAIS(shorterString, numLMS, numLMS, &SAofShorterString); //note!! the alphabetsize is also numLMS so that bucketHeads/Tails indexes properly
    }

    // at this point, SAofShorterString is the sorted order of LMS suffixes
    int* sortedLMSSuffixes = malloc(numLMS * sizeof(int));
    for (int i = 0; i < numLMS; i++){
        sortedLMSSuffixes[i] = LMSSuffixes[SAofShorterString[i]];
    }

    //finally we perform an induced sort to obtain the suffix array of the original string
    int* finalSA = inducedSort(str, len, alphabetSize, sortedLMSSuffixes, numLMS, bucketSizes, typemap);

    free(SA);
    free(typemap);
    free(sortedLMSBlocks);
    free(bucketSizes);
    free(LMSSuffixes);
    free(shorterString);
    free(SAofShorterString);
    free(sortedLMSSuffixes);

    *output = finalSA;
}

int areArraysEqual(int* arr1, int* arr2, int len) {

    for (int i = 0; i < len; i++) {
        if (arr1[i] != arr2[i]) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char* argv[]) {

    if(argc != 2) {
        printf("Need one arguments when calling executable; %d given.\n", argc - 1);
        printf("Expected a filename which refers to the file containing the string.\n");
        //end program with error code
        exit(1);
    }
    FILE* file = fopen(argv[1], "r"); 
    if(file == NULL){
        printf("Error opening file %s.\n", argv[1]);
        exit(0);
    }

    char* str = read_file(file);
    fclose(file);
    // hardcode example string for short test input
    // char* str = "CGACTCCAACAACAAGCT";

    int len = strlen(str) + 1;
    clock_t naive_start = clock();
    int* naiveSuffixArray = naiveSA(str, len);
    clock_t naive_end = clock();
    double naive_time = (double)(naive_end - naive_start) / CLOCKS_PER_SEC;
    // printf("Naive ");
    // printSA(naiveSuffixArray, len);

    int* suffixArray = malloc(len*sizeof(int));
    int* convertedString = convertString(str, len); //we convert to allow for recursive (integer) inputs later
    clock_t SAIS_start = clock();
    SAIS(convertedString, len, 256, &suffixArray); //assumes 256 ASCII alphabet initially
    clock_t SAIS_end = clock();
    double SAIS_time = (double)(SAIS_end - SAIS_start) / CLOCKS_PER_SEC;
    // printf("SA-IS ");
    // printSA(suffixArray, len);

    if (areArraysEqual(naiveSuffixArray, suffixArray, len)) {
        printf("Success! Suffix arrays match.\n");
        printf("Naive took %.6f seconds.\n", naive_time);
        printf("SA-IS took %.6f seconds.\n", SAIS_time);
    }
    else {
        printf("Naive ");
        printSA(naiveSuffixArray, len);
        printf("SA-IS ");
        printSA(suffixArray, len);
        printf("Different suffix arrays from naive and SA-IS algorithms.\n");
    }

    free(naiveSuffixArray);
    free(suffixArray);
    free(convertedString);
    free(str);

    return 0;
}