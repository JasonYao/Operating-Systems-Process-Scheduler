#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {false, true} bool;    // Allows boolean types in C

/* Defines a job struct */
struct processNode {
    int32_t A;                          // A: Arrival time of the process
    int32_t B;                          // B: Upper Bound of CPU burst times of the given random integer list
    int32_t C;                          // C: Total CPU time needed
    int32_t M;                          // M: Multiplier of CPU burst time
    struct processNode *next;       // Pointer to the next node in the list
};

// Global constants
bool IS_VERBOSE_MODE = false;       // Flags whether the output should be detailed or not
bool IS_RANDOM_MODE = false;        // Flags whether the output should include the random digit or not

// Process queue head & tail pointers
struct processNode* head = NULL;
struct processNode* tail = NULL;
uint32_t processQueueSize = 0;

int32_t randomOS(int32_t upperBound)
{

    return 1;
} // End of the randomOS function

/**
 * A queue insertion function
 */
void enqueueProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((head == NULL) || (tail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        head = newNode;
        tail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        tail->next = newNode;
        tail = newNode;
    }
    ++processQueueSize;
} // End of the process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueProcess()
{
    // Identical to removeFront() of a linked list
    if (head == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = head;
        head = head->next;
        --processQueueSize;
        return oldHead;
    }
} // End of the process dequeue function

/**
 * Sets global flags for output depending on user input
 */
uint8_t setFlags(int32_t argc, char *argv[])
{
    if (argc == 2)
        return 1;
    else
    {
        if ((strcmp(argv[1], "--verbose") == 0) || (strcmp(argv[2], "--verbose") == 0))
            IS_VERBOSE_MODE = true;
        if ((strcmp(argv[1], "--random") == 0) || (strcmp(argv[2], "--random") == 0))
            IS_RANDOM_MODE = true;
        if ((strcmp(argv[1], "--random") != 0) && (strcmp(argv[1], "--verbose") != 0))
            return 1;
        else if ((strcmp(argv[2], "--random") != 0) && (strcmp(argv[2], "--verbose") != 0))
            return 2;
        else
            return 3;
    }
} // End of the setFlags function

int main(int argc, char *argv[])
{
    // Reads in from file
    FILE *inputFile;
    char *filePath;

    filePath = argv[setFlags(argc, argv)]; // Sets any global flags from input

    inputFile = fopen(filePath, "r");
    // [ERROR CHECKING]: INVALID FILENAME
    if (inputFile == NULL) {
        fprintf(stderr, "Error: cannot open input file %s!\n",filePath);
        exit(1);
    }

    uint32_t indicator;                                 // Indicator number showing the number of jobs
    uint32_t currentIndicatorCount = 0;                 // The current job that we have read in
    fscanf(inputFile, "%i", &indicator);

    // Reads through the input, and adds processes by ordering
    while (!feof(inputFile))
    {
        int32_t inputA = 0;
        int32_t inputB = 0;
        int32_t inputC = 0;
        int32_t inputM = 0;

        if (indicator == currentIndicatorCount)
            break;

        struct processNode* newNode = (struct processNode*) malloc(sizeof(struct processNode));
        // Ain't C cool, that you can read in something like this, scans in the job
        fscanf(inputFile, "%*c%*c%i %i %i %i%*c", &inputA, &inputB, &inputC, &inputM);
        newNode->A = inputA;
        newNode->B = inputB;
        newNode->C = inputC;
        newNode->M = inputM;
        ++currentIndicatorCount;

        // At this point, there is a new process created with the time, so now it needs to be inserted into the queue
        enqueueProcess(newNode);
    } // End of file input reading
    printf("Got to the end of the file reading\n");

    // TODO remove after testing
    // Tests whether each node has its correct ABCM values
    int32_t i;
    struct processNode* currentNode = head;
    printf("Size of the queue is: %i\n", processQueueSize);
    for (i = 0; i < processQueueSize; ++i)
    {
        if (currentNode == NULL)
        {
            printf("Queue is empty, and you should feel bad for having bad scope\n");
        }
        else
        {
            printf("\n");
            printf("Start of node: %i\n", i);
            printf("Current's A value is: %i\n", currentNode->A);
            printf("Current's B value is: %i\n", currentNode->B);
            printf("Current's C value is: %i\n", currentNode->C);
            printf("Current's M value is: %i\n", currentNode->M);
            printf("End of node: %i\n", i);
            currentNode = currentNode->next;
        }
    }

    printf("\n");
    // Closes files & exits
    fclose(inputFile);
    return EXIT_SUCCESS;
} // End of the main function