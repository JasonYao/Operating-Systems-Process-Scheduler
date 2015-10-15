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

    uint32_t IOBurst;                   // The amount of IO time after a CPU burst
    uint32_t CPUBurst;                  // The amount of CPU time before a block is hit
    struct processNode *next;           // Pointer to the next node in the list
    struct processNode *nextInPreparation;           // Pointer to the next node in the list
    struct processNode *nextInReady;           // Pointer to the next node in the list
    struct processNode *nextInReadySuspended;           // Pointer to the next node in the list
    struct processNode *nextInRunning;           // Pointer to the next node in the list
    struct processNode *nextInBlocked;           // Pointer to the next node in the list
    struct processNode *nextInBlockedSuspended;           // Pointer to the next node in the list
    struct processNode *nextInFinished;           // Pointer to the next node in the list
};

// Global constants
bool IS_VERBOSE_MODE = false;           // Flags whether the output should be detailed or not
bool IS_RANDOM_MODE = false;            // Flags whether the output should include the random digit or not
uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
bool IS_PRINTED = false;                // Flags whether the output has already been printed yet
uint32_t TOTAL_CREATED_PROCESSES = 0;

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";

// Backup process queue head & tail pointers
struct processNode* backupHead = NULL;
struct processNode* backupTail = NULL;
uint32_t backupProcessQueueSize = 0;

// Original process queue head & tail pointers
struct processNode* head = NULL;
struct processNode* tail = NULL;
uint32_t processQueueSize = 0;

// preparationQueue head & tail pointers
struct processNode* preparationHead = NULL;
struct processNode* preparationTail = NULL;
uint32_t preparationProcessQueueSize = 0;

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
 * HELPER FUNCTION: Prints out all queue sizes
 */
void printAllQueueSizes()
{
    printf("The size of the original process queue is: %i\n", processQueueSize);
    printf("The size of the ready process queue is: %i\n", readyProcessQueueSize);
    printf("The size of the ready suspended process queue is: %i\n", readySuspendedProcessQueueSize);
    printf("The size of the running process queue is: %i\n", runningProcessQueueSize);
    printf("The size of the blocked process queue is: %i\n", blockedProcessQueueSize);
    printf("The size of the blocked suspended process queue is: %i\n", blockedSuspendedProcessQueueSize);
    printf("The size of the finished process queue is: %i\n", finishedProcessQueueSize);
} // End of the printAllQueueSizes helper function

/**
 * Reads a random non-negative integer X from a file named random-numbers (in the current directory)
 * NOTE: Only works if the integer values in random-number are less than 20 in length
 * @return The value 1 + (X % U)
 */
uint32_t randomOS(uint32_t upperBound, FILE* randomNumberFile)
{
    char str[20];
    uint32_t unsignedRandomInteger = (uint32_t) atoi(fgets(str, 20, randomNumberFile));
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

/************************ START OF BACKUP PROCESS QUEUE HELPER FUNCTIONS *************************************/

/**
 * A queue insertion function
 */
void enqueueBackupProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((backupHead == NULL) || (backupTail == NULL))
    {
        // Queue is empty, simply point backupHead and backupTail to the newNode
        backupHead = newNode;
        backupTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the backupTail
        backupTail->next = newNode;
        backupTail = newNode;
    }
    ++backupProcessQueueSize;
} // End of the process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueBackupProcess()
{
    // Identical to removeFront() of a linked list
    if (backupHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old backupHead for the return value, and sets the new backupHead
        struct processNode* oldBackupHead = backupHead;
        backupHead = backupHead->next;
        --backupProcessQueueSize;
        return oldBackupHead;
    }
} // End of the process dequeue function

