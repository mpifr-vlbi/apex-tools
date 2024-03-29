#include <ncurses.h>
#include <signal.h>
#include <math.h>
#include <sys/types.h>
#include <stdlib.h>
#include "mparm.h"
#include "dpi.h"
#include "../include/params.h"
#include "../include/fs_types.h"
#include "../include/fscom.h"

extern struct fscom *shm_addr;

main()
{
  int it[6], iyear, isleep;
  int i;

  int m5init();
  int mout5();
  void die();
  unsigned rte_sleep();

  setup_ids();

/*  First check to see if the field system is running */

  if (nsem_test(NSEMNAME) != 1) {
    printf("Field System not running\n");
    exit(0);
  }

  initscr();
  signal(SIGINT, die);
  cbreak();
  noecho ();
  nodelay(stdscr, TRUE);

  curs_set(0);
  clear();
  curs_set(0);
  refresh();

  m5init();

/*  Initialize the display window */

  while(1) {

    while(ERR!=getch())
      ;

    mout5();
    move(ROW1,COL1+12);
    refresh();
    rte_time(it,&iyear);
    isleep=100-it[0];
    isleep=isleep>100?100:isleep;
    isleep=isleep<1?100:isleep;
    rte_sleep((unsigned) isleep);
    if (nsem_test(NSEMNAME) != 1) {
      printf("Field System terminated\n");
      die();
      exit(0);
    }
  }

}  /* end main of monit5 */
