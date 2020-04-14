#ifndef SMASH__SIGNALS_H_
#define SMASH__SIGNALS_H_

#include <signal.h>

#define MIN_SIG 1
#define MAX_SIG 31


void ctrlZHandler(int sig_num);
void ctrlCHandler(int sig_num);
void alarmHandler(int sig_num);

bool isLegalSignal(int s){
    if (s>=MIN_SIG & s<=MAX_SIG) return true;
    return false;
}






#endif //SMASH__SIGNALS_H_
