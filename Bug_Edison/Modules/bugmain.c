//
//  overmind.c
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

#include <softwareProfile.h>

#include "lua.h"
#include "mraa.h"

#include "pubsubdata.h"
#include "pubsub/pubsub.h"
#include "syslog/syslog.h"

#include "agent/agent.h"
#include "arbotix/arbotix.h"
#include "autopilot/autopilot.h"
#include "behavior/behavior.h"
#include "pubsub/broker.h"
#include "dancer/dancer.h"
#include "gripper/gripper.h"
#include "i2c_task/i2c_task.h"
#include "lidar/lidar.h"
#include "navigator/navigator.h"
#include "pubsub/responder.h"

FILE *mainDebugFile;

#ifdef MAIN_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(mainDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(mainDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf(__VA_ARGS__);tfprintf(mainDebugFile, __VA_ARGS__);

#define PROCESS_NAME "overmind"

bool initComplete = false;

int *pidof (char *pname);
void KillAllOthers(char * name);
void fatal_error_signal (int sig);

int main(int argc, const char * argv[])
{
	int reply;
	char * initFail = "";

	mainDebugFile = fopen_logfile("main");
    if (mainDebugFile == NULL)
    {
        fprintf (stderr, "Couldn’t open file main.log; %s\n", strerror (errno));
        return -1;
    }

	DEBUGPRINT("main() Logfile opened\n");

	KillAllOthers(PROCESS_NAME);

	reply = mraa_init();

	if (reply != MRAA_SUCCESS && reply != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)
	{
		ERRORPRINT("mraa_init() fail: %i\n", reply);
		initFail = "mraa";
	}
	else {
		DEBUGPRINT("mraa init() OK\n");
	}

//	LSM303StartCalibrate();

	//init subsystems
	//start broker thread
	if (Broker_INIT() != 0)
	{
		ERRORPRINT( "BrokerInit() fail\n");
		initFail = "broker";
	}
	else {
		DEBUGPRINT("BrokerInit() OK\n");
	}

	//broker queue
	if (BrokerQueueInit(100) != 0)
	{
		ERRORPRINT("BrokerQueueInit() fail\n");
		initFail = "brokerQ";
	}
	else {
		DEBUGPRINT("BrokerQueueInit() OK\n");
	}
	
	//start syslog
	if (SysLog_INIT() != 0)
	{
		ERRORPRINT("SysLogInit() fail\n");
		initFail = "syslog";
	}
	else {
		DEBUGPRINT("SysLogInit() OK\n");
	}

	//start agent threads
	if (Agent_INIT() != 0)
	{
		ERRORPRINT("AgentInit() fail\n");
		initFail = "agent";
	}
	else {
		DEBUGPRINT("AgentInit() OK\n");
	}

	//start arbotix threads
	if (Arbotix_INIT() != 0)
	{
		ERRORPRINT("ArbotixInit() fail\n");
		initFail = "arbotix";
	}
	else {
		DEBUGPRINT("ArbotixInit() OK\n");
	}

	//start autopilot threads
	if (Autopilot_INIT() != 0)
	{
		ERRORPRINT("AutopilotInit() fail\n");
		initFail = "pilot";
	}
	else {
		DEBUGPRINT("AutopilotInit() OK\n");
	}

	//start behavior tree threads
	if (Behavior_INIT() != 0)
	{
		ERRORPRINT("BehaviorInit() fail\n");
		initFail = "behavior";
	}
	else {
		DEBUGPRINT("BehaviorInit OK\n");
	}

	//start Dancer threads
	if (Dancer_INIT() != 0)
	{
		ERRORPRINT("DancerInit()\n fail");
		initFail = "dancer";
	}
	else {
		DEBUGPRINT("DancerInit() OK\n");
	}

	//start Gripper threads
	if (Gripper_INIT() != 0)
	{
		ERRORPRINT("GripperInit()\n fail");
		initFail = "gripper";
	}
	else {
		DEBUGPRINT("GripperInit() OK\n");
	}

	//start i2c threads
	if (I2C_INIT() != 0)
	{
		ERRORPRINT("I2CInit() fail\n");
		initFail = "i2c";
	}
	else {
		DEBUGPRINT("I2CInit() OK\n");
	}

	//start Lidar threads
	if (Lidar_INIT() != 0)
	{
		ERRORPRINT("LidarInit()\n");
		initFail = "RP_LIDAR";
	}
	else {
		DEBUGPRINT("LidarInit() OK\n");
	}

	//start navigator threads
	if (Navigator_INIT() != 0)
	{
		ERRORPRINT("NavigatorInit() fail\n");
		initFail = "navigator";
	}
	else {
		DEBUGPRINT("NavigatorInit() OK\n");
	}

	if (Responder_INIT() != 0)
	{
		ERRORPRINT( "ResponderInit() fail\n");
		initFail = "responder";
	}
	else {
		DEBUGPRINT("ResponderInit() OK\n");
	}

	//Notifications
	reply=NotificationsInit();
	if (reply != 0)
	{
		ERRORPRINT("NotificationsInit() fail\n");
		initFail = "responder";
	}
	else {
		DEBUGPRINT("NotificationsInit() OK\n");
	}

	initComplete = true;

	if (strlen(initFail) > 0)
	{
		SetCondition(INIT_ERROR);
		LogError("Hexapod Init Fail '%s'\n", initFail);
		SetCondition(INIT_ERROR);
		sleep(5);
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
		msg.tickPayload.systemPowerState = POWER_ACTIVE;
		NewBrokerMessage(&msg);

		sleep(1);
	}

	return 0;
}


//other signals
volatile sig_atomic_t fatal_error_in_progress = 0;
void fatal_error_signal (int sig)
{
	/* Since this handler is established for more than one kind of signal, it might still get invoked recursively by delivery of some other kind of signal. Use a static variable to keep track of that. */
	if (fatal_error_in_progress)
		raise (sig);
	fatal_error_in_progress = 1;

	LogError("Signal %i raised", sig);
	sleep(1);	//let there be printing

	/* Now re-raise the signal. We reactivate the signal�s default handling, which is to terminate the process. We could just call exit or abort,
but re-raising the signal sets the return status
from the process correctly. */
	signal (sig, SIG_DFL);
	raise (sig);
}

//functions to find any existing processes of a given name

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
int *pidof (char *pname)
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

  pidlist = malloc (sizeof (int) * PID_LIST_BLOCK);
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
        if (strcmp (read_buf, pname) == 0)
        {
          /* add to list and expand list if needed */
          pidlist[pidlist_index++] = atoi (entry->d_name);
          if (pidlist_index == PID_LIST_BLOCK * pidlist_realloc_count)
          {
            pidlist_realloc_count++;
            pidlist = realloc (pidlist, sizeof (int) * PID_LIST_BLOCK * pidlist_realloc_count); //Error check todo
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

void KillAllOthers(char * name)
{
	//kill any others of this name
	int *pidlist = pidof(name);	//list of pids
	int *pids = pidlist;
	//kill each pid in list (except me)
	while (*pids != -1) {
		if (*pids != getpid())	//don't kill me
		{
			kill(*pids, SIGTERM);
			DEBUGPRINT("Killed pid %i (%s)\n", *pids, name);
		}
		pids++;
	}
	free(pidlist);
}