/************************ END OF BACKUP PROCESS QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF PREPARATION QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueuePreparationProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if ((preparationHead == NULL) || (preparationTail == NULL))
    {
        // Queue is empty, simply point head and tail to the newNode
        preparationHead = newNode;
        preparationTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        preparationTail->nextInPreparation = newNode;
        preparationTail = newNode;
    }
    ++preparationProcessQueueSize;
} // End of the preparation process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeuePreparationProcess()
{
    // Identical to removeFront() of a linked list
    if (preparationHead == NULL)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = preparationHead;
        preparationHead = preparationHead->nextInPreparation;

        if (preparationHead == NULL)
            preparationTail = NULL;
        --preparationProcessQueueSize;
        return oldHead;
    }
} // End of the preparation process dequeue function

/************************ END OF PREPARATION QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF READY QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueReadyProcess(struct processNode *newNode)
{
    // Identical to the insertBack() of a linked list
    if (readyProcessQueueSize == 0)
    {
        // Queue is empty, simply point head and tail to the newNode
        readyHead = newNode;
        readyTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        newNode->nextInReady = NULL;
        readyTail->nextInReady = newNode;
        readyTail = readyTail->nextInReady;
    }
    ++readyProcessQueueSize;
} // End of the ready process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct processNode* dequeueReadyProcess()
{
    // Identical to removeFront() of a linked list
    if (readyProcessQueueSize == 0)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct processNode* oldHead = readyHead;
        readyHead = readyHead->nextInReady;
        --readyProcessQueueSize;

        if (readyProcessQueueSize == 0)
        {
            // Queue is now empty, with both head & tail set to NULL
            readyHead = NULL;
            readyTail = NULL;
        }
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
        readySuspendedTail->nextInReadySuspended = newNode;
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
        readySuspendedHead = readySuspendedHead->nextInReadySuspended;

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
        runningTail->nextInRunning = newNode;
        runningTail = newNode;
    }
    ++runningProcessQueueSize;
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
        runningHead = runningHead->nextInRunning;

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
        blockedTail->nextInBlocked = newNode;
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
        blockedHead = blockedHead->nextInBlocked;
        if (blockedHead == NULL)
            blockedTail = NULL;
        --blockedProcessQueueSize;
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
        blockedSuspendedTail->nextInBlockedSuspended = newNode;
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
        blockedSuspendedHead = blockedSuspendedHead->nextInBlockedSuspended;
        if (blockedSuspendedHead == NULL)
            blockedSuspendedTail = NULL;
        --blockedSuspendedProcessQueueSize;
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
        finishedTail->nextInFinished = newNode;
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
        finishedHead = finishedHead->nextInFinished;

        if (finishedHead == NULL)
            finishedTail = NULL;
        --finishedProcessQueueSize;
        return oldHead;
    }
} // End of the finished process dequeue function

/************************ END OF FINISHED QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF RUNNING PROGRAM FUNCTIONS *************************************/

