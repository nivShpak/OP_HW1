#include <iostream>
#include <signal.h>
#include "Commands.h"
#include <sys/types.h>
#include <unistd.h>

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
	cout<<"smash: got ctrl-Z"<<endl;
    __pid_t front_pid = getFrontPid();
    if (front_pid!=0) {
        kill(getFrontPid()*(-1), SIGSTOP);
        cout<<"smash: process "<< front_pid <<" was stoped"<<endl;
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    cout<<"smash: got ctrl-C"<<endl;
    __pid_t front_pid = getFrontPid();
    if (front_pid!=0) {
        kill(getFrontPid()*(-1), SIGKILL);
        cout<<"smash: process "<< front_pid <<" was killed"<<endl;
    }
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation

    //search for endTimeOut and destroy
    time_t currentTime;
    currentTime = time(nullptr);
    SmallShell& smashGlob = SmallShell::getInstance();
    TimeOutList *tOList = smashGlob.GetTimeOutList();
    TimeOutList::TimeOutEntry *tOentry = tOList->GetTOFinishNow(currentTime);
        if (tOentry == nullptr) {
            //the process is already dead
            tOList->SetAlarmTONext(currentTime);
            return;
        }
        int fdScreen=getFdScreen();
        pid_t pid = tOentry->GetTimeOutPid();
        if (getSmashPid() != pid) {
            if(pid!=0&&is_pid_running(pid)) {
                int check = kill(pid*(-1), SIGKILL);//pid=0 is a built in command
                if(check==-1)
                    perror("smash error: kill failed");
            }
            if(fdScreen!=-1){//if its timeout front
                //stdout back to screen for alarm
                int fdCheck = dup2(fdScreen, 1);
                if (fdCheck == -1) {
                    perror("smash error: dup2 failed");
                }
            }
            cout << "smash: got an alarm" << endl;
            cout << tOentry->GetTimeOutCmdLine() << " timed out!" << endl;
            tOList->removeTimeOutById(tOentry->GetTimeOutId());
        }

        tOList->SetAlarmTONext(currentTime);



}

