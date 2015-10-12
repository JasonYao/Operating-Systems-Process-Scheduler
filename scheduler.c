#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {false, true} bool;        // Allows boolean types in C

/* Defines a job struct */
struct processNode {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time needed
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read

    int32_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated

    int32_t finishingTime;             // The cycle when the the process finishes (initially -1)
    uint32_t turnaroundTime;            // The finishingTime - A
    uint32_t IOTime;                    // Time in blocked state
    uint32_t waitingTime;               // Time in ready state
    struct processNode *next;           // Pointer to the next node in the list
};

// Global constants
bool IS_VERBOSE_MODE = false;           // Flags whether the output should be detailed or not
bool IS_RANDOM_MODE = false;            // Flags whether the output should include the random digit or not
uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
bool IS_PRINTED = false;                // Flags whether the output has already been printed yet

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";

// Original process queue head & tail pointers
struct processNode* head = NULL;
struct processNode* tail = NULL;
uint32_t processQueueSize = 0;

// readyQueue head & tail pointers
struct processNode* readyHead = NULL;
struct processNode* readyTail = NULL;
uint32_t readyProcessQueueSize = 0;

// readySuspendedQueue head & tail pointers
struct processNode* readySuspendedHead = NULL;
struct processNode* readySuspendedTail = NULL;
uint32_t readySuspendedProcessQueueSize = 0;

// runningQueue head & tail pointers
struct processNode* runningHead = NULL;
struct processNode* runningTail = NULL;
uint32_t runningProcessQueueSize = 0;

// blockedQueue head & tail pointers
struct processNode* blockedHead = NULL;
struct processNode* blockedTail = NULL;
uint32_t blockedProcessQueueSize = 0;

// blockedSuspendedQueue head & tail pointers
struct processNode* blockedSuspendedHead = NULL;
struct processNode* blockedSuspendedTail = NULL;
uint32_t blockedSuspendedProcessQueueSize = 0;


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
    char line[15];
    char* anyRemainderString;
    uint32_t unsignedRandomInteger;

    fgets(line, 15, randomNumberFile);
    unsignedRandomInteger = (uint32_t) strtoul(line, &anyRemainderString, 8);
    uint32_t returnValue = 1 + (unsignedRandomInteger % upperBound);
    return returnValue;
} // End of the randomOS function

/************************ START OF ORIGINAL PROCESS QUEUE HELPER FUNCTIONS *************************************/

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

/************************ END OF ORIGINAL PROCESS QUEUE HELPER FUNCTIONS *************************************/

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

/************************ START OF READY SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueReadySuspendedProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((readySuspendedHead == NULL) || (readySuspendedTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        readySuspendedHead = newNode;
        readySuspendedTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        readySuspendedTail->next = newNode;
        readySuspendedTail = newNode;
    }
    ++readySuspendedProcessQueueSize;
} // End of the readySuspended process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueReadySuspendedProcess()
{
    // Identical to removeFront() of a linked list
    if (readySuspendedHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = readySuspendedHead;
        readySuspendedHead = readySuspendedHead->next;

        if (readySuspendedHead == NULL)
            readySuspendedTail = NULL;
        --readySuspendedProcessQueueSize;
        return oldHead;
    }
} // End of the readySuspended process dequeue function

/************************ END OF READY SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF RUNNING QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueRunningProcess(struct processNode *newNode)
{
    printf("Got to start of enqueue running\n");
    // Identical to the insertBack() of a linked list
    if ((runningHead == NULL) || (runningTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        runningHead = newNode;
        runningTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        runningTail->next = newNode;
        runningTail = newNode;
    }
    ++runningProcessQueueSize;
    printf("Got to end of enqueue running\n");
    printf("At this moment, the size of the running process is: %i\n", runningProcessQueueSize);
} // End of the running process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueRunningProcess()
{
    // Identical to removeFront() of a linked list
    if (runningHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = runningHead;
        runningHead = runningHead->next;

        if (runningHead == NULL)
            runningTail = NULL;
        --runningProcessQueueSize;
        return oldHead;
    }
} // End of the running process dequeue function

/************************ END OF RUNNING QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF BLOCKED QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueBlockedProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((blockedHead == NULL) || (blockedTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        blockedHead = newNode;
        blockedTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        blockedTail->next = newNode;
        blockedTail = newNode;
    }
    ++blockedProcessQueueSize;
} // End of the blocked process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueBlockedProcess()
{
    // Identical to removeFront() of a linked list
    if (blockedHead == NULL)
    {
        printf("ERROR: Attempted to dequeue from the blocked process pool\n");
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = blockedHead;
        blockedHead = blockedHead->next;
        if (blockedHead == NULL)
            blockedTail = NULL;
        --blockedProcessQueueSize;
        printf("Attempting to dequeue from the blocked process pool\n");
        printf("At this moment, the blockedQueueSize is: %i\n", blockedProcessQueueSize);
        return oldHead;
    }
} // End of the blocked process dequeue function

/************************ END OF BLOCKED QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF BLOCKED SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueBlockedSuspendedProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((blockedSuspendedHead == NULL) || (blockedSuspendedTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        blockedSuspendedHead = newNode;
        blockedSuspendedTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        blockedSuspendedTail->next = newNode;
        blockedSuspendedTail = newNode;
    }
    ++blockedSuspendedProcessQueueSize;
} // End of the blockedSuspended process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueBlockedSuspendedProcess()
{
    // Identical to removeFront() of a linked list
    if (blockedSuspendedHead == NULL)
    {
        printf("ERROR: Attempted to dequeue from the blockedSuspended process pool\n");
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = blockedSuspendedHead;
        blockedSuspendedHead = blockedSuspendedHead->next;
        if (blockedSuspendedHead == NULL)
            blockedSuspendedTail = NULL;
        --blockedSuspendedProcessQueueSize;
        printf("Attempting to dequeue from the blockedSuspended process pool\n");
        printf("At this moment, the blockedSuspendedQueueSize is: %i\n", blockedSuspendedProcessQueueSize);
        return oldHead;
    }
} // End of the blockedSuspended process dequeue function

/************************ END OF BLOCKED SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

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

/************************ START OF RUNNING PROGRAM FUNCTIONS *************************************/

void DoBlockedProcesses()
{
    printf("Got to the start of DoBlockedProcesses\n");
    if (blockedSuspendedProcessQueueSize != 0)
    {
        // Does the blocked suspended portion first
        struct processNode* unBlockedNode = dequeueBlockedSuspendedProcess();
        printf("Unblocking process from blocked suspended to ready suspended\n");
        unBlockedNode->status = 1;
        enqueueReadySuspendedProcess(unBlockedNode);
    }
    if (blockedProcessQueueSize != 0)
    {
        // Does the blocked portion next
        struct processNode* unBlockedNode = dequeueBlockedProcess();
        printf("Suspending process from blocked to blocked suspended\n");
        enqueueBlockedSuspendedProcess(unBlockedNode);
//
//        if (runningProcessQueueSize > 0)
//        {
//            printf("A process is being \n");
//            // A process is being run, adds to the ready queue
//            unBlockedNode->waitingTime += 1;
//            unBlockedNode->status = 1;
//            enqueueReadyProcess(unBlockedNode);
//        }
//        else
//        {
//            printf("Got to blocked 3\n");
//            // No process is being run, can be moved directly to running
//            unBlockedNode->turnaroundTime += 1;
//            unBlockedNode->status = 2;
//            enqueueRunningProcess(unBlockedNode);
//            printf("At this point the node is in running mode\n");
//        }
    }

} // End of the doBlockedProcess function

void DoRunningProcesses()
{
    // Process finished running, needs to be moved to either finishedQueue, blockedQueue, or pre-empted into readyQueue
    if (runningProcessQueueSize != 0)
    {
        struct processNode* processedNode = dequeueRunningProcess();
        if (processedNode->C == processedNode->turnaroundTime)
        {
            // Process has completed running, moves to the completed block
            processedNode->status = 4;
            processedNode->finishingTime = CURRENT_CYCLE;
            enqueueFinishedProcess(processedNode);
        }
        else if (processedNode->C > processedNode->turnaroundTime)
        {
            // Process has not completed running, moves to the blocked queue
            processedNode->status = 3;
            processedNode->IOTime += 1;
            enqueueBlockedProcess(processedNode);
        }
        else
        {
            // Process has been preempted, moves back to the ready queue
            //TODO Will need to actually figure out the preemption code for this
            processedNode->status = 1;
            processedNode->waitingTime += 1;
            enqueueReadyProcess(processedNode);
        }
    }
    else
    {
        // No processes being run, does nothing
    }
} // End of the doRunningProcess function

void createProcesses()
{
    int i = 0;
    struct processNode* currentNode = head;
    for (; i < processQueueSize; ++i)
    {
        if (currentNode->A == CURRENT_CYCLE)
        {
            // It is the current cycle begin time for this process, moves this to running unless there is something run
            if (runningProcessQueueSize == 0)
            {
                // No other processes running, moves to running queue
                currentNode->turnaroundTime += 1;
                currentNode->status = 2;
                enqueueRunningProcess(currentNode);
            }
            else
            {
                // Process is running, waits in the ready queue
                currentNode->waitingTime += 1;
                currentNode->status = 1;
                enqueueReadyProcess(currentNode);
            }
        }
    }
} // End of the createProcess function

void DoReadyProcesses()
{
    if (readyProcessQueueSize != 0)
    {
        // Ready cycle is ready to move into the running queue
        struct processNode* readiedNode = dequeueReadyProcess();
        readiedNode->turnaroundTime += 1;
        enqueueRunningProcess(readiedNode);
    }
} // End of the doReadyProcess function

/************************ END OF RUNNING PROGRAM FUNCTIONS *************************************/

/********************************** START OF THE FCFS SPECIFIC FUNCTIONS **********************************/

/**
 * TODO finish
 */
void runFCFS()
{
    if (IS_VERBOSE_MODE)
    {
        int i = 0;
        struct processNode* currentNode = head;
        if (IS_PRINTED == false)
        {
            IS_PRINTED = true;


        }
        for (; i < processQueueSize; ++i)
        {
            printf("The size of the originalQueue is: %i\n", processQueueSize);
            printf("The size of the readyQueue is: %i\n", readyProcessQueueSize);
            printf("The size of the runningQueue is: %i\n", runningProcessQueueSize);
            printf("The size of the blockedQueue is: %i\n", blockedProcessQueueSize);
            printf("The size of the finishedQueue is: %i\n", finishedProcessQueueSize);
            if (currentNode == NULL)
            {
                printf("well fuck\n");
            }
            else
            {
                printf("Before cycle\t%i:\t", CURRENT_CYCLE);
                switch (currentNode->status)
                {
                    case 0:
                        // Node has not started
                        printf("unstarted\t\n");
                        break;
                    case 1:
                        // Node is ready
                        printf("ready\t\n");
                        break;
                    case 2:
                        // Node is running
                        printf("running\t\n");
                        break;
                    case 3:
                        // Node is I/O blocked
                        printf("blocked\t\n");
                        break;
                    case 4:
                        // Node is terminated
                        printf("terminated\t\n");
                        break;
                    default:
                        // Invalid node status, exiting now
                        fprintf(stderr, "Error: Invalid process status code, exiting now!\n");
                        exit(1);
                }
            }
        }
    }
    DoBlockedProcesses();
    DoRunningProcesses();
    createProcesses();
    DoReadyProcesses();
    ++CURRENT_CYCLE;
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
    switch(schedulerAlgorithm)
    {
        case 1:
            // Will print specifics for each process that has undergone FCFS scheduling
            break;
        case 2:
            // Will print specifics for each process that has undergone RR scheduling
            break;
        case 3:
            // Will print specifics for each process that has undergone Uniprogramming scheduling
            break;
        case 4:
            // Will print specifics for each process that has undergone SJF scheduling
            break;
        default:
            // Invalid scheduler algorithm provided, prints an error message and exits
            fprintf(stderr, "Error: Invalid scheduler algorithm provided, exiting now!\n");
            exit(1);
    }
} // End of the print process specifics function

void printSummaryData()
{
    //TODO finish
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
    while (processQueueSize != finishedProcessQueueSize)
    {
        runFCFS();
    }

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
        uint32_t inputA = 0;
        uint32_t inputB = 0;
        uint32_t inputC = 0;
        uint32_t inputM = 0;

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
        newNode->finishingTime = -1;
        newNode->turnaroundTime = 0;
        newNode->IOTime = 0;
        newNode->waitingTime = 0;
        newNode->status = 0;
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