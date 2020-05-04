#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

int main(int argc, char* argv[]) {

    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }
   /* if(signal(SIGALRM , alarmHandler)==SIG_ERR) {
        perror("smash error: failed to set alarm handler");
    }*/

    struct sigaction new_action;

    /* Set up the structure to specify the new action. */

    new_action.sa_handler = alarmHandler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;
    if(sigaction(SIGALRM , &new_action, nullptr)<0)
        perror("smash error: failed to set alarm handler");


    //TODO: setup sig alarm handler

    SmallShell& smash = SmallShell::getInstance();
    while(true) {
            std::cout << smash.getPrompt(); // TODO: change this (why?)
            std::string cmd_line;
            std::getline(std::cin, cmd_line);
            try {
                smash.executeCommand(cmd_line.c_str());
            }
            catch (EmptyCommandException){}
            catch (string& a){
                break;
            }
            catch (...){
            }
    }
    return 0;
}
