#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef enum {false, true} bool;        // Allows boolean types in C

/* Defines a job struct */
struct Process {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time required
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read

    uint8_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated

    int32_t finishingTime;              // The cycle when the the process finishes (initially -1)
    uint32_t currentCPUTimeRun;         // The amount of time the process has already run (time in running state)
    uint32_t currentIOBlockedTime;      // The amount of time the process has been IO blocked (time in blocked state)
    uint32_t currentWaitingTime;        // The amount of time spent waiting to be run (time in ready state)

    uint32_t IOBurst;                   // The amount of time until the process finishes being blocked
    uint32_t CPUBurst;                  // The CPU availability of the process (has to be > 1 to move to running)

    bool isFirstTimeRunning;

    struct Process* nextInBlockedList;  // A pointer to the next process available in the blocked list
    struct Process* nextInReadyQueue;   // A pointer to the next process available in the ready queue
    struct Process* nextInReadySuspendedQueue; // A pointer to the next process available in the ready suspended queue
};

/* Global values */
// Flags to be set
bool IS_VERBOSE_MODE = false;           // Flags whether the output should be detailed or not
bool IS_RANDOM_MODE = false;            // Flags whether the output should include the random digit or not
bool IS_PROCESS_RUNNING = false;        // Flags whether any process is currently running

uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
uint32_t TOTAL_CREATED_PROCESSES = 0;   // The total number of processes constructed
uint32_t TOTAL_STARTED_PROCESSES = 0;   // The total number of processes that have started being simulated
uint32_t TOTAL_FINISHED_PROCESSES = 0;  // The total number of processes that have finished running
uint32_t TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0;

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";

/* Queue & List pointers */
// readyQueue head & tail pointers
struct Process* readyHead = NULL;
struct Process* readyTail = NULL;
uint32_t readyProcessQueueSize = 0;

// readySuspendedQueue head & tail pointers
struct Process* readySuspendedHead = NULL;
struct Process* readySuspendedTail = NULL;
uint32_t readySuspendedProcessQueueSize = 0;

// blockedList head & tail pointers
struct Process* blockedHead = NULL;
struct Process* blockedTail = NULL;
uint32_t blockedProcessListSize = 0;

// finished processes pointer
struct Process* runningProcess = NULL;

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

/************************ START OF READY QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueReadyProcess(struct Process* newNode)
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
        newNode->nextInReadyQueue = NULL;

        readyTail->nextInReadyQueue = newNode;
        readyTail = readyTail->nextInReadyQueue; // Sets the new tail.nextInReady == NULL
    }
    ++readyProcessQueueSize;
} // End of the ready process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct Process* dequeueReadyProcess()
{
//    printf("Got to the start of dequeueReady\n");
    // Identical to removeFront() of a linked list
    if (readyProcessQueueSize == 0)
    {
        // Queue is empty, returns null
//        printf("Got to the end of dequeueReady\n");
        return NULL;
    }
    else
    {
//        printf("Queue is not empty, retains old head for return value\n");
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct Process* oldHead = readyHead;
        readyHead = readyHead->nextInReadyQueue;
        --readyProcessQueueSize;

        // Queue is now empty, with both head & tail set to NULL
        if (readyProcessQueueSize == 0)
            readyTail = NULL;
        oldHead->nextInReadyQueue = NULL;
//        printf("Got to the end of dequeueReady\n");
        return oldHead;
    }
} // End of the ready process dequeue function

/************************ END OF READY QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF READY SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void enqueueReadySuspendedProcess(struct Process* newNode)
{
    // Identical to the insertBack() of a linked list
    if (readySuspendedProcessQueueSize == 0)
    {
        // Queue is empty, simply point head and tail to the newNode
        readySuspendedHead = newNode;
        readySuspendedTail = newNode;
    }
    else
    {
        // Queue is not empty, gets the back and inserts behind the tail
        newNode->nextInReadySuspendedQueue = NULL;

        readySuspendedTail->nextInReadySuspendedQueue = newNode;
        readySuspendedTail = readySuspendedTail->nextInReadySuspendedQueue; // Sets the new tail.nextInreadySuspended == NULL
    }
    ++readySuspendedProcessQueueSize;
} // End of the readySuspended process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct Process* dequeueReadySuspendedProcess()
{
    // Identical to removeFront() of a linked list
    if (readySuspendedProcessQueueSize == 0)
    {
        // Queue is empty, returns null
        return NULL;
    }
    else
    {
        // Queue is not empty, retains the old head for the return value, and sets the new head
        struct Process* oldHead = readySuspendedHead;
        readySuspendedHead = readySuspendedHead->nextInReadySuspendedQueue;
        --readySuspendedProcessQueueSize;

        // Checks if queue is now empty, with both head & tail set to NULL
        if (readySuspendedProcessQueueSize == 0)
            readySuspendedTail = NULL;

        oldHead->nextInReadySuspendedQueue = NULL;
        return oldHead;
    }
} // End of the readySuspended process dequeue function

/************************ END OF READY SUSPENDED QUEUE HELPER FUNCTIONS *************************************/

/************************ START OF BLOCKED LIST HELPER FUNCTIONS *************************************/

/**
* A queue insertion function
*/
void addToBlockedList(struct Process* newNode)
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
        blockedTail->nextInBlockedList = newNode;
        blockedTail = newNode;
    }
    ++blockedProcessListSize;
} // End of the blocked process enqueue function

/**
 * Dequeues the process from the queue, and returns the removed node
 */
struct Process* dequeueBlockedProcess()
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
        struct Process* oldHead = blockedHead;
        blockedHead = blockedHead->nextInBlockedList;
        if (blockedHead == NULL)
            blockedTail = NULL;
        --blockedProcessListSize;
        oldHead->nextInBlockedList = NULL;
        return oldHead;
    }
} // End of the blocked process dequeue function

/************************ END OF BLOCKED LIST HELPER FUNCTIONS *************************************/

/************************ START OF RUNNING PROGRAM FUNCTIONS *************************************/

void DoBlockedProcesses()
{
//    printf("Got to doBlocked\n");
    if (blockedProcessListSize != 0)
    {
//        printf("Does some operations in the blocked list\n");
        struct Process* currentNode = blockedHead;
        while (currentNode != NULL)
        {
//            printf("process with id: %i has an IOBurst of: %i\n", currentNode->processID, currentNode->IOBurst);
            // Iterates through the blocked process list
            if ( (int32_t) currentNode->IOBurst <= 0)
            {
//                printf("IO burst is negative or 0\n");
                // IOBurst time is 0, moves to ready
                currentNode->status = 1;

                // Removes this process from the blocked list to the ready queue
                if (currentNode->processID == blockedHead->processID)
                {
//                    printf("About to move from the front of blocked to ready\n");
                    // Removes from the front, simply dequeues
                    struct Process* unBlockedProcess = dequeueBlockedProcess();
                    enqueueReadyProcess(unBlockedProcess);
                }
                else if (currentNode->processID == blockedTail->processID)
                {
//                    printf("About to move from the back of blocked to ready\n");
                    // Removes from the back
                    struct Process* currentBlockedIterationProcess = blockedHead;
                    while (currentBlockedIterationProcess->nextInBlockedList->nextInBlockedList != NULL)
                    {currentBlockedIterationProcess = currentBlockedIterationProcess->nextInBlockedList;}

                    blockedTail = currentBlockedIterationProcess;

                    struct Process* nodeToBeAddedToReady = currentBlockedIterationProcess->nextInBlockedList;
                    currentBlockedIterationProcess->nextInBlockedList = NULL;
                    nodeToBeAddedToReady->nextInBlockedList = NULL;
                    --blockedProcessListSize;

                    enqueueReadyProcess(nodeToBeAddedToReady);
                }
                else
                {
//                    printf("About to move from the middle of blocked to ready\n");
                    // Removes from the middle
                    struct Process* currentBlockedIterationProcess = blockedHead;

                    // Iterates until right before the middle block
                    while (currentBlockedIterationProcess->nextInBlockedList->processID != currentNode->processID)
                    {currentBlockedIterationProcess = currentBlockedIterationProcess->nextInBlockedList;}

                    struct Process* nodeToBeAddedToReady = currentBlockedIterationProcess->nextInBlockedList;

                    currentBlockedIterationProcess->nextInBlockedList =
                            currentBlockedIterationProcess->nextInBlockedList->nextInBlockedList;
                    nodeToBeAddedToReady->nextInBlockedList = NULL;
                    --blockedProcessListSize;
                    enqueueReadyProcess(nodeToBeAddedToReady);
                }
            } // End of dealing with removing any processes that need to be moved to ready
            currentNode = currentNode->nextInBlockedList;
        } // End of iterating through the blocked queue
    }
//    printf("Got to the end of doBlocked\n");
} // End of the doBlockedProcess function

void DoRunningProcesses(struct Process finishedProcessContainer[])
{
//    printf("Got to doRunning\n");
    // Process finished running, needs to be moved to either finishedQueue, blockedQueue, or pre-empted into readyQueue
    if (IS_PROCESS_RUNNING)
    {
//        printf("A process is running, is in doRunning\n");
        if (runningProcess->C == runningProcess->currentCPUTimeRun)
        {
//            printf("A process has completed running, moves to completed block\n");
            // Process has completed running, moves to the completed block
            runningProcess->status = 4;
            runningProcess->finishingTime = CURRENT_CYCLE;

            finishedProcessContainer[TOTAL_FINISHED_PROCESSES] = *runningProcess; //TODO might need to do this brute force
            runningProcess = NULL;
            IS_PROCESS_RUNNING = false;
            ++TOTAL_FINISHED_PROCESSES;
        }
        else if (runningProcess->CPUBurst > 0)
        {
//            printf("A process has not completed running, simply decrements the CPU Burst\n");
            // Process has not completed running, still has CPU burst
            if (runningProcess->isFirstTimeRunning == true)
            {
//                printf("Got to inside of isFirstTimeRunning\n");
                runningProcess->isFirstTimeRunning = false;
                runningProcess->IOBurst = runningProcess->M * (runningProcess->CPUBurst + 1); //TODO check if required
                runningProcess->status = 2; // Shouldn't require this, but switches to ready occur without this statement
            }
        }
        else
        {
//            printf("A process has not completed running, moves to blocked list\n");
             //Process has not completed running, no more CPU burst, moves to the blocked list
            runningProcess->status = 3;
            addToBlockedList(runningProcess);
            runningProcess = NULL;
            IS_PROCESS_RUNNING = false;
        }
    }
    else
    {
        // No processes being run, does nothing
    }
} // End of the doRunningProcess function

void createProcesses(struct Process processContainer[])
{
//    printf("Got to createProcess\n");
    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        if (processContainer[i].A == CURRENT_CYCLE)
        {
            // Time for this process to be created, and enqueued to the ready queue
            ++TOTAL_STARTED_PROCESSES;
            processContainer[i].status = 1; // Sets the status to ready
            enqueueReadyProcess(&processContainer[i]);
        }
    }
} // End of the createProcess function

void DoReadyProcesses(uint8_t schedulerAlgorithm, uint8_t currentPassNumber, FILE* randomFile)
{
//    printf("Got to doReady\n");
    // Deals with the ready suspended queue first
    if (readySuspendedProcessQueueSize != 0)
    {
        // Resumes the ready suspended process back to the ready queue
        struct Process* readiedSuspendedNode = dequeueReadySuspendedProcess();
        enqueueReadyProcess(readiedSuspendedNode);
    }
    // End of dealing with the ready suspended queue

    // Deals with the ready queue second
    if (readyProcessQueueSize != 0)
    {
        if (IS_PROCESS_RUNNING == false)
        {
//            printf("Got to 1\n");
            // Only does things when there are no processes running
            struct Process* readiedNode = dequeueReadyProcess();

//            printf("Got to 2\n");
            // Calculates CPU Burst stuff
            char str[20];
            uint32_t unsignedRandomInteger = (uint32_t) atoi(fgets(str, 20, randomFile));
            // Prints out the random number, assuming the random flag is passed in
            if ((IS_RANDOM_MODE) && (currentPassNumber % 2 != 0)) // TODO change ! to = after testing
                printf("Find burst when choosing ready process to run %i\n", unsignedRandomInteger);

//            printf("Got to 3\n");
            uint32_t newCPUBurst = 1 + (unsignedRandomInteger % readiedNode->B);
            // Checks if the new CPU Burst time is greater than the time remaining
            if (newCPUBurst > (readiedNode->C - readiedNode->currentCPUTimeRun))
                newCPUBurst = readiedNode->C - readiedNode->currentCPUTimeRun;
            readiedNode->CPUBurst = newCPUBurst;

            if (readiedNode->CPUBurst > 0)
            {
                // There are no running processes, runs the process, and adds it to the running pool
                readiedNode->status = 2;
                readiedNode->isFirstTimeRunning = true;
                IS_PROCESS_RUNNING = true;
                runningProcess = readiedNode;
            }
        }
        else if ((IS_PROCESS_RUNNING == true) && (schedulerAlgorithm == 2))
        {
            // [UNIPROGRAMMED] There are running processes, suspends the ready process to the ready suspended pool
            struct Process* readiedNode = dequeueReadyProcess();
            enqueueReadySuspendedProcess(readiedNode);
        }
    }
    // End of dealing with the ready queue
//    printf("Got to the end of doReadyProcess\n");
} // End of the doReadyProcess function

void incrementTimers(struct Process processContainer[])
{
//    printf("Got to incrementTimers\n");
    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        switch (processContainer[i].status)
        {
            case 0:
                // Node has not started
                break;
            case 3:
                // Node is I/O blocked (I/O time)
                ++processContainer[i].currentIOBlockedTime;
//                printf("For Process %i, the current IOBlockedTime is: %i\n", i, processContainer[i].currentIOBlockedTime);
                --processContainer[i].IOBurst;
//                printf("For Process %i, the current IO burst is: %i\n", i, processContainer[i].IOBurst);
                //TODO check if need to add calculate cpu burst here, or earlier
                break;
            case 2:
                // Node is running (CPU time)
                //currentProcess.IOBurst = currentProcess.M * currentProcess.CPUBurst; //TODO check if required, or if possible to do earlier
                ++processContainer[i].currentCPUTimeRun;
//                printf("For Process %i, the current CPUTimeRun is: %i\n", i, processContainer[i].currentCPUTimeRun);
                --processContainer[i].CPUBurst;
//                printf("For Process %i, the current CPU Burst is: %i\n", i, processContainer[i].CPUBurst);
                break;
            case 1:
                // Node is ready (waiting)
                ++processContainer[i].currentWaitingTime;
//                printf("For Process %i, the current waiting time is: %i\n", i, processContainer[i].currentWaitingTime);
                break;
            case 4:
                // Node is terminated
                break;
            default:
                // Invalid node status, exiting now
                fprintf(stderr, "Error: Invalid process status code, exiting now!\n");
                exit(1);
        } // End of the per process status print statement
    }

    for (i = 0; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        if (processContainer[i].status == 3)
        {
            // At least 1 process is blocked, increments the total count
            ++TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED;
            break;
        }
    }
//    printAllQueueSizes();
//    if (IS_PROCESS_RUNNING)
//        printf("There is a process running\n");
//    printf("\n");
} // End of the increment timers function

/************************ END OF RUNNING PROGRAM FUNCTIONS *************************************/

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
void printStart(struct Process processContainer[])
{
    printf("The original input was: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", processContainer[i].A, processContainer[i].B,
               processContainer[i].C, processContainer[i].M);
    }
    printf("\n");
} // End of the print start function

/**
 * Prints to standard output the final output
 */
void printFinal(struct Process finishedProcessContainer[])
{
    printf("The (sorted) input is: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_FINISHED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", finishedProcessContainer[i].A, finishedProcessContainer[i].B,
               finishedProcessContainer[i].C, finishedProcessContainer[i].M);
    }
    printf("\n");
} // End of the print final function

void printProcessSpecifics(struct Process processContainer[])
{
    uint32_t i = 0;
    printf("\n");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf("Process %i:\n", processContainer[i].processID);
        printf("\t(A,B,C,M) = (%i,%i,%i,%i)\n", processContainer[i].A, processContainer[i].B,
               processContainer[i].C, processContainer[i].M);
        printf("\tFinishing time: %i\n", processContainer[i].finishingTime);
        printf("\tTurnaround time: %i\n", processContainer[i].finishingTime - processContainer[i].A);
        printf("\tI/O time: %i\n", processContainer[i].currentIOBlockedTime);
        printf("\tWaiting time: %i\n", processContainer[i].currentWaitingTime);
        printf("\n");
    }
} // End of the print process specifics function

void printSummaryData(struct Process processContainer[])
{
    uint32_t i = 0;
    double totalAmountOfTimeUtilisingCPU = 0.0;
    double totalAmountOfTimeIOBlocked = 0.0;
    double totalAmountOfTimeSpentWaiting = 0.0;
    double totalTurnaroundTime = 0.0;
    uint32_t finalFinishingTime = CURRENT_CYCLE - 1;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        totalAmountOfTimeUtilisingCPU += processContainer[i].currentCPUTimeRun;
        totalAmountOfTimeIOBlocked += processContainer[i].currentIOBlockedTime;
        totalAmountOfTimeSpentWaiting += processContainer[i].currentWaitingTime;
        totalTurnaroundTime += (processContainer[i].finishingTime - processContainer[i].A);
    }

    // Calculates the CPU utilisation
    double CPUUtilisation = totalAmountOfTimeUtilisingCPU / finalFinishingTime;

    // Calculates the IO utilisation
    double IOUtilisation = (double) TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED / finalFinishingTime;

    // Calculates the throughput
    double throughput =  100 * ((double) TOTAL_CREATED_PROCESSES/ finalFinishingTime); // Number of processes over the final finishing time times 100

    // Calculates the average turnaround time
    double averageTurnaroundTime = totalTurnaroundTime / TOTAL_CREATED_PROCESSES;

    // Calculates the average waiting time
    double averageWaitingTime = totalAmountOfTimeSpentWaiting / TOTAL_CREATED_PROCESSES;

    printf("Summary Data:\n");
    printf("\tFinishing time: %i\n", CURRENT_CYCLE - 1);
    printf("\tCPU Utilisation: %6f\n", CPUUtilisation);
    printf("\tI/O Utilisation: %6f\n", IOUtilisation);
    printf("\tThroughput: %6f processes per hundred cycles\n", throughput);
    printf("\tAverage turnaround time: %6f\n", averageTurnaroundTime);
    printf("\tAverage waiting time: %6f\n", averageWaitingTime);
} // End of the print summary data function

void resetAfterRun(struct Process processContainer[])
{
    CURRENT_CYCLE = 0;
    TOTAL_STARTED_PROCESSES = 0;
    TOTAL_FINISHED_PROCESSES = 0;
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0;
    IS_PROCESS_RUNNING = false;

    // readyQueue head & tail pointers
    readyHead = NULL;
    readyTail = NULL;
    readyProcessQueueSize = 0;

    // readySuspendedQueue head & tail pointers
    readySuspendedHead = NULL;
    readySuspendedTail = NULL;
    readySuspendedProcessQueueSize = 0;

    // blockedQueue head & tail pointers
    blockedHead = NULL;
    blockedTail = NULL;
    blockedProcessListSize = 0;

    runningProcess = NULL;

    uint32_t i = 0;
    FILE* randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        processContainer[i].status = 0;
        processContainer[i].nextInReadyQueue = NULL;
        processContainer[i].nextInReadySuspendedQueue = NULL;
        processContainer[i].nextInBlockedList = NULL;
        processContainer[i].finishingTime = -1;

        processContainer[i].currentCPUTimeRun = 0;
        processContainer[i].currentIOBlockedTime = 0;
        processContainer[i].currentWaitingTime = 0;

        processContainer[i].isFirstTimeRunning = false;

        processContainer[i].CPUBurst = randomOS(processContainer[i].B, randomNumberFile);
        processContainer[i].IOBurst = processContainer[i].M * processContainer[i].CPUBurst;

    }
    fclose(randomNumberFile);
} // End of reset after run function

/********************* END OF GLOBAL OUTPUT FUNCTIONS *********************************************************/

/* Actual scheduler simulators */
void simulateFirstComeFirstServe(uint8_t currentPassNumber, struct Process processContainer[],
                                 struct Process finishedProcessContainer[], FILE* randomFile)
{
//    if (CURRENT_CYCLE == 150) //TODO remove after
//    {
//        printf("Exiting due to retardedness\n"); //TODO remove after
//        exit(1);
//    }

    if ((IS_VERBOSE_MODE) && ((currentPassNumber) % 2 == 0))
    {
        // Prints out the state of each process during the current cycle
        printf("Before cycle\t%i:\t", CURRENT_CYCLE);
        int i = 0;
        for (; i < TOTAL_CREATED_PROCESSES; ++i)
        {
            switch (processContainer[i].status)
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
                    printf("running \t%i\t", processContainer[i].CPUBurst + 1);
                    break;
                case 3:
                    // Node is I/O blocked
                    printf("blocked \t%i\t", processContainer[i].IOBurst + 1);
                    break;
                case 4:
                    // Node is terminated
                    printf("terminated \t0\t");
                    break;
                default:
                    // Invalid node status, exiting now
                    fprintf(stderr, "Error: Invalid process status code, exiting now!\n");
                    exit(1);
            } // End of the per process status print statement
        } // End of the per line for loop
        printf("\n");
    }

    DoBlockedProcesses();
    DoRunningProcesses(finishedProcessContainer);
    // Checks whether the processes are all created, so it can skip creation if not required
    if (TOTAL_STARTED_PROCESSES != TOTAL_CREATED_PROCESSES)
    {
        // Not all processes created, goes into creation loop
        createProcesses(processContainer);
    }
    DoReadyProcesses(0, currentPassNumber, randomFile);

    incrementTimers(processContainer);

    ++CURRENT_CYCLE;
} // End of the simulate first come first serve function


/******************* START OF THE OUTPUT WRAPPER FOR EACH SCHEDULING ALGORITHM *********************************/

/**
 * Wrapper for the first come first servce scheduler algorithm
 */
void firstComeFirstServeWrapper(struct Process processContainer[])
{
    printf("######################### START OF FIRST COME FIRST SERVE #########################\n");

    printStart(processContainer);
    uint8_t currentPassNumber = 1;

    struct Process finishedProcessContainer[TOTAL_CREATED_PROCESSES];
    FILE* randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    // Runs this the first time in order to have the final output be available
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES)
        simulateFirstComeFirstServe(currentPassNumber, processContainer, finishedProcessContainer, randomNumberFile);

    fclose(randomNumberFile);

    printFinal(finishedProcessContainer);
    resetAfterRun(processContainer);
    printf("\n");

    ++currentPassNumber;

    if (IS_VERBOSE_MODE)
        printf("This detailed printout gives the state and remaining burst for each process\n");

    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES)
        simulateFirstComeFirstServe(currentPassNumber, processContainer, finishedProcessContainer, randomNumberFile);
    fclose(randomNumberFile);
    printf("The scheduling algorithm used was First Come First Served\n");
    printProcessSpecifics(processContainer);
    printSummaryData(processContainer);

    resetAfterRun(processContainer);        // Resets all values to initial conditions
    printf("######################### END OF FCFS #########################\n");
} // End of the first come first serve wrapper

