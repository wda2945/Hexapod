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
#define DEBUGPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(mainDebugFile, __VA_ARGS__);fflush(mainDebugFile);
#else
#define DEBUGPRINT(...) fprintf(mainDebugFile, __VA_ARGS__);fflush(mainDebugFile);
#endif

#define ERRORPRINT(...) fprintf(stdout, __VA_ARGS__);fprintf(mainDebugFile, __VA_ARGS__);fflush(mainDebugFile);

#define PROCESS_NAME "overmind"

bool initComplete = false;

int *pidof (char *pname);

//SIGHUP signal used to trigger reload of edited lua scripts
void SIGHUPhandler(int sig);
int SIGHUPflag = 0;

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

	//kill any others of this name
	int *pidlist = pidof(PROCESS_NAME);	//list of pids
	int *pids = pidlist;
	//kill each pid in list (except me)
	while (*pids != -1) {
		if (*pids != getpid())	//don't kill me
		{
			kill(*pids, SIGTERM);
			DEBUGPRINT("Killed pid %i (%s)\n", *pids, PROCESS_NAME);
		}
		pids++;
	}
	free(pidlist);

	reply = mraa_init();

	if (reply != MRAA_SUCCESS && reply != MRAA_ERROR_PLATFORM_ALREADY_INITIALISED)
	{
		ERRORPRINT("mraa_init() fail: %i\n", reply);
		initFail = "mraa";
	}
	else DEBUGPRINT("mraa init() OK\n");

//	LSM303StartCalibrate();

	//init subsystems
	//broker queue
	if (BrokerQueueInit(100) != 0)
	{
		ERRORPRINT("BrokerQueueInit() fail\n");
		initFail = "brokerQ";
	}
	else DEBUGPRINT("BrokerQueueInit() OK\n");
	
	//start syslog
	if (SysLog_INIT() != 0)
	{
		ERRORPRINT("SysLogInit() fail\n");
		initFail = "syslog";
	}
	else DEBUGPRINT("SysLogInit() OK\n");

	//start agent threads
	if (Agent_INIT() != 0)
	{
		ERRORPRINT("AgentInit() fail\n");
		initFail = "agent";
	}
	else DEBUGPRINT("AgentInit() OK\n");

	//start arbotix threads
	if (Arbotix_INIT() != 0)
	{
		ERRORPRINT("ArbotixInit() fail\n");
		initFail = "arbotix";
	}
	else DEBUGPRINT("ArbotixInit() OK\n");

	//start autopilot threads
	if (Autopilot_INIT() != 0)
	{
		ERRORPRINT("AutopilotInit() fail\n");
		initFail = "pilot";
	}
	else DEBUGPRINT("AutopilotInit() OK\n");

	//start behavior tree threads
	if (Behavior_INIT() != 0)
	{
		ERRORPRINT("BehaviorInit() fail\n");
		initFail = "behavior";
	}
	else DEBUGPRINT("BehaviorInit OK\n");

	//start Dancer threads
	if (Dancer_INIT() != 0)
	{
		ERRORPRINT("DancerInit()\n fail");
		initFail = "dancer";
	}
	else DEBUGPRINT("DancerInit() OK\n");

	//start Gripper threads
	if (Gripper_INIT() != 0)
	{
		ERRORPRINT("GripperInit()\n fail");
		initFail = "gripper";
	}
	else DEBUGPRINT("GripperInit() OK\n");

	//start i2c threads
	if (I2C_INIT() != 0)
	{
		ERRORPRINT("I2CInit() fail\n");
		initFail = "i2c";
	}
	else DEBUGPRINT("I2CInit() OK\n");

	//start Lidar threads
	if (Lidar_INIT() != 0)
	{
		ERRORPRINT("LidarInit()\n");
		initFail = "RP_LIDAR";
	}
	else DEBUGPRINT("LidarInit() OK\n");

	//start navigator threads
	if (Navigator_INIT() != 0)
	{
		ERRORPRINT("NavigatorInit() fail\n");
		initFail = "navigator";
	}
	else DEBUGPRINT("NavigatorInit() OK\n");

	//start broker threads
	if (Broker_INIT() != 0)
	{
		ERRORPRINT( "BrokerInit() fail\n");
		initFail = "broker";
	}
	else DEBUGPRINT("BrokerInit() OK\n");

	if (Responder_INIT() != 0)
	{
		ERRORPRINT( "ResponderInit() fail\n");
		initFail = "responder";
	}
	else DEBUGPRINT("ResponderInit() OK\n");

	if (strlen(initFail) > 0)
	{
		LogError("Hexapod Init Fail '%s'\n", initFail);
		SetCondition(INIT_ERROR);
		sleep(5);
		return -1;
	}

	initComplete = true;

	LogRoutine("Init complete");

	if (getppid() == 1)
	{
		//child of init
		//set SIGHUP handler
		if (signal(SIGHUP, SIGHUPhandler) == SIG_ERR)
		{
			ERRORPRINT("SIGHUP err: %s", strerror(errno));
		}
		else
		{
			DEBUGPRINT("SIGHUP handler set");
		}
		//redirect stdout and stderr to null
		fclose(stdout);
		fclose(stderr);
		stdout = fopen("/dev/null", "w");
		stderr = fopen("/dev/null", "w");
	}

	while(1)
	{
		sleep(1);

		psMessage_t msg;
		psInitPublish(msg, TICK_1S);
		NewBrokerMessage(&msg);

		if (SIGHUPflag)
		{
			//SIGHUP used to reload edited scripts
			psInitPublish(msg, RELOAD);
			NewBrokerMessage(&msg);

			SIGHUPflag = 0;
		}
	}

	return 0;
}
//SIGHUP
void SIGHUPhandler(int sig)
{
	SIGHUPflag = 1;
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

