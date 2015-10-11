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
    uint32_t processID;                 // The process ID given upon input read
    struct processNode *next;           // Pointer to the next node in the list
};

// Global constants
bool IS_VERBOSE_MODE = false;           // Flags whether the output should be detailed or not
bool IS_RANDOM_MODE = false;            // Flags whether the output should include the random digit or not
uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";

// Original process queue head & tail pointers
struct processNode* head = NULL;
struct processNode* tail = NULL;
uint32_t processQueueSize = 0;

// readyQueue head & tail pointers
struct processNode* readyHead = NULL;
struct processNode* readyTail = NULL;
uint32_t readyProcessQueueSize = 0;

// BlockedQueue head & tail pointers
struct processNode* blockedHead = NULL;
struct processNode* blockedTail = NULL;
uint32_t blockedProcessQueueSize = 0;

// finishedQueue head & tail pointers
struct processNode* finishedHead = NULL;
struct processNode* finishedTail = NULL;
uint32_t finishedProcessQueueSize = 0;


/**
 * Reads a random non-negative integer X from a file named random-numbers (in the current directory)
 * @return The value 1 + (X % U)
 */
uint32_t randomOS(uint32_t upperBound, FILE* randomNumberFile)
{
    uint32_t randomNumber = 0;
    char line[15];
    fgets(line, 15, randomNumberFile);
    randomNumber = atoi(line);
    uint32_t returnValue = 1 + (randomNumber % upperBound);
    return returnValue;
} // End of the randomOS function


/************************ START OF FINISHED QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueFinishedProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((finishedHead == NULL) || (finishedTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        finishedHead = newNode;
        finishedTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        finishedTail->next = newNode;
        finishedTail = newNode;
    }
    ++finishedProcessQueueSize;
} // End of the finished process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueFinishedProcess()
{
    // Identical to removeFront() of a linked list
    if (finishedHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = finishedHead;
        finishedHead = finishedHead->next;

        if (finishedHead == NULL)
            finishedTail = NULL;
        --finishedProcessQueueSize;
        return oldHead;
    }
} // End of the finished process dequeue function

/************************ END OF FINISHED QUEUE HELPER FUNCTIONS *************************************/


/************************ START OF READY QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueReadyProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((readyHead == NULL) || (readyTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        readyHead = newNode;
        readyTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        readyTail->next = newNode;
        readyTail = newNode;
    }
    ++readyProcessQueueSize;
} // End of the ready process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueReadyProcess()
{
    // Identical to removeFront() of a linked list
    if (readyHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = readyHead;
        readyHead = readyHead->next;

        if (readyHead == NULL)
            readyTail = NULL;
        --readyProcessQueueSize;
        return oldHead;
    }
} // End of the ready process dequeue function

/************************ END OF READY QUEUE HELPER FUNCTIONS *************************************/

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

/********************************** START OF THE FCFS SPECIFIC FUNCTIONS **********************************/

void runFCFS()
{

} // End of the runFCFS function

/********************************** END OF THE FCFS SPECIFIC FUNCTIONS **********************************/

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

/********************* START OF GLOBAL OUTPUT FUNCTIONS *********************************************************/

/**
 * Prints to standard output the original input
 */
void printStart()
{
    printf("The original input was: %i", processQueueSize);

    int32_t i;
    struct processNode* currentNode = head;
    for (i = 0; i < processQueueSize; ++i)
    {
        printf(" ( %i %i %i %i  )", currentNode->A, currentNode->B, currentNode->C, currentNode->M);
        currentNode = currentNode->next;
    }
    printf("\n");
} // End of the print start function

/**
 * Prints to standard output the final output
 */
void printFinal()
{
    printf("The (sorted) input is: %i", finishedProcessQueueSize);

    int32_t i;
    struct processNode* currentNode = finishedHead;
    for (i = 0; i < finishedProcessQueueSize; ++i)
    {
        printf(" ( %i %i %i %i  )", currentNode->A, currentNode->B, currentNode->C, currentNode->M);
        currentNode = currentNode->next;
    }
    printf("\n");
} // End of the print final function

