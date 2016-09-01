//
//  hexapod.cpp
//
//  Created by Martin Lane-Smith on 7/2/14.
//  Copyright (c) 2014 Martin Lane-Smith. All rights reserved.
//
// Starts all robot processes

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>

#include <string>

#include "hexapod.h"

#include "lua.hpp"

#include "mraa.h"

#include "serial/socket/ps_socket_server.hpp"
#include "syslog/linux/ps_syslog_linux.hpp"

#include "arbotix/arbotix.hpp"
#include "autopilot/autopilot.hpp"
#include "behavior/behavior.hpp"
#include "scanner/scanner.hpp"
#include "dancer/dancer.hpp"
#include "i2c_task/i2c_task.hpp"
#include "navigator/navigator.hpp"
#include "responder/responder.hpp"

#include "main_debug.h"

using namespace std;

FILE *mainDebugFile;

#define PROCESS_NAME "hexapod"

bool initComplete = false;

int *pidof (char *pname);
void KillAllOthers(string name);
void fatal_error_signal (int sig);
void SIGPIPE_signal (int sig){}

int main()
{
	int reply;
	string initFail = "";	//fail flag

	//set up logging
	mainDebugFile = fopen_logfile("main");

	//kill any running services
	KillAllOthers(PROCESS_NAME);

	//initialize mraa
	reply = mraa_init();

	if (reply != MRAA_SUCCESS)
	{
		ERRORPRINT("mraa_init() fail: %i", reply);
		initFail = "mraa";
	}
	else {
		DEBUGPRINT("mraa init() OK");
	}

	const struct sigaction sa {SIGPIPE_signal, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

	//plumbing

	//start the log
	ps_syslog_linux syslog;

	//start the socket server
	ps_socket_server appConnection(4000, 5000);

	ps_register_event_names(eventNames, EVENT_COUNT);
	ps_register_condition_names(conditionNames, CONDITIONS_COUNT);
	ps_register_topic_names(psTopicNames, PS_TOPIC_COUNT);

	//init subsystems

	//start arbotix threads
	if (Arbotix_INIT() != 0)
	{
		ERRORPRINT("ArbotixInit() fail");
		initFail = "arbotix";
	}
	else {
		DEBUGPRINT("ArbotixInit() OK");
	}

	//start LIDAR threads
	if (Scanner_INIT() != 0)
	{
		ERRORPRINT("ScannerInit() fail");
		initFail = "lidar";
	}
	else {
		DEBUGPRINT("ScannerInit() OK");
	}

	//start autopilot threads
	if (Autopilot_INIT() != 0)
	{
		ERRORPRINT("AutopilotInit() fail");
		initFail = "pilot";
	}
	else {
		DEBUGPRINT("AutopilotInit() OK");
	}

	//start behavior tree threads
	if (Behavior_INIT() != 0)
	{
		ERRORPRINT("BehaviorInit() fail");
		initFail = "behavior";
	}
	else {
		DEBUGPRINT("BehaviorInit OK");
	}

	//start Dancer threads
	if (Dancer_INIT() != 0)
	{
		ERRORPRINT("DancerInit() fail");
		initFail = "dancer";
	}
	else {
		DEBUGPRINT("DancerInit() OK");
	}

	//start i2c threads
	if (I2C_INIT() != 0)
	{
		ERRORPRINT("I2CInit() fail");
		initFail = "i2c";
	}
	else {
		DEBUGPRINT("I2CInit() OK");
	}

	//start navigator threads
	if (Navigator_INIT() != 0)
	{
		ERRORPRINT("NavigatorInit() fail");
		initFail = "navigator";
	}
	else {
		DEBUGPRINT("NavigatorInit() OK");
	}

	if (Responder_INIT() != 0)
	{
		ERRORPRINT( "ResponderInit() fail");
		initFail = "responder";
	}
	else {
		DEBUGPRINT("ResponderInit() OK");
	}

	initComplete = true;

	if (initFail.length() > 0)
	{
		ps_set_condition(INIT_ERROR);
		LogError("Hexapod Init Fail '%s'", initFail.c_str());
		sleep(10);
		return -1;
	}

	LogRoutine("Init complete");

	if (getppid() == 1)
	{
		//child of init/systemd

		//close stdio
		fclose(stdout);
		fclose(stderr);
		stdout = fopen("/dev/null", "w");
		stderr = fopen("/dev/null", "w");
	}

	signal(SIGILL, fatal_error_signal);
	signal(SIGABRT, fatal_error_signal);
	signal(SIGIOT, fatal_error_signal);
	signal(SIGBUS, fatal_error_signal);
	signal(SIGFPE, fatal_error_signal);
	signal(SIGSEGV, fatal_error_signal);
	signal(SIGTERM, fatal_error_signal);
	signal(SIGCHLD, fatal_error_signal);
	signal(SIGSYS, fatal_error_signal);
	signal(SIGCHLD, fatal_error_signal);

	while(1)
	{
		psMessage_t msg;
		psInitPublish(msg, TICK_1S);
		NewBrokerMessage(msg);
		sleep(1);
	}

	return 0;
}

//other signals
volatile sig_atomic_t fatal_error_in_progress = 0;
void fatal_error_signal (int sig)
{
	/* Since this handler is established for more than one kind of signal, it might still get invoked recursively by delivery of some other kind of signal. Use a static variable to keep track of that. */
	if (fatal_error_in_progress) raise (sig);

	fatal_error_in_progress = 1;

	LogError("Signal %i raised", sig);
	sleep(1);	//let there be printing

	/* Now re-raise the signal. We reactivate the signal�s default handling, which is to terminate the process. We could just call exit or abort,
but re-raising the signal sets the return status
from the process correctly. */
	signal (sig, SIG_DFL);
	raise (sig);
}

//helper functions to find any existing processes of a given name

/* checks if the string is purely an integer
 * we can do it with `strtol' also
 */
int check_if_number (char *str)
{
  int i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (!isdigit (str[i]))
    {
      return 0;
    }
  }
  return 1;
}

#define MAX_BUF 1024
#define PID_LIST_BLOCK 32

//returns a list of up to 32 pids of processes matching the provided name
int *pidof (string pname)
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  int *pidlist, pidlist_index = 0, pidlist_realloc_count = 1;
  char path[MAX_BUF], read_buf[MAX_BUF];

  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
    perror ("Fail");
    return NULL;
  }

  pidlist = (int*) malloc (sizeof (int) * PID_LIST_BLOCK);
  if (pidlist == NULL)
  {
    return NULL;
  }

  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
      strcpy (path, "/proc/");
      strcat (path, entry->d_name);
      strcat (path, "/comm");

      /* A file may not exist, it may have been removed.
       * due to termination of the process. Actually we need to
       * make sure the error is actually file does not exist to
       * be accurate.
       */
      fp = fopen (path, "r");
      if (fp != NULL)
      {
        fscanf (fp, "%s", read_buf);
        if (pname.compare(read_buf) == 0)
        {
          /* add to list and expand list if needed */
          pidlist[pidlist_index++] = atoi (entry->d_name);
          if (pidlist_index == PID_LIST_BLOCK * pidlist_realloc_count)
          {
            pidlist_realloc_count++;
            pidlist = (int*) realloc (pidlist, sizeof (int) * PID_LIST_BLOCK * pidlist_realloc_count); //Error check
            if (pidlist == NULL)
            {
              return NULL;
            }
          }
        }
        fclose (fp);
      }
    }
  }

  closedir (dirp);
  pidlist[pidlist_index] = -1; /* indicates end of list */
  return pidlist;
}

void KillAllOthers(string name)
{
	//kill any others of this name
	int *pidlist = pidof(name);	//list of pids
	int *pids = pidlist;
	//kill each pid in list (except me)
	while (*pids != -1) {
		if (*pids != getpid())	//don't kill me
		{
			kill(*pids, SIGTERM);
			DEBUGPRINT("Killed pid %i (%s)", *pids, name.c_str());
		}
		pids++;
	}
	free(pidlist);
}


