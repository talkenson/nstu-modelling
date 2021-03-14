#include <stdio.h>
#include <iostream>
#include <locale.h>

using namespace std;

class Client {
    class Task {
    public:
        int created = 0;
        int amount = 0;
        int start = 0;
        int end = 0;
    };

public:
    // static
    int timeWait = 0;
    int timeIdle = 0;
    int timeBusy = 0;
    int taskCount = 0;
    int tsIdledFrom = 0;
    int taskRequests = 0;
    int taskFinished = 0;
    int maxQueue = 0;
    int timeInProgress = 0;
    int _debugAmountSum = 0;

    // temporaries
    int tStart = 0; // ts of start
    int tEst = 0; // time amount
    int tRec = 0; // ts of recv
    int tEnd = 0; // ts of end

    Task *queue = nullptr;

    void push(int currentTS) {
        this->taskRequests += 1;
        if (this->maxQueue > 0 && this->taskCount >= this->maxQueue)
            return;
        this->taskCount += 1;
        Task *newQ = new Task[this->taskCount];
        for (int i = 0; i < this->taskCount - 1; i++) {
            newQ[i] = this->queue[i];
        }
        newQ[this->taskCount - 1].start = -1;
        newQ[this->taskCount - 1].amount = 1 + rand() % 15;
        this->_debugAmountSum += newQ[this->taskCount - 1].amount;
        newQ[this->taskCount - 1].created = currentTS;
        newQ[this->taskCount - 1].end = -1;

        if (this->taskCount == 1) {
            this->timeIdle += currentTS - this->tsIdledFrom;
            newQ[0].start = currentTS;
            newQ[0].end = newQ[0].start + newQ[0].amount;
        }
        delete[] this->queue;
        this->queue = newQ;
    }

    void pop(int currentTS) {
        this->timeInProgress += currentTS - this->queue[0].created;

        this->taskCount -= 1;
        Task *newQ = new Task[this->taskCount];
        for (int i = 0; i < this->taskCount; i++) {
            newQ[i] = this->queue[i + 1];
        }

        delete[] this->queue;
        this->queue = newQ;

        if (this->taskCount == 0) {
            this->tsIdledFrom = currentTS;
        } else {
            this->queue[0].start = currentTS;
            this->queue[0].end = this->queue[0].amount + this->queue[0].start;
            this->timeWait += this->queue[0].start - this->queue[0].created;
        }
        this->taskFinished += 1;
    }

    void checkTick(int currentTS) {
        if (this->queue && this->queue[0].end == currentTS) {
            this->pop(currentTS);
        }
    }

    int getEstimate(int currentTS) {
        int est = 0;
        for (int i = 0; i < this->taskCount; i++) {
            est += this->queue[i].amount;
            if (i == 0) {
                est -= currentTS - (this->queue[i].start);
            }
        }
        return est;
    }

    void getStats() {
        printf(""
               "Task Wait time: %d\n"
               "Machine Idle time: %d\n"
               "Tasks wanted: %d\n"
               "Tasks Processed: %d\n"
               "Tasks declined (DOS Status): %d\n"
               "Average Wait per Task: %f\n",
               this->timeWait, this->timeIdle, this->taskRequests, this->taskFinished,
               this->taskRequests - this->taskFinished, (1. * this->timeWait / this->taskFinished));
    }

    int getCount() {
        return this->taskCount;
    }

    int getFinished() {
        return this->taskFinished;
    }

    int getRequested() {
        return this->taskRequests;
    }

    int getIdleTime() {
        return this->timeIdle;
    }

    int getWaitTime() {
        return this->timeWait;
    }

    int getBusy() {
        return this->timeInProgress;
    }

    Client() = default;

    Client(int maxQueue) {
        this->maxQueue = maxQueue;
    };

};

Client *c1 = new Client(2), *c2 = new Client(); // компьютеры
long int tasksFinished1 = 0, tasksFinished2 = 0, totalTimeSpent = 0, idleTime1, idleTime2, waitTime1, waitTime2, inSys1, inSys2;
long int sumBusy1, sumBusy2;

int main() {
    setlocale(0, "");
    for (int epoch = 0; epoch < 100; epoch++) {
        int i = 0, t = 0, g = 1 + rand() % 13;
        for (; t < 1000 || (c1->getCount() + c2->getCount()) > 0; g--, i++) {
            if (g == 0 && t < 1000) { // pcBusy recv
                if (t % 2 == 0) {
                    c1->push(i);
                } else {
                    c2->push(i);
                }

                g = 1 + rand() % 13;
                t++;
            }
            c1->checkTick(i);
            c2->checkTick(i);
        }
        tasksFinished1 += c1->getFinished();
        tasksFinished2 += c2->getFinished();
        idleTime1 += c1->getIdleTime();
        idleTime2 += c2->getIdleTime();
        waitTime1 += c1->getWaitTime();
        waitTime2 += c2->getWaitTime();
        inSys1 += c1->getBusy();
        inSys2 += c2->getBusy();
        sumBusy1 += c1->_debugAmountSum;
        sumBusy2 += c2->_debugAmountSum;
        totalTimeSpent += i;
        printf("\n\n----------------\nEPOCH %d: MADE 1000 FOR %d\n", epoch, i);
        printf("Host 1 Statistics:\n");
        c1->getStats();
        printf("\n-----\nHost 2 Statistics:\n");
        c2->getStats();
        delete c1;
        delete c2;
        c1 = new Client(2);
        c2 = new Client();
    }
    printf("\n\n----------------\nGLOBAL STATISTICS\n");
    printf(""
           "Mean Task Wait time: %f\n"
           "Mean Task In-System time: %f\n"
           "Host 1 Idle time: %f%%\n"
           "Host 2 Idle time: %f%%\n"
           "Tasks Processed overall: %ld\n"
           "Mean Tasks Processed: %f\n",
           (float) (waitTime1 + waitTime2) / (float) (tasksFinished1 + tasksFinished2),
           (float) (inSys1 + inSys2) / (float) (tasksFinished1 + tasksFinished2),
           ((float) idleTime1 / (float) totalTimeSpent) * 100,
           ((float) idleTime2 / (float) totalTimeSpent) * 100, (tasksFinished1 + tasksFinished2),
           (float) (tasksFinished1 + tasksFinished2) / 100);
    printf("\nChecking: %f + %f ~= %f\n\n", (float) (waitTime1 + waitTime2) / (float) (tasksFinished1 + tasksFinished2),
           (float) (sumBusy1 + sumBusy2) / (float) (tasksFinished1 + tasksFinished2),
           (float) (inSys1 + inSys2) / (float) (tasksFinished1 + tasksFinished2));
    return 0;
}