void printProcessSpecifics(uint8_t schedulerAlgorithm)
{
    if (schedulerAlgorithm == 1)
    {
        // Will print specifics for each process that has undergone FCFS scheduling
    }
    else if (schedulerAlgorithm == 2)
    {
        // Will print specifics for each process that has undergone RR scheduling
    }
    else if (schedulerAlgorithm == 3)
    {
        // Will print specifics for each process that has undergone Uniprogramming scheduling
    }
    else
    {
        // Will print specifics for each process that has undergone SJF scheduling
    }
} // End of the print process specifics function

void printSummaryData()
{

} // End of the print summary data function

/**
 * Resets everything so the other schedulers can be run after
 */
void resetFinishedQueue()
{
    CURRENT_CYCLE = 0;

    // Gets rid of everything in the finished queue
    uint32_t i;
    for (i = 0; i < finishedProcessQueueSize; ++i)
    {
        struct processNode* currentNode = dequeueFinishedProcess();
        free(currentNode);
    }
} // End of the scheduler reset function

void freeAllQueues()
{
    // Gets rid of everything in the finished process queue
    uint32_t i;
    for (i = 0; i < finishedProcessQueueSize; ++i)
    {
        struct processNode* currentNode = dequeueFinishedProcess();
        free(currentNode);
    }

    // Gets rid of everything in the ready process queue
    for (i = 0; i < readyProcessQueueSize; ++i)
    {
        struct processNode* currentNode = dequeueReadyProcess();
        free(currentNode);
    }

    // Gets rid of everything in the original process queue
    for (i = 0; i < processQueueSize; ++i)
    {
        struct processNode* currentNode = dequeueProcess();
        free(currentNode);
    }
} // End of the free all queus function

/********************* END OF GLOBAL OUTPUT FUNCTIONS *********************************************************/

/******************* START OF THE OUTPUT WRAPPER FOR EACH SCHEDULING ALGORITHM *********************************/

void outputFCFS()
{
    printStart();
    printFinal();
    printf("\n");

    if (IS_VERBOSE_MODE)
        printf("This detailed printout gives the state and remaining burst for each process\n");
    runFCFS();
    printf("The scheduling algorithm used was First Come First Served\n");
    printf("\n");
    printProcessSpecifics(1);
    printSummaryData();
} // End of the print first come first serve function

void outputRR()
{
//    printStart(); TODO
//    printFinal();
} // End of the print round robin function


void outputUniprogrammed()
{
//    printStart(); TODO
//    printFinal();
} // End of the print uniprogrammed function

void outputSJF()
{
//    printStart(); TODO
//    printFinal();
} // End of the print shortest job first function

/******************* END OF THE OUTPUT WRAPPER FOR EACH SCHEDULING ALGORITHM *********************************/

/**
 * Runs the actual process scheduler, based upon the commandline input. For example run commands, please see the README
 */
int main(int argc, char *argv[])
{
    // Reads in from file
    FILE* inputFile;
    FILE* randomNumberFile;
    char* filePath;

    filePath = argv[setFlags(argc, argv)]; // Sets any global flags from input
    inputFile = fopen(filePath, "r");
    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");

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
        // Ain't C cool, that you can read in something like this, that scans in the job
        fscanf(inputFile, "%*c%*c%i %i %i %i%*c", &inputA, &inputB, &inputC, &inputM);
        newNode->A = inputA;
        newNode->B = inputB;
        newNode->C = inputC;
        newNode->M = inputM;
        newNode->processID = currentIndicatorCount;
        newNode->next = NULL;
        ++currentIndicatorCount;

        // At this point, there is a new process created with the time, so now it needs to be inserted into the queue
        enqueueProcess(newNode);
    } // End of file input reading
    fclose(inputFile);

    outputFCFS();
    resetFinishedQueue(); // Resets the cycle time to start the new process scheduler again

    outputRR();
    resetFinishedQueue(); // Resets the cycle time to start the new process scheduler again

    outputUniprogrammed();
    resetFinishedQueue(); // Resets the cycle time to start the new process scheduler again

    outputSJF();

    fclose(randomNumberFile);
    freeAllQueues();
    return EXIT_SUCCESS;
} // End of the main function