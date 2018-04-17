#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#define check(exp, msg) if(exp) {} else {\
    printf(1, "%s:%d check (" #exp ") failed: %s\n", __FILE__, __LINE__, msg);\
    exit();}
#define PROC 3

void spin()
{
    int i;
    int j = 0;
    for(i = 0; i < 100000000000000000; ++i)
    { 
        j++;
    }
}


    int
main(int argc, char *argv[])
{
    struct pstat st;

    int i = 0;
    int ticket = 10;
    int pid[NPROC];
    printf(1,"Spinning...\n");
    while(i < PROC)
    {
        pid[i] = fork();
        if(pid[i] == 0)
        {
            settickets(ticket);
            spin();
            exit();
        }
        i++;
        ticket += 10;
    }

    for(int j = 0; j < 100; j ++){
        sleep(100);
        //spin();
        check(getpinfo(&st) == 0, "getpinfo");

        printf(1, "%d %d %d %d\n", j, st.ticks[3],st.ticks[4],st.ticks[5]);


    }

    for(i = 0; i < PROC; i++)
    {
        kill(pid[i]);
    }
    while (wait() > 0);
    exit();

}

