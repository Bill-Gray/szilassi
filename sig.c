// sp/sig.c  Tom Ace  crux@qnet.com
 

#include <signal.h>

static int interrupted;

static void sigintHandler()
{
   signal(SIGINT,sigintHandler);
   interrupted = 1;
}

void SPinitSig()
{
   signal(SIGINT,sigintHandler);
}

int SPsigInt()
{
   int    toReturn = interrupted;
   interrupted = 0;
   return toReturn;
}