void roundRobinWrapper(struct Process processContainer[])
{
    printf("######################### START OF ROUND ROBIN #########################\n");



    resetAfterRun(processContainer);        // Resets all values to initial conditions
    printf("######################### END OF ROUND ROBIN #########################\n");
} // End of the print round robin function


void uniprogrammedWrapper(struct Process processContainer[])
{
    printf("######################### START OF UNIPROGRAMMED #########################\n");



    resetAfterRun(processContainer);        // Resets all values to initial conditions
    printf("######################### END OF UNIPROGRAMMED #########################\n");
} // End of the print uniprogrammed function

void shortestJobFirstWrapper(struct Process processContainer[])
{
    printf("######################### START OF SHORTEST JOB FIRST #########################\n");



    resetAfterRun(processContainer);        // Resets all values to initial conditions
    printf("######################### END OF SHORTEST JOB FIRST #########################\n");
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

    uint32_t totalNumberOfProcessesToCreate;                    // Given as the first number in the mix
    fscanf(inputFile, "%i", &totalNumberOfProcessesToCreate);   // Reads in the indicator number for the mix

    randomNumberFile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    struct Process processContainer[totalNumberOfProcessesToCreate]; // Creates a container for all processes

    // Reads through the input, and creates all processes given, saving into an array
    uint32_t currentNumberOfMixesCreated = 0;
    for (; currentNumberOfMixesCreated < totalNumberOfProcessesToCreate; ++currentNumberOfMixesCreated)
    {
        uint32_t currentInputA;
        uint32_t currentInputB;
        uint32_t currentInputC;
        uint32_t currentInputM;

        // Ain't C cool, that you can read in something like this that scans in the job
        fscanf(inputFile, " %*c%i %i %i %i%*c", &currentInputA, &currentInputB, &currentInputC, &currentInputM);

        processContainer[currentNumberOfMixesCreated].A = currentInputA;
        processContainer[currentNumberOfMixesCreated].B = currentInputB;
        processContainer[currentNumberOfMixesCreated].C = currentInputC;
        processContainer[currentNumberOfMixesCreated].M = currentInputM;

        processContainer[currentNumberOfMixesCreated].processID = currentNumberOfMixesCreated;
        processContainer[currentNumberOfMixesCreated].status = 0;
        processContainer[currentNumberOfMixesCreated].finishingTime = -1;

        processContainer[currentNumberOfMixesCreated].currentCPUTimeRun = 0;
        processContainer[currentNumberOfMixesCreated].currentIOBlockedTime = 0;
        processContainer[currentNumberOfMixesCreated].currentWaitingTime = 0;

        processContainer[currentNumberOfMixesCreated].CPUBurst = randomOS(processContainer[currentNumberOfMixesCreated].B, randomNumberFile);
        processContainer[currentNumberOfMixesCreated].IOBurst = processContainer[currentNumberOfMixesCreated].M * processContainer[currentNumberOfMixesCreated].CPUBurst;

        processContainer[currentNumberOfMixesCreated].isFirstTimeRunning = false;

        processContainer[currentNumberOfMixesCreated].nextInBlockedList = NULL;
        processContainer[currentNumberOfMixesCreated].nextInReadyQueue = NULL;
        processContainer[currentNumberOfMixesCreated].nextInReadySuspendedQueue = NULL;
        ++TOTAL_CREATED_PROCESSES;
    }
    // All processes from mix instantiated
    fclose(inputFile);
    fclose(randomNumberFile);

    // First Come First Serve Run
    firstComeFirstServeWrapper(processContainer);

    // Round Robin Run
    roundRobinWrapper(processContainer);

    // Uniprogrammed Run
    uniprogrammedWrapper(processContainer);

    // Shortest Job First Run
    shortestJobFirstWrapper(processContainer);

    return EXIT_SUCCESS;
} // End of the main function