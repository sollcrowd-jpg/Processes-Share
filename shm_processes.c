#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_ITERATIONS 25

void ParentProcess(int []); // Parent process function (Dear Old Dad)
void ChildProcess(int []);  // Child process function (Poor Student)

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create a shared memory segment with space for two integers: BankAccount and Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    // Attach the shared memory segment to the address space
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }

    // Initialize BankAccount and Turn to 0
    ShmPTR[0] = 0;  // BankAccount
    ShmPTR[1] = 0;  // Turn (0 means Parent's turn, 1 means Child's turn)

    // Fork to create a child process
    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } else if (pid == 0) {
        // In the child process (Poor Student)
        ChildProcess(ShmPTR);
        exit(0);
    } else {
        // In the parent process (Dear Old Dad)
        ParentProcess(ShmPTR);
    }

    // Wait for the child process to finish
    wait(&status);

    // Detach and remove shared memory
    shmdt((void *) ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);

    return 0;
}

void ParentProcess(int SharedMem[]) {
    int account;
    int i;  // Declare variable outside the for loop
    srand(time(NULL));  // Seed random number generator

    for (i = 0; i < NUM_ITERATIONS; i++) {
        sleep(rand() % 6);  // Sleep for a random time between 0-5 seconds

        // Wait for Turn to be 0 (Parent's turn)
        while (SharedMem[1] != 0);

        // Copy the current value of BankAccount to a local variable
        account = SharedMem[0];

        // If the account balance is less than or equal to 100, attempt to deposit
        if (account <= 100) {
            int balance = rand() % 101;  // Random deposit amount between 0-100

            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        // Write the updated account balance back to shared memory
        SharedMem[0] = account;

        // Set Turn to 1 (Child's turn)
        SharedMem[1] = 1;
    }
}

void ChildProcess(int SharedMem[]) {
    int account;
    int i;  // Declare variable outside the for loop
    srand(time(NULL) + getpid());  // Seed random number generator

    for (i = 0; i < NUM_ITERATIONS; i++) {
        sleep(rand() % 6);  // Sleep for a random time between 0-5 seconds

        // Wait for Turn to be 1 (Child's turn)
        while (SharedMem[1] != 1);

        // Copy the current value of BankAccount to a local variable
        account = SharedMem[0];

        // Generate a random withdrawal amount between 0-50
        int balance = rand() % 51;
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        // Write the updated account balance back to shared memory
        SharedMem[0] = account;

        // Set Turn to 0 (Parent's turn)
        SharedMem[1] = 0;
    }
}