void DoBlockedProcesses(uint32_t randomNumber)
{
    printf("Got to blocked 0\n");
    // Does the blocked suspended portion first
    if (blockedSuspendedProcessQueueSize != 0)
    {
        // Unblocks the process to the ready suspended queue
        struct processNode* unBlockedNode = dequeueBlockedSuspendedProcess();
        unBlockedNode->status = 1;
        enqueueReadySuspendedProcess(unBlockedNode);
    }

    if (blockedProcessQueueSize != 0)
    {
        struct processNode* currentNode = blockedHead;
        while (currentNode != NULL)
        {
            // Iterates through the blocked process queue
            if (currentNode->IOBurst == 0)
            {
                // IOBurst time is 0, moves to ready
                currentNode->CPUBurst = 1 + (randomNumber % currentNode->B);
                currentNode->status = 1;

                printf("Initial blocked queue is:%i\n", blockedProcessQueueSize); //TODO remove after
                // Removes this process from the blocked list to the ready queue
                if (currentNode == blockedHead)
                {
                    printf("Removing from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                    // Removes from the front, simply dequeues
                    struct processNode* unBlockedProcess = dequeueBlockedProcess();
                    enqueueReadyProcess(unBlockedProcess);
                    printf("Removed from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                }
                else if (currentNode == blockedTail)
                {
                    // Removes from the back
                    printf("Removing from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                    struct processNode* currentBlockedIterationProcess = blockedHead;
                    while (currentBlockedIterationProcess->nextInBlocked->nextInBlocked != NULL)
                    {currentBlockedIterationProcess = currentBlockedIterationProcess->nextInBlocked;}

                    blockedTail = currentBlockedIterationProcess;

                    struct processNode* nodeToBeAddedToReady = currentBlockedIterationProcess->nextInBlocked;
                    currentBlockedIterationProcess->nextInBlocked = NULL;
                    --blockedProcessQueueSize;

                    enqueueReadyProcess(nodeToBeAddedToReady);
                    printf("Removed from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                }
                else
                {
                    printf("Removing from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                    // Removes from the middle
                    struct processNode* currentBlockedIterationProcess = blockedHead;

                    // Iterates until right before the middle block
                    while (currentBlockedIterationProcess->nextInBlocked != currentNode)
                    {currentBlockedIterationProcess = currentBlockedIterationProcess->nextInBlocked;}

                    struct processNode* nodeToBeAddedToReady = currentBlockedIterationProcess->nextInBlocked;
                    currentBlockedIterationProcess->nextInBlocked =
                            currentBlockedIterationProcess->nextInBlocked->nextInBlocked;
                    --blockedProcessQueueSize;
                    enqueueReadyProcess(nodeToBeAddedToReady);
                    printf("Removed from blocked to ready from the front:%i\n", blockedProcessQueueSize); //TODO remove after
                }
            } // End of dealing with removing any processes that need to be moved to ready
            currentNode = currentNode->nextInBlocked;
        }
    }
    printf("Got to blocked 2\n");
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
        else
        {
            // Process has not completed running, moves to the blocked queue
            processedNode->status = 3;
            enqueueBlockedProcess(processedNode);
        }
    }
    else
    {
        // No processes being run, does nothing
    }
} // End of the doRunningProcess function

void createProcesses()
{
    // Iterates through the original list, finding those that begin at this cycle time
    uint32_t i;
    struct processNode* currentNode = head;
    for (i = 0; i < processQueueSize; ++i)
    {
        // For any that are found to start now, adds it to the prepared pool
        if (currentNode->A == CURRENT_CYCLE)
            enqueuePreparationProcess(currentNode);
        currentNode = currentNode->next;
    }

    // At this point, we have a queue of processes that all begin at the same time, in order of pid (lowest first)
    while (preparationProcessQueueSize != 0)
    {
        struct processNode* preparedProcess = dequeuePreparationProcess();
        ++TOTAL_CREATED_PROCESSES;
        preparedProcess->status = 1;
        enqueueReadyProcess(preparedProcess);
    }
} // End of the createProcess function

void DoReadyProcesses()
{
    // Deals with the ready suspended queue first
    if (readySuspendedProcessQueueSize != 0)
    {
        // Resumes the ready suspended process back to the ready queue
        struct processNode* readiedSuspendedNode = dequeueReadySuspendedProcess();
        enqueueReadyProcess(readiedSuspendedNode);
    }

    // Deals with the ready queue second
    if (readyProcessQueueSize != 0)
    {
        struct processNode* readiedNode = dequeueReadyProcess();
        if ((runningProcessQueueSize == 0) && (readiedNode->CPUBurst > 0))
        {
            // There are no running processes, runs the process, and adds it to the running pool
            readiedNode->status = 2;
            enqueueRunningProcess(readiedNode);
        }
//        else
//        {
//            printf("Got to 2\n");
//            // There are running processes, suspends the ready process to the ready suspended pool
//            enqueueReadySuspendedProcess(readiedNode);
//        }
    }

} // End of the doReadyProcess function

void incrementTimers(uint32_t randomNumber)
{
    int i;
    struct processNode* currentNode = readyHead;
    // Increments any processes in the ready pool (increases wait time by 1)
    for (i = 0; i < readyProcessQueueSize; ++i)
    {
        currentNode->waitingTime += 1;
        currentNode = currentNode->nextInReady;
    }
    // Increments any processes in the blocked pool (increases IO time by 1)
    currentNode = blockedHead;
    for (i = 0; i < blockedProcessQueueSize; ++i)
    {
        currentNode->IOTime += 1;
        currentNode->IOBurst -= 1;
        currentNode = currentNode->nextInBlocked;
    }

    // Increments any processes in the running pool (increases turnaround time by 1)
    currentNode = runningHead;
    for (i = 0; i < runningProcessQueueSize; ++i)
    {
        currentNode->IOBurst = currentNode->M * currentNode->CPUBurst;
        currentNode->turnaroundTime += 1;
        currentNode->CPUBurst -= 1;
        currentNode = currentNode->nextInRunning;
    }
} // End of the increment timers function

/************************ END OF RUNNING PROGRAM FUNCTIONS *************************************/

/********************************** START OF THE FCFS SPECIFIC FUNCTIONS **********************************/

/**
 * Runs the First Come First Serve Scheduler
 */
void runFCFS(FILE* randomNumberFile, uint8_t passNumber)
{
//    if (CURRENT_CYCLE == 20) //TODO remove after
//    {
//        printf("Exiting due to retardedness\n"); //TODO remove after
//        exit(1);
//    }
    char str[20];
    uint32_t unsignedRandomInteger;

    if ((IS_VERBOSE_MODE == true) && (IS_PRINTED == false)) // TODO change from false to true to hide the first round
    {
//        printf("The following shows the processes added here 2:\n"); //TODO remove after testing
//        struct processNode* new = head;
//        int j;
//        for (j = 0; j < processQueueSize; ++j)
//        {
//////            printf("For process: %i, the current A value is: %i\n", new->processID, new->A);
//////            printf("For process: %i, the current B value is: %i\n", new->processID, new->B);
//////            printf("For process: %i, the current C value is: %i\n", new->processID, new->C);
//////            printf("For process: %i, the current M value is: %i\n", new->processID, new->M);
//////            printf("For process: %i, the current turnaround time value is: %i\n", new->processID, new->turnaroundTime);
////            printf("For process: %i, the current status value is: ************%i*************\n", new->processID, new->status);
////            printf("For process: %i, the current IOBurst value is: %i\n", new->processID, new->IOBurst);
////            printf("For process: %i, the current CPUBurst value is: %i\n", new->processID, new->CPUBurst);
//////            printf("For process: %i, the current waiting in ready time is: %i\n", new->processID, new->waitingTime);
////            new = new->next;
//        }

//        printf("The ready queue size is: %i\n", readyProcessQueueSize);
//        if (readyHead == NULL)
//            printf("The ready head is null at cycle: %i\n", CURRENT_CYCLE);
//        if (readyTail == NULL)
//            printf("The ready tail is null at cycle: %i\n", CURRENT_CYCLE);
        //printAllQueueSizes(); //TODO remove after

        printf("Before cycle\t%i:\t", CURRENT_CYCLE);
        int i = 0;
        struct processNode* currentNode = head;


        for (; i < processQueueSize; ++i)
        {
            //printf("The current process %i's status is: %i\n", i, currentNode->status); //TODO remove after
            switch (currentNode->status)
            {
                case 0:
                    // Node has not started
                    printf("unstarted \t0\t");
                    break;
                case 1:
                    // Node is ready
                    printf("ready   \t0\t");
                    break;
                case 2:
                    // Node is running
                    printf("running \t1\t");
                    break;
                case 3:
                    // Node is I/O blocked
                    printf("blocked \t1\t");
                    break;
                case 4:
                    // Node is terminated
                    printf("terminated \t0\t");
                    break;
                default:
                    // Invalid node status, exiting now
                    fprintf(stderr, "Error: Invalid process status code, exiting now!\n");
                    exit(1);
            } // End of the switch statement
            currentNode = currentNode->next;
        } // End of the per line for loop
        printf("\n");
    } // End of the printed output

    unsignedRandomInteger = (uint32_t) atoi(fgets(str, 20, randomNumberFile));
    if ((IS_RANDOM_MODE) && (passNumber == 2))
        printf("Find burst when choosing ready process to run %i\n", unsignedRandomInteger);

    printf("Got to 1\n");
    DoBlockedProcesses(unsignedRandomInteger);
    printf("Got to 2\n");
    DoRunningProcesses();
    printf("Got to 3\n");

    // Checks whether the processes are all created, so it can skip creation again
    if (TOTAL_CREATED_PROCESSES != processQueueSize)
    {
        // Not all processes created, goes into creation loop
        createProcesses();
    }

    printf("Got to 4\n");
    DoReadyProcesses();
    printf("Got to 5\n");
    incrementTimers(unsignedRandomInteger);
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

void backup(FILE* randomNumberFile)
{
    // Copies all values from the original queue to the backup queue
    uint32_t i;
    struct processNode* currentOriginalNode = head;
    for (i = 0; i < processQueueSize; ++i)
    {
        struct processNode* newBackupNode = (struct processNode*) malloc(sizeof(struct processNode));
        newBackupNode->A = currentOriginalNode->A;
        newBackupNode->B = currentOriginalNode->B;
        newBackupNode->C = currentOriginalNode->C;
        newBackupNode->M = currentOriginalNode->M;
        newBackupNode->processID = currentOriginalNode->processID;
        newBackupNode->next = NULL;
        newBackupNode->finishingTime = -1;
        newBackupNode->turnaroundTime = 0;
        newBackupNode->IOTime = 0;
        newBackupNode->CPUBurst = randomOS(newBackupNode->B, randomNumberFile);
        newBackupNode->IOBurst = newBackupNode->M * newBackupNode->CPUBurst;
        newBackupNode->waitingTime = 0;
        newBackupNode->status = 0;
        newBackupNode->nextInRunning = currentOriginalNode->nextInRunning;
        newBackupNode->nextInBlocked = currentOriginalNode->nextInBlocked;
        newBackupNode->nextInPreparation = currentOriginalNode->nextInPreparation;
        newBackupNode->nextInReady = currentOriginalNode->nextInReady;
        newBackupNode->nextInFinished = currentOriginalNode->nextInFinished;
        newBackupNode->nextInBlockedSuspended = currentOriginalNode->nextInBlockedSuspended;
        newBackupNode->nextInReadySuspended = currentOriginalNode->nextInReadySuspended;
        enqueueBackupProcess(newBackupNode);
        currentOriginalNode = currentOriginalNode->next;
    }
} // End of the backup function

void restoreFromBackup()
{
    // Deletes everything from the original queue and resets counter
    while (processQueueSize != 0)
    {
        struct processNode* currentNode = dequeueProcess();
        free(currentNode);
    }

    // Restores everything using the backup queue
    int i;
    struct processNode* currentBackupNode = backupHead;
    for (i = 0; i < backupProcessQueueSize; ++i)
    {
        struct processNode* newNode = (struct processNode*) malloc(sizeof(struct processNode));
        newNode->A = currentBackupNode->A;
        newNode->B = currentBackupNode->B;
        newNode->C = currentBackupNode->C;
        newNode->M = currentBackupNode->M;
        newNode->processID = currentBackupNode->processID;
        newNode->next = NULL;
        newNode->finishingTime = -1;
        newNode->turnaroundTime = 0;
        newNode->CPUBurst = currentBackupNode->CPUBurst;
        newNode->IOBurst = currentBackupNode->IOBurst;
        newNode->IOTime = 0;
        newNode->waitingTime = 0;
        newNode->status = 0;
        newNode->nextInRunning = NULL;
        newNode->nextInBlocked = NULL;
        newNode->nextInPreparation = NULL;
        newNode->nextInReady = NULL;
        newNode->nextInFinished = NULL;
        newNode->nextInBlockedSuspended = NULL;
        newNode->nextInReadySuspended = NULL;
        enqueueProcess(newNode);
        currentBackupNode = currentBackupNode->next;
    }
} // End of the restore from backup

void printSummaryData()
{
    //TODO finish
} // End of the print summary data function

/**
 * Resets everything so the other schedulers can be run after
 */
void resetAllQueues()
{
    CURRENT_CYCLE = 0;
    TOTAL_CREATED_PROCESSES = 0;

    // readyQueue head & tail pointers
    readyHead = NULL;
    readyTail = NULL;
    readyProcessQueueSize = 0;

    // readySuspendedQueue head & tail pointers
    readySuspendedHead = NULL;
    readySuspendedTail = NULL;
    readySuspendedProcessQueueSize = 0;

    // runningQueue head & tail pointers
    runningHead = NULL;
    runningTail = NULL;
    runningProcessQueueSize = 0;

    // blockedQueue head & tail pointers
    blockedHead = NULL;
    blockedTail = NULL;
    blockedProcessQueueSize = 0;

    // blockedSuspendedQueue head & tail pointers
    blockedSuspendedHead = NULL;
    blockedSuspendedTail = NULL;
    blockedSuspendedProcessQueueSize = 0;

    // finishedQueue head & tail pointers
    finishedHead = NULL;
    finishedTail = NULL;
    finishedProcessQueueSize = 0;

    // Fixes the original process queue
    restoreFromBackup();
} // End of the scheduler reset function

void freeAll()
{
    resetAllQueues();

    // Gets rid of everything in the original process queue
    uint32_t i;
    while (processQueueSize != 0)
    {
        struct processNode* currentNode = dequeueProcess();
        free(currentNode);
    }

    // Gets rid of everything from the backup
    while (backupProcessQueueSize != 0)
    {
        struct processNode* currentNode = dequeueBackupProcess();
        free(currentNode);
    }
} // End of the free all queues function

/********************* END OF GLOBAL OUTPUT FUNCTIONS *********************************************************/

/******************* START OF THE OUTPUT WRAPPER FOR EACH SCHEDULING ALGORITHM *********************************/

void outputFCFS(FILE* randomNumberFile)
{
    printStart();
    // Runs this the first time in order to have the final output be available
    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    uint8_t passNumber = 1;
    while (processQueueSize != finishedProcessQueueSize)
        runFCFS(randomNumberFile, passNumber);
    fclose(randomNumberFile);

    printFinal();
    resetAllQueues();
    printf("\n");
    IS_PRINTED = true;
    ++passNumber;

    if (IS_VERBOSE_MODE)
        printf("This detailed printout gives the state and remaining burst for each process\n");
    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    while (processQueueSize != finishedProcessQueueSize)
        runFCFS(randomNumberFile, passNumber);
    fclose(randomNumberFile);
    IS_PRINTED = false;

    printf("The scheduling algorithm used was First Come First Served\n");
    printf("\n");
    printProcessSpecifics(1);
    printSummaryData();
} // End of the print first come first serve function

void outputRR(FILE* randomNumberFile)
{
} // End of the print round robin function


void outputUniprogrammed(FILE* randomNumberFile)
{
} // End of the print uniprogrammed function

void outputSJF(FILE* randomNumberFile)
{
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

    // [ERROR CHECKING]: INVALID FILENAME
    if (inputFile == NULL) {
        fprintf(stderr, "Error: cannot open input file %s!\n",filePath);
        exit(1);
    }

    uint32_t indicator;                                 // Indicator number showing the number of jobs
    uint32_t currentIndicatorCount = 0;                 // The current job that we have read in
    fscanf(inputFile, "%i", &indicator);

    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");

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
        fscanf(inputFile, " %*c%i %i %i %i%*c", &inputA, &inputB, &inputC, &inputM);

        newNode->A = inputA;
        newNode->B = inputB;
        newNode->C = inputC;
        newNode->M = inputM;
        newNode->processID = currentIndicatorCount;
        newNode->next = NULL;
        newNode->finishingTime = -1;
        newNode->turnaroundTime = 0;
        newNode->CPUBurst = randomOS(newNode->B, randomNumberFile);
        newNode->IOBurst = newNode->M * newNode->CPUBurst;
        newNode->IOTime = 0;
        newNode->waitingTime = 0;
        newNode->status = 0;

        newNode->nextInRunning = NULL;
        newNode->nextInBlocked = NULL;
        newNode->nextInPreparation = NULL;
        newNode->nextInReady = NULL;
        newNode->nextInFinished = NULL;
        newNode->nextInBlockedSuspended = NULL;
        newNode->nextInReadySuspended = NULL;

        ++currentIndicatorCount;

        // At this point, there is a new process created with the time, so now it needs to be inserted into the queue
        enqueueProcess(newNode);
    } // End of file input reading
    fclose(inputFile);
    fclose(randomNumberFile);

    // Backs up the original processes using the backup queue
    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    backup(randomNumberFile);
    fclose(randomNumberFile);

    // First Come First Serve Run
    printf("######################### START OF FIRST COME FIRST SERVE #########################\n");
    outputFCFS(randomNumberFile);
    resetAllQueues();       // Resets the cycle time to start the new process scheduler again
    printf("######################### END OF FCFS #########################\n");

    // Round Robin Run
    printf("######################### START OF ROUND ROBIN #########################\n");
    outputRR(randomNumberFile);
    resetAllQueues();       // Resets the cycle time to start the new process scheduler again
    printf("######################### END OF ROUND ROBIN #########################\n");

    // Uniprogrammed Run
    printf("######################### START OF UNIPROGRAMMED #########################\n");
    outputUniprogrammed(randomNumberFile);
    resetAllQueues();       // Resets the cycle time to start the new process scheduler again
    printf("######################### END OF UNIPROGRAMMED #########################\n");

    // Shortest Job First Run
    printf("######################### START OF SHORTEST JOB FIRST #########################\n");
    outputSJF(randomNumberFile);
    printf("######################### END OF SHORTEST JOB FIRST #########################\n");

    freeAll();              // Resets and frees all objects

    return EXIT_SUCCESS;
} // End of the main function