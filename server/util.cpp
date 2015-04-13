//SECURITY WARNING
//Chod prec uboha dusa. temne su miesta kam vchadzas
#include <cstdio>
#include <ctime>
#include <cstring>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <sstream>
using namespace std;

#include "util.h"


static void (*cleanupFunkcia)();


static pid_t hlavnyProces = 0;
static volatile sig_atomic_t praveUkoncujem = 0;


static void shutdownHandler(int signum) {
  // TODO: overit, ci je signum masked, aby nemohol nastat pocas vykonavania
  // handlera (inak moze vzniknut race condition, aj ked to asi moc nevadi...)
  if (getpid() == hlavnyProces && !praveUkoncujem) {
    praveUkoncujem = 1;
    log("dostal som ukoncovaci signal %d", signum);
    if (cleanupFunkcia) {
      cleanupFunkcia();
    }
  }

  signal(signum, SIG_DFL);
  raise(signum);
}


static void sigchldHandler(int signum) {
  // handler na reapovanie zombie procesov:
  // jednoduchsie by bolo pouzit SA_NOCLDWAIT, ale potom system() nedokaze
  // zistit exit codes. handlovat SIGCHLD je OK, lebo system() pocas svojho
  // behu SIGCHLD blokuje, takze sa nestane, ze by sme exitcode odchytili
  // odtialto pred tym, ako sa k nemu dostane on, apod.
  int pid, status;
  while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (WIFSIGNALED(status)) {
      if (WTERMSIG(status) == SIGTERM) continue;   // toho som asi zabil ja
      log("proces %d umrel na: %s", (int)pid, strsignal(WTERMSIG(status)));
    }
  }
}


void inicializujSignaly(void (*_cleanupFunkcia)()) {
  hlavnyProces = getpid();   // vo forkoch nechceme volat cleanup funkciu
  cleanupFunkcia = _cleanupFunkcia;
  signal(SIGCHLD, sigchldHandler);
  signal(SIGINT, shutdownHandler);
  signal(SIGTERM, shutdownHandler);
  signal(SIGHUP, shutdownHandler);
  signal(SIGPIPE, SIG_IGN);
}


#ifndef NELOGUJ
void logheader() {
  struct timeval stv;
  gettimeofday(&stv, NULL);
  struct tm *stm = localtime(&stv.tv_sec);
  if(stm == NULL) return;
  fprintf(stderr, "[%02d:%02d:%02d.%03ld] ", stm->tm_hour, stm->tm_min, stm->tm_sec, stv.tv_usec/1000);
}
#endif


bool jeAdresar(string filename) {
  struct stat st;
  if (stat(filename.c_str(), &st)) return false;
  return S_ISDIR(st.st_mode);
}


bool jeSubor(string filename) {
  struct stat st;
  if (stat(filename.c_str(), &st)) return false;
  return S_ISREG(st.st_mode);
}


long long gettime() {
  struct timeval tim;
  gettimeofday(&tim, NULL);
  return tim.tv_sec*1000LL + tim.tv_usec/1000LL;
}


string itos(int i) {
  stringstream ss;
  ss << i;
  return ss.str();
}
