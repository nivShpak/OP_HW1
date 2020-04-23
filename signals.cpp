#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
	cout<<"smash: got ctrl-Z"<<endl;
    __pid_t front_pid = getFrontPid();
    if (front_pid!=0) {
        kill(getFrontPid(), SIGSTOP);
        cout<<"smash: process "<< front_pid <<" was stoped"<<endl;
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout<<"smash: got ctrl-C"<<endl;
    __pid_t front_pid = getFrontPid();
    if (front_pid!=0) {
        kill(getFrontPid(), SIGKILL);
        cout<<"smash: process "<< front_pid <<" was killed"<<endl;
    }
}
/*
void ctrlZHandlerPipe(int sig_num) {
    // TODO: Add your implementation
    cout<<"smash: got ctrl-Z"<<endl;
    __pid_t pipe_pid = getPipePid();
    if (pipe_pid!=0) {
        kill(pipe_pid,SIGSTOP);
        kill(getFrontPid(),SIGSTOP);
        killpg(getPipePidGrp(), SIGSTOP);
        cout<<"smash: process "<< pipe_pid <<" was stoped"<<endl;
    }
}

void ctrlCHandlerPipe(int sig_num) {
    // TODO: Add your implementation
    cout<<"smash: got ctrl-C"<<endl;
    __pid_t pipe_pid = getPipePid();
    if (pipe_pid!=0) {
        killpg(getPipePidGrp(), SIGKILL);
        cout<<"smash: process "<< pipe_pid <<" was killed"<<endl;
    }
}
*/

void alarmHandler(int sig_num) {
    // TODO: Add your implementation


    //search for endTimeOut and destroy
    time_t currentTime;
    currentTime = time(nullptr);
    TimeOutList *tOList = getSmashGlob()->GetTimeOutList();
    TimeOutList::TimeOutEntry *tOentry = tOList->GetTOFinishNow(currentTime);
    do {
        if (tOentry == nullptr) {
            //cout<<"error!! smash: got an alarm and its null"<<endl;
            return;
        }
        pid_t pid = tOentry->GetTimeOutPid();
        if (getSmashPid() != tOentry->GetTimeOutPid()) {
            kill(pid, SIGKILL);
            tOList->removeTimeOutById(tOentry->GetTimeOutId());
            cout << "smash: got an alarm" << endl;
            cout << "smash:" << tOentry->GetTimeOutCmdLine() << " timed out!" << endl;
        }
        if(alarm(0)<2)
            tOentry=tOList->GetTOFinishNow(currentTime);//how to get the next alarm?
        else
            break;
    }
    while (tOentry!= nullptr);


}

