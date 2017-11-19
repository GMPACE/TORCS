/***************************************************************************

 file                 : human.cpp
 created              : Sat Mar 18 23:16:38 CET 2000
 copyright            : (C) 2000-2013 by Eric Espie, Bernhard Wymann
 email                : torcs@free.fr
 version              : $Id: human.cpp,v 1.45.2.18 2014/05/22 11:51:24 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
 Human driver
 @author	Bernhard Wymann, Eric Espie
 @version	$Id: human.cpp,v 1.45.2.18 2014/05/22 11:51:24 berniw Exp $
 */

#ifdef _WIN32
#include <windows.h>
#define isnan _isnan
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <plib/js.h>

#include "../../linux/shared_memory.h"

#include <tgfclient.h>
#include <portability.h>

#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include <playerpref.h>
#include <fstream>
#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <string>
#include "pref.h"
#include "human.h"
#include <algorithm>

#include "linalg.h"
#include "../data_list.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

#define RMAX 10000.0
#define BOTS 10
static const double g = 9.81;

#define DRWD 0
#define DFWD 1
#define D4WD 2
/* Hwancheol */
#define MIN(a, b) (((a) < (b))) ? a : b
#define MAX(a, b) (((a) > (b))) ? a : b
#define RECORD_COUNT 20
/* Hwancheol */

static void initTrack(int index, tTrack* track, void *carHandle,
		void **carParmHandle, tSituation *s);
static void drive_mt(int index, tCarElt* car, tSituation *s);
static void drive_at(int index, tCarElt* car, tSituation *s);
static void newrace(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);

/* Hwancheol */
static double car_speed = 0;
tdble drivespeed = 0.0;
short onoff_Mode = 0;
static short current_mode = 0;
static short prev_mode = 0;
double cur_speed;
double pre_speed;

v2d prev_pos;
//struct sembuf semopen = { 0, -1, SEM_UNDO };
//struct sembuf semclose = { 0, 1, SEM_UNDO };

static double* dist_to_ocar;
static double* dist_to_ocar_dlane;
static bool* acc_flag;
static double* speed_ocar;
static double temp_target_speed = -1;
static double calculate_CC(bool updown, tCarElt* car);
/* 한이음 */
static double cur_dist_l;
static double pre_dist_l;
static double sum_error_l;
static double cur_dist_r;
static double pre_dist_r;
static double sum_error_r;
static int change_count_l;
static int change_count_r;
static int record_count = RECORD_COUNT;
static long current_time;
static long prev_time;
static bool cur_rc_sig;
static bool pre_rc_sig;
static std::string f_output_string;
static int ldws(bool isOnleft, double dist_to_left, double dist_to_right, double dist_to_middle);
/* 한이음 */

using namespace std;

typedef struct {
	v3d pre_v;
	v3d current_v;
} direct_vec;
//for direction

direct_vec direc_vec;
direct_vec direc_vec_o;

ofstream f_output;
/* Hwancheol */
int joyPresent = 0;

static tTrack *curTrack;

static float color[] = { 0.0, 0.0, 1.0, 1.0 };

static tCtrlJoyInfo *joyInfo = NULL;
static tCtrlMouseInfo *mouseInfo = NULL;
static int masterPlayer = -1;

tHumanContext *HCtx[10] = { 0 };

static int speedLimiter = 0;
static tdble Vtarget;

typedef struct {
	int state;
	int edgeDn;
	int edgeUp;
} tKeyInfo;

static tKeyInfo keyInfo[256];
static tKeyInfo skeyInfo[256];

static int currentKey[256];
static int currentSKey[256];

static double lastKeyUpdate = -10.0;

static int firstTime = 0;

#ifdef _WIN32
/* should be present in mswindows */
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
	return TRUE;
}
#endif

static MyCar* mycar = NULL;

static OtherCar* ocar = NULL;
static TrackDesc* myTrackDesc = NULL;
static double currenttime;
static const tdble waitToTurn = 1.0; /* how long should i wait till i try to turn backwards */

static void shutdown(int index) {
	int idx = index - 1;

	free(HCtx[idx]);

	if (firstTime) {
		GfParmReleaseHandle(PrefHdle);
		GfctrlJoyRelease(joyInfo);
		GfctrlMouseRelease(mouseInfo);
		GfuiKeyEventRegisterCurrent(NULL);
		GfuiSKeyEventRegisterCurrent(NULL);
		firstTime = 0;
	}
//	delete(mycar);
//	delete(ocar);
//	delete(dist_to_ocar);
//	delete(acc_flag);
	onoff_Mode = 0;
	f_output.close();
}

/*
 * Function
 *	InitFuncPt
 *
 * Description
 *	Robot functions initialisation
 *
 * Parameters
 *	pt	pointer on functions structure
 *
 * Return
 *	0
 *
 * Remarks
 *
 */
static int InitFuncPt(int index, void *pt) {
	tRobotItf *itf = (tRobotItf *) pt;
	int idx = index - 1;

	if (masterPlayer == -1) {
		masterPlayer = index;
	}

	if (firstTime < 1) {
		firstTime = 1;
		joyInfo = GfctrlJoyInit();
		if (joyInfo) {
			joyPresent = 1;
		}

		mouseInfo = GfctrlMouseInit();
	}

	/* Allocate a new context for that player */
	HCtx[idx] = (tHumanContext *) calloc(1, sizeof(tHumanContext));

	HCtx[idx]->ABS = 1.0;
	HCtx[idx]->AntiSlip = 1.0;

	itf->rbNewTrack = initTrack; /* give the robot the track view called */
	/* for every track change or new race */
	itf->rbNewRace = newrace;

	HmReadPrefs(index);

	if (HCtx[idx]->Transmission == 0) {
		itf->rbDrive = drive_at;
	} else {
		itf->rbDrive = drive_mt; /* drive during race */
	}
	itf->rbShutdown = shutdown;
	itf->rbPitCmd = pitcmd;
	itf->index = index;

	return 0;
}

/*
 * Function
 *	human
 *
 * Description
 *	DLL entry point (general to all types of modules)
 *
 * Parameters
 *	modInfo	administrative info on module
 *
 * Return
 *	0
 *
 * Remarks
 *
 */

extern "C" int human(tModInfo *modInfo) {
	int i;
	const char *driver;
	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	char sstring[BUFSIZE];
//	/*NaYeon*/
//	shmid = shmget((key_t) skey, sizeof(int), 0777);
//	if (shmid == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//
////			semid = semget((key_t) sekey, 0, 0777);
////			if (semid == -1) {
////				perror("semget failed : ");
////				exit(1);
////			}
//	shared_memory = shmat(shmid, (void *) 0, 0);
//	if (!shared_memory) {
//		perror("shmat failed");
////		exit(1);
//	}
//	torcs_steer = (int*) shared_memory;
//
//	shmid2 = shmget((key_t) skey2, sizeof(int), 0777);
//	if (shmid2 == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory2 = shmat(shmid2, (void *) 0, 0);
//	if (!shared_memory2) {
//		perror("shmat failed");
////		exit(1);
//	}
//	ptr_brake = (int*) shared_memory2;
//
//	shmid3 = shmget((key_t) skey3, sizeof(int), 0777);
//	if (shmid3 == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory3 = shmat(shmid3, (void *) 0, 0);
//	if (!shared_memory3) {
//		perror("shmat failed");
////		exit(1);
//	}
//	ptr_accel = (int*) shared_memory3;
//
//
//
//	shmid_recspeed = shmget((key_t) skey_recspeed, sizeof(int), 0777);
//	if (shmid_recspeed == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory_recspeed = shmat(shmid_recspeed, (void *) 0, 0);
//	if (!shared_memory_recspeed) {
//		perror("shmat failed");
////		exit(1);
//	}
//	rec_speed = (int*) shared_memory_recspeed;
//
//
//
//	shmid_recrpm = shmget((key_t) skey_recrpm, sizeof(int), 0777);
//	if (shmid_recrpm == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory_recrpm = shmat(shmid_recrpm, (void *) 0, 0);
//	if (!shared_memory_recrpm) {
//		perror("shmat failed");
////		exit(1);
//	}
//	rec_rpm = (int*) shared_memory_recrpm;
//
//
//
//	shmid_recsteer = shmget((key_t) skey_recsteer, sizeof(int), 0777);
//	if (shmid_recsteer == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory_recsteer = shmat(shmid_recsteer, (void *) 0, 0);
//	if (!shared_memory_recsteer) {
//		perror("shmat failed");
////		exit(1);
//	}
//	rec_steer = (int*) shared_memory_recsteer;
//
//
//
////	shmid_acc = shmget((key_t) skey_acc, sizeof(int), 0777);
////	if (shmid_acc == -1) {
////		perror("shmget failed :");
//////		exit(1);
////	}
////	printf("test7\n");
////	shared_memory_acc = shmat(shmid_acc, (void *) 0, 0);
////	if (!shared_memory_acc) {
////		perror("shmat failed");
//////		exit(1);
////	}
////	rec_acc = (int*) shared_memory_acc;
//
//
//	shmid_lkas = shmget((key_t) skey_lkas, sizeof(int), 0777);
//	if (shmid_lkas == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory_lkas = shmat(shmid_lkas, (void *) 0, 0);
//	if (!shared_memory_acc) {
//		perror("shmat failed");
////		exit(1);
//	}
//	rec_lkas = (int*) shared_memory_lkas;
//

//	shmid_targetspeed = shmget((key_t) skey_targetspeed, sizeof(int), 0777);
//	if (shmid_acc == -1) {
//		perror("shmget failed :");
////		exit(1);
//	}
//	shared_memory_targetspeed = shmat(shmid_targetspeed, (void *) 0, 0);
//	if (!shared_memory_targetspeed) {
//		perror("shmat failed");
////		exit(1);
//	}
//	rec_targetspeed = (int*) shared_memory_targetspeed;
	memset(modInfo, 0, 10 * sizeof(tModInfo));

	snprintf(buf, BUFSIZE, "%sdrivers/human/human.xml", GetLocalDir());
	void *DrvInfo = GfParmReadFile(buf,
	GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	if (DrvInfo != NULL) {
		for (i = 0; i < 10; i++) {
			snprintf(sstring, BUFSIZE, "Robots/index/%d", i + 1);
			driver = GfParmGetStr(DrvInfo, sstring, "name", "");
			if (strlen(driver) == 0) {
				break;
			}

			modInfo->name = strdup(driver); /* name of the module (short) */
			modInfo->desc = strdup("Joystick controlable driver"); /* description of the module (can be long) */
			modInfo->fctInit = InitFuncPt; /* init function */
			modInfo->gfId = ROB_IDENT; /* supported framework version */
			modInfo->index = i + 1;
			modInfo++;
		}
		// Just release in case we got it.
		GfParmReleaseHandle(DrvInfo);
	}

	return 0;
}

/*
 * Function
 *
 *
 * Description
 *	search under drivers/human/tracks/<trackname>/car-<model>-<index>.xml
 *		     drivers/human/car-<model>-<index>.xml
 *		     drivers/human/tracks/<trackname>/car-<model>.xml
 *		     drivers/human/car-<model>.xml
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *
 */
string to_string(int n)
{
	stringstream s;
	s << n;
	return s.str();
}

static void initTrack(int index, tTrack* track, void *carHandle,
		void **carParmHandle, tSituation *s) {
	struct tm* datetime;
	time_t t;
	t = time(NULL);
	datetime = localtime(&t);
	string path = "/home/kang/temp/";
	string s_t = path.append(
				to_string(datetime->tm_year + 1900)).append("-").append(
				to_string(datetime->tm_mon + 1)).append("-").append(
				to_string(datetime->tm_mday)).append("_").append(
				to_string(datetime->tm_hour)).append(":").append(
				to_string(datetime->tm_min)).append(":").append(
				to_string(datetime->tm_sec));
	f_output.open(s_t.c_str());

	v3d v1, v2;
	v1.x = 0.0;
	v1.y = 0.0;
	v2.x = 0.0;
	v2.y = 0.0;
	direc_vec = {v1, v2};
	direc_vec_o = {v1, v2};
	if ((myTrackDesc != NULL) && (myTrackDesc->getTorcsTrack() != track)) {
		delete myTrackDesc;
		myTrackDesc = NULL;
	}
	if (myTrackDesc == NULL) {
		myTrackDesc = new TrackDesc(track);
	}

	const char *carname;
	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	char sstring[BUFSIZE];
	tdble fuel;
	int idx = index - 1;

	curTrack = track;

	snprintf(sstring, BUFSIZE, "Robots/index/%d", index);
	snprintf(buf, BUFSIZE, "%sdrivers/human/human.xml", GetLocalDir());
	void *DrvInfo = GfParmReadFile(buf,
	GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	carname = "";
	if (DrvInfo != NULL) {
		carname = GfParmGetStr(DrvInfo, sstring, "car name", "");
	}

	*carParmHandle = NULL;
	// If session type is "race" and we have a race setup use it
	if (s->_raceType == RM_TYPE_RACE) {
		*carParmHandle = RtParmReadSetup(RACE, "human", index,
				track->internalname, carname);
	}

	// If session type is "qualifying" and we have a qualifying setup use it, use qualifying setup as 
	// fallback if not race setup is available
	if (s->_raceType == RM_TYPE_QUALIF
			|| (*carParmHandle == NULL && s->_raceType == RM_TYPE_RACE)) {
		*carParmHandle = RtParmReadSetup(QUALIFYING, "human", index,
				track->internalname, carname);
	}

	// If we have not yet loaded a setup we have not found a fitting one or want to use the practice setup,
	// so try to load this
	if (*carParmHandle == NULL) {
		*carParmHandle = RtParmReadSetup(PRACTICE, "human", index,
				track->internalname, carname);
	}

	// Absolute fallback, nothing found
	if (*carParmHandle == NULL) {
		snprintf(sstring, BUFSIZE, "%sdrivers/human/car.xml", GetLocalDir());
		*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
	}

	if (curTrack->pits.type != TR_PIT_NONE) {
		snprintf(sstring, BUFSIZE, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV,
				index);
		HCtx[idx]->NbPitStopProg = (int) GfParmGetNum(PrefHdle, sstring,
		HM_ATT_NBPITS, (char*) NULL, 0);
		GfOut("Player: index %d , Pits stops %d\n", index,
				HCtx[idx]->NbPitStopProg);
	} else {
		HCtx[idx]->NbPitStopProg = 0;
	}
	fuel = 0.0008 * curTrack->length * (s->_totLaps + 1)
			/ (1.0 + ((tdble) HCtx[idx]->NbPitStopProg)) + 20.0;
	if (*carParmHandle) {
		GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*) NULL, fuel);
	}
	Vtarget = curTrack->pits.speedLimit;
	if (DrvInfo != NULL) {
		GfParmReleaseHandle(DrvInfo);
	}
}

/*
 * Function
 *
 *
 * Description
 *
 *
 * Parameters
 *
 *
 * Return
 *
 */

static void newrace(int index, tCarElt* car, tSituation *s) {
	prev_pos.x = car->_pos_X;
	prev_pos.y = car->_pos_Y;
	mycar = new MyCar(myTrackDesc, car, s);
	dist_to_ocar = new double();
	dist_to_ocar_dlane = new double();
	speed_ocar = new double();
	acc_flag = new bool();
	*acc_flag = false;
	if (ocar != NULL)
		delete[] ocar;
	ocar = new OtherCar[s->_ncars];
	for (int i = 0; i < s->_ncars; i++) {
		ocar[i].init(myTrackDesc, s->cars[i], s);
	}
	int idx = index - 1;

	if (HCtx[idx]->MouseControlUsed) {
		GfctrlMouseCenter();
	}

	memset(keyInfo, 0, sizeof(keyInfo));
	memset(skeyInfo, 0, sizeof(skeyInfo));

	memset(currentKey, 0, sizeof(currentKey));
	memset(currentSKey, 0, sizeof(currentSKey));

#ifndef WIN32
#ifdef TELEMETRY
	if (s->_raceType == RM_TYPE_PRACTICE) {
		RtTelemInit(-10, 10);
		RtTelemNewChannel("Dist", &HCtx[idx]->distToStart, 0, 0);
		RtTelemNewChannel("Ax", &car->_accel_x, 0, 0);
		RtTelemNewChannel("Ay", &car->_accel_y, 0, 0);
		RtTelemNewChannel("Steer", &car->ctrl->steer, 0, 0);
		RtTelemNewChannel("Throttle", &car->ctrl->accelCmd, 0, 0);
		RtTelemNewChannel("Brake", &car->ctrl->brakeCmd, 0, 0);
		RtTelemNewChannel("Gear", &HCtx[idx]->Gear, 0, 0);
		RtTelemNewChannel("Speed", &car->_speed_x, 0, 0);
	}
#endif
#endif

	const char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN,
	PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
		HCtx[idx]->drivetrain = DRWD;
	} else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
		HCtx[idx]->drivetrain = DFWD;
	} else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
		HCtx[idx]->drivetrain = D4WD;
	}

	tControlCmd *cmd = HCtx[idx]->CmdControl;
	if (cmd[CMD_CLUTCH].type != GFCTRL_TYPE_JOY_AXIS
			&& cmd[CMD_CLUTCH].type != GFCTRL_TYPE_MOUSE_AXIS)
		HCtx[idx]->autoClutch = 1;
	else
		HCtx[idx]->autoClutch = 0;

	printf("newrace\n");
}

static void updateKeys(void) {
	int i;
	int key;
	int idx;
	tControlCmd *cmd;

	for (idx = 0; idx < 10; idx++) {
		if (HCtx[idx]) {
			cmd = HCtx[idx]->CmdControl;
			for (i = 0; i < nbCmdControl; i++) {
				if (cmd[i].type == GFCTRL_TYPE_KEYBOARD) {
					key = cmd[i].val;
					if (currentKey[key] == GFUI_KEY_DOWN) {
						if (keyInfo[key].state == GFUI_KEY_UP) {
							keyInfo[key].edgeDn = 1;
						} else {
							keyInfo[key].edgeDn = 0;
						}
					} else {
						if (keyInfo[key].state == GFUI_KEY_DOWN) {
							keyInfo[key].edgeUp = 1;
						} else {
							keyInfo[key].edgeUp = 0;
						}
					}
					keyInfo[key].state = currentKey[key];
				}

				if (cmd[i].type == GFCTRL_TYPE_SKEYBOARD) {
					key = cmd[i].val;
					if (currentSKey[key] == GFUI_KEY_DOWN) {
						if (skeyInfo[key].state == GFUI_KEY_UP) {
							skeyInfo[key].edgeDn = 1;
						} else {
							skeyInfo[key].edgeDn = 0;
						}
					} else {
						if (skeyInfo[key].state == GFUI_KEY_DOWN) {
							skeyInfo[key].edgeUp = 1;
						} else {
							skeyInfo[key].edgeUp = 0;
						}
					}
					skeyInfo[key].state = currentSKey[key];
				}
			}
		}
	}
}

static int onKeyAction(unsigned char key, int modifier, int state) {
	currentKey[key] = state;

	return 0;
}

static int onSKeyAction(int key, int modifier, int state) {
	currentSKey[key] = state;
	return 0;
}

static void common_drive(int index, tCarElt* car, tSituation *s) {
	/* Hwancheol */
	/************ACC***********/
	*dist_to_ocar = 1000000.f;
	*dist_to_ocar_dlane = 1000000.f;
	if(current_mode != onoff_Mode){
		prev_mode = current_mode;
		current_mode = onoff_Mode;
	}
	/* update some values needed */
	mycar->update(myTrackDesc, car, s);
	if (car->pub.trkPos.toLeft < car->pub.trkPos.toRight)
		mycar->isonLeft = true;
	else
		mycar->isonLeft = false;
	//double raced_dist = mycar->getCarPtr()->race.distRaced;
	//double raced_dist_o = 0;
	double myCar_x = mycar->getCurrentPos()->x;
	double oCar_x = 0;
	/* 한이음 */
	//ldws(mycar->isonLeft, car->pub.trkPos.toLeft, car->pub.trkPos.toRight, car->pub.trkPos.toMiddle);
	/* Nayeon : transfer to K7 */
	//car->PRM_RPM
	/* update the other cars just once */
	if (currenttime != s->currentTime) {
		currenttime = s->currentTime;
		double temp = *dist_to_ocar;
		for (int i = 0; i < s->_ncars; i++) {
			ocar[i].update();
			temp = sqrt(
					pow(
							(mycar->getCurrentPos()->x
									- ocar[i].getCurrentPos()->x), 2.0)
							+ pow(
									(mycar->getCurrentPos()->y
											- ocar[i].getCurrentPos()->y),
									2.0));
			//raced_dist_o = ocar[i].getCarPtr()->race.distRaced;
			oCar_x = ocar[i].getCurrentPos()->x;
			if (ocar[i].getCarPtr()->pub.trkPos.toLeft
					< ocar[i].getCarPtr()->pub.trkPos.toRight)
				ocar[i].isonLeft = true;
			else
				ocar[i].isonLeft = false;
			//if (temp != 0 && (raced_dist_o - raced_dist) > 0 && mycar->isonLeft == ocar[i].isonLeft) {

			//printf("%f %f %f\n", myCar_x, oCar_x, temp);
			if(temp != 0 && (oCar_x - myCar_x > 0) && mycar->isonLeft == ocar[i].isonLeft) {
				*speed_ocar = ocar[i].getCarPtr()->_speed_x;
				*dist_to_ocar = min(temp, *dist_to_ocar);
			}
			//else if (temp != 0 && (raced_dist_o - raced_dist) <= 0 && mycar->isonLeft != ocar[i].isonLeft) {
			else if(temp != 0 && (oCar_x - myCar_x <= 0) && mycar->isonLeft == ocar[i].isonLeft) {
				*dist_to_ocar_dlane = min(temp, *dist_to_ocar_dlane);
			}

//			printf("mycar's position : (%f, %f)\n", mycar->getCurrentPos()->x,
//					mycar->getCurrentPos()->y);
//			printf("other car's position : (%f, %f)\n",
//					ocar[i].getCurrentPos()->x, ocar[i].getCurrentPos()->y);
		}
	}

	tdble slip;
	tdble ax0;
	tdble brake;
	tdble clutch;
	tdble throttle;
	tdble leftSteer;
	tdble rightSteer;
	int scrw, scrh, dummy;
	int idx = index - 1;
	tControlCmd *cmd = HCtx[idx]->CmdControl;
	const int BUFSIZE = 1024;
	char sstring[BUFSIZE];

	static int firstTime = 1;

	if (firstTime) {
		if (HCtx[idx]->MouseControlUsed) {
			GfuiMouseShow();
			GfctrlMouseInitCenter();
		}
		GfuiKeyEventRegisterCurrent(onKeyAction);
		GfuiSKeyEventRegisterCurrent(onSKeyAction);
		firstTime = 0;
	}

	HCtx[idx]->distToStart = RtGetDistFromStart(car);

	HCtx[idx]->Gear = (tdble) car->_gear; /* telemetry */

	GfScrGetSize(&scrw, &scrh, &dummy, &dummy);

	memset(&(car->ctrl), 0, sizeof(tCarCtrl));

	car->_lightCmd = HCtx[idx]->lightCmd;

	if (car->_laps != HCtx[idx]->LastPitStopLap) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
	}

	if (lastKeyUpdate != s->currentTime) {
		/* Update the controls only once for all the players */
		updateKeys();

		if (joyPresent) {
			GfctrlJoyGetCurrent(joyInfo);
		}

		GfctrlMouseGetCurrent(mouseInfo);
		lastKeyUpdate = s->currentTime;
	}

	if (((cmd[CMD_ABS].type == GFCTRL_TYPE_JOY_BUT)
			&& joyInfo->edgeup[cmd[CMD_ABS].val])
			|| ((cmd[CMD_ABS].type == GFCTRL_TYPE_KEYBOARD)
					&& keyInfo[cmd[CMD_ABS].val].edgeUp)
			|| ((cmd[CMD_ABS].type == GFCTRL_TYPE_SKEYBOARD)
					&& skeyInfo[cmd[CMD_ABS].val].edgeUp)) {
		HCtx[idx]->ParamAbs = 1 - HCtx[idx]->ParamAbs;
		snprintf(sstring, BUFSIZE, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV,
				index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ABS,
				Yn[1 - HCtx[idx]->ParamAbs]);
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	if (((cmd[CMD_ASR].type == GFCTRL_TYPE_JOY_BUT)
			&& joyInfo->edgeup[cmd[CMD_ASR].val])
			|| ((cmd[CMD_ASR].type == GFCTRL_TYPE_KEYBOARD)
					&& keyInfo[cmd[CMD_ASR].val].edgeUp)
			|| ((cmd[CMD_ASR].type == GFCTRL_TYPE_SKEYBOARD)
					&& skeyInfo[cmd[CMD_ASR].val].edgeUp)) {
		HCtx[idx]->ParamAsr = 1 - HCtx[idx]->ParamAsr;
		snprintf(sstring, BUFSIZE, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV,
				index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ASR,
				Yn[1 - HCtx[idx]->ParamAsr]);
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	const int bufsize = sizeof(car->_msgCmd[0]);
	snprintf(car->_msgCmd[0], bufsize, "%s %s",
			(HCtx[idx]->ParamAbs ? "ABS" : ""),
			(HCtx[idx]->ParamAsr ? "ASR" : ""));
	memcpy(car->_msgColorCmd, color, sizeof(car->_msgColorCmd));

	if (((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_JOY_BUT)
			&& (joyInfo->levelup[cmd[CMD_SPDLIM].val] == 1))
			|| ((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_KEYBOARD)
					&& (keyInfo[cmd[CMD_SPDLIM].val].state == GFUI_KEY_DOWN))
			|| ((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_SKEYBOARD)
					&& (skeyInfo[cmd[CMD_SPDLIM].val].state == GFUI_KEY_DOWN))) {
		speedLimiter = 1;
		snprintf(car->_msgCmd[1], bufsize, "Speed Limiter On");
	} else {
		speedLimiter = 0;
		snprintf(car->_msgCmd[1], bufsize, "Speed Limiter Off");
	}

	if (((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_JOY_BUT)
			&& joyInfo->edgeup[cmd[CMD_LIGHT1].val])
			|| ((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_KEYBOARD)
					&& keyInfo[cmd[CMD_LIGHT1].val].edgeUp)
			|| ((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_SKEYBOARD)
					&& skeyInfo[cmd[CMD_LIGHT1].val].edgeUp)) {
		if (HCtx[idx]->lightCmd & RM_LIGHT_HEAD1) {
			HCtx[idx]->lightCmd &= ~(RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2);
		} else {
			HCtx[idx]->lightCmd |= RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;
		}
	}

	switch (cmd[CMD_LEFTSTEER].type) {
	case GFCTRL_TYPE_JOY_AXIS:
		ax0 = joyInfo->ax[cmd[CMD_LEFTSTEER].val] + cmd[CMD_LEFTSTEER].deadZone;
		if (ax0 > cmd[CMD_LEFTSTEER].max) {
			ax0 = cmd[CMD_LEFTSTEER].max;
		} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
			ax0 = cmd[CMD_LEFTSTEER].min;
		}

		// normalize ax0 to -1..0
		ax0 = (ax0 - cmd[CMD_LEFTSTEER].max)
				/ (cmd[CMD_LEFTSTEER].max - cmd[CMD_LEFTSTEER].min);
		leftSteer = -SIGN(ax0) * cmd[CMD_LEFTSTEER].pow
				* pow(fabs(ax0), cmd[CMD_LEFTSTEER].sens)
				/ (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->pub.speed);
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_MOUSE_AXIS:

		ax0 = mouseInfo->ax[cmd[CMD_LEFTSTEER].val]
				- cmd[CMD_LEFTSTEER].deadZone; //FIXME: correct?
		if (ax0 > cmd[CMD_LEFTSTEER].max) {
			ax0 = cmd[CMD_LEFTSTEER].max;
		} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
			ax0 = cmd[CMD_LEFTSTEER].min;
		}
		ax0 = ax0 * cmd[CMD_LEFTSTEER].pow;
		leftSteer = pow(fabs(ax0), cmd[CMD_LEFTSTEER].sens)
				/ (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->pub.speed / 10.0);
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_KEYBOARD:
	case GFCTRL_TYPE_SKEYBOARD:
	case GFCTRL_TYPE_JOY_BUT:
		if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
			ax0 = keyInfo[cmd[CMD_LEFTSTEER].val].state;
		} else if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_SKEYBOARD) {
			ax0 = skeyInfo[cmd[CMD_LEFTSTEER].val].state;
		} else {
			ax0 = joyInfo->levelup[cmd[CMD_LEFTSTEER].val];
		}
		if (ax0 == 0) {
			HCtx[idx]->prevLeftSteer = leftSteer = 0;
		} else {
			ax0 = 2 * ax0 - 1;
			leftSteer = HCtx[idx]->prevLeftSteer
					+ ax0 * cmd[CMD_LEFTSTEER].sens * s->deltaTime
							/ (1.0
									+ cmd[CMD_LEFTSTEER].spdSens
											* car->pub.speed / 10.0);
			if (leftSteer > 1.0)
				leftSteer = 1.0;
			if (leftSteer < 0.0)
				leftSteer = 0.0;
			HCtx[idx]->prevLeftSteer = leftSteer;
		}
		break;
	default:
		leftSteer = 0;
		break;
	}

	/* Right Steer Value */
	switch (cmd[CMD_RIGHTSTEER].type) {
	case GFCTRL_TYPE_JOY_AXIS:
		ax0 = joyInfo->ax[cmd[CMD_RIGHTSTEER].val]
				- cmd[CMD_RIGHTSTEER].deadZone;
		if (ax0 > cmd[CMD_RIGHTSTEER].max) {
			ax0 = cmd[CMD_RIGHTSTEER].max;
		} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
			ax0 = cmd[CMD_RIGHTSTEER].min;
		}

		// normalize ax to 0..1
		ax0 = (ax0 - cmd[CMD_RIGHTSTEER].min)
				/ (cmd[CMD_RIGHTSTEER].max - cmd[CMD_RIGHTSTEER].min);
		rightSteer = -SIGN(ax0) * cmd[CMD_RIGHTSTEER].pow
				* pow(fabs(ax0), cmd[CMD_RIGHTSTEER].sens)
				/ (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->pub.speed);
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_MOUSE_AXIS:
		ax0 = mouseInfo->ax[cmd[CMD_RIGHTSTEER].val]
				- cmd[CMD_RIGHTSTEER].deadZone;
		if (ax0 > cmd[CMD_RIGHTSTEER].max) {
			ax0 = cmd[CMD_RIGHTSTEER].max;
		} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
			ax0 = cmd[CMD_RIGHTSTEER].min;
		}
		ax0 = ax0 * cmd[CMD_RIGHTSTEER].pow;
		rightSteer = -pow(fabs(ax0), cmd[CMD_RIGHTSTEER].sens)
				/ (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->pub.speed / 10.0);
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_KEYBOARD:
	case GFCTRL_TYPE_SKEYBOARD:
	case GFCTRL_TYPE_JOY_BUT:
		if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
			ax0 = keyInfo[cmd[CMD_RIGHTSTEER].val].state;
		} else if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_SKEYBOARD) {
			ax0 = skeyInfo[cmd[CMD_RIGHTSTEER].val].state;
		} else {
			ax0 = joyInfo->levelup[cmd[CMD_RIGHTSTEER].val];
		}
		if (ax0 == 0) {
			HCtx[idx]->prevRightSteer = rightSteer = 0;
		} else {
			ax0 = 2 * ax0 - 1;
			rightSteer = HCtx[idx]->prevRightSteer
					- ax0 * cmd[CMD_RIGHTSTEER].sens * s->deltaTime
							/ (1.0
									+ cmd[CMD_RIGHTSTEER].spdSens
											* car->pub.speed / 10.0);
			if (rightSteer > 0.0)
				rightSteer = 0.0;
			if (rightSteer < -1.0)
				rightSteer = -1.0;
			HCtx[idx]->prevRightSteer = rightSteer;
		}
		break;
	default:
		rightSteer = 0;
		break;
	}
	if ((onoff_Mode & 1) == 1) {

		/*shared memory & semaphore change value*/
		/*NaYeon*/
//		if(semop(semid,&semopen,1) == -1)
//		{
//			perror("semop error : ");
//			exit(0);
//		}
//		double k7_steer = ((double) (*torcs_steer)) / 180;
////		printf("after access shared memory\n");
//		car->_steerCmd = k7_steer;
//		semop(semid, &semclose,1);
		/*******************************************/

//
		float length = 0.0;
		float angle = 0.0;
		float dist = 0.0;
		int check = 0;
		tTrackSeg *seg = car->_trkPos.seg;
		float track_info;
		float lookahead_const = 17.0;	// m
		float lookahead_factor = 0.33;	// 1/s
		float lookahead = lookahead_const + car->_speed_x * lookahead_factor;
		v2d re;
		v2d a;

		/* LKAS */
		// enhanced steering (based on track info. and current position)
		if (car->_trkPos.seg->type == TR_STR)
			length = car->_trkPos.seg->length - car->_trkPos.toStart;
		else
			length = (car->_trkPos.seg->arc - car->_trkPos.toStart)
					* car->_trkPos.seg->radius;

		while (length < lookahead) {
			seg = seg->next;
			length += seg->length;
		}

		length = lookahead - length + seg->length;
		if (car->_trkPos.toRight < car->_trkPos.toLeft) {
			a.x = (seg->vertex[TR_SL].x * 4 / 15
					+ seg->vertex[TR_SR].x * 11 / 15);
			a.y = (seg->vertex[TR_SL].y * 4 / 15
					+ seg->vertex[TR_SR].y * 11 / 15);
		} else {
			a.x = (seg->vertex[TR_SL].x * 11 / 15
					+ seg->vertex[TR_SR].x * 4 / 15);
			a.y = (seg->vertex[TR_SL].y * 11 / 15
					+ seg->vertex[TR_SR].y * 4 / 15);
		}
		if (seg->type == TR_STR) {
			v2d d;
			d.x = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / seg->length;
			d.y = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / seg->length;
			re = a + d * length;
		} else {
			v2d c;
			c.x = seg->center.x;
			c.y = seg->center.y;
			float arc = length / seg->radius;
			float arcsign = (seg->type == TR_RGT) ? -1 : 1;
			arc = arc * arcsign;
			re = a.rotate(c, arc);
		}

		angle = atan2(re.y - car->_pos_Y, re.x - car->_pos_X);
		angle -= car->_yaw;
		NORM_PI_PI(angle);

		car->_steerCmd = angle;

	} else {
		car->_steerCmd = leftSteer + rightSteer;
	}

	car_speed = car->_speed_x;

	/* Brake Value */
	switch (cmd[CMD_BRAKE].type) {
	case GFCTRL_TYPE_JOY_AXIS:
		brake = joyInfo->ax[cmd[CMD_BRAKE].val];
		if (brake > cmd[CMD_BRAKE].max) {
			brake = cmd[CMD_BRAKE].max;
		} else if (brake < cmd[CMD_BRAKE].min) {
			brake = cmd[CMD_BRAKE].min;
		}
		car->_brakeCmd = fabs(
				cmd[CMD_BRAKE].pow
						* pow(
								fabs(
										(brake - cmd[CMD_BRAKE].minVal)
												/ (cmd[CMD_BRAKE].max
														- cmd[CMD_BRAKE].min)),
								cmd[CMD_BRAKE].sens));
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_MOUSE_AXIS:
		ax0 = mouseInfo->ax[cmd[CMD_BRAKE].val] - cmd[CMD_BRAKE].deadZone;
		if (ax0 > cmd[CMD_BRAKE].max) {
			ax0 = cmd[CMD_BRAKE].max;
		} else if (ax0 < cmd[CMD_BRAKE].min) {
			ax0 = cmd[CMD_BRAKE].min;
		}
		ax0 = ax0 * cmd[CMD_BRAKE].pow;
		car->_brakeCmd = pow(fabs(ax0), cmd[CMD_BRAKE].sens)
				/ (1.0 + cmd[CMD_BRAKE].spdSens * car->_speed_x / 10.0);
		/* for K7 */
		brake_value = *ptr_brake;
		printf("brake : %d\n", brake_value);
		car->_brakeCmd = brake_value;
		/* for K7 */
		/* CC Mode On */
		if ((onoff_Mode & (short) 2) == (short) 2) {

			car->_brakeCmd = calculate_CC(false, car);
		}
		break;

		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_JOY_BUT:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_brakeCmd = calculate_CC(false, car);
		else
			car->_brakeCmd = joyInfo->levelup[cmd[CMD_BRAKE].val];
		break;
	case GFCTRL_TYPE_MOUSE_BUT:
		car->_brakeCmd = mouseInfo->button[cmd[CMD_BRAKE].val];
		break;
	case GFCTRL_TYPE_KEYBOARD:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_brakeCmd = calculate_CC(false, car);
		else
			car->_brakeCmd = keyInfo[cmd[CMD_BRAKE].val].state;
		break;
	case GFCTRL_TYPE_SKEYBOARD:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_brakeCmd = calculate_CC(false, car);
		else
			car->_brakeCmd = skeyInfo[cmd[CMD_BRAKE].val].state;
		break;
	default:
		car->_brakeCmd = 0;
		break;
	}
	switch (cmd[CMD_CLUTCH].type) {
	case GFCTRL_TYPE_JOY_AXIS:
		clutch = joyInfo->ax[cmd[CMD_CLUTCH].val];
		if (clutch > cmd[CMD_CLUTCH].max) {
			clutch = cmd[CMD_CLUTCH].max;
		} else if (clutch < cmd[CMD_CLUTCH].min) {
			clutch = cmd[CMD_CLUTCH].min;
		}
		car->_clutchCmd = fabs(
				cmd[CMD_CLUTCH].pow
						* pow(
								fabs(
										(clutch - cmd[CMD_CLUTCH].minVal)
												/ (cmd[CMD_CLUTCH].max
														- cmd[CMD_CLUTCH].min)),
								cmd[CMD_CLUTCH].sens));
		break;
	case GFCTRL_TYPE_MOUSE_AXIS:
		ax0 = mouseInfo->ax[cmd[CMD_CLUTCH].val] - cmd[CMD_CLUTCH].deadZone;
		if (ax0 > cmd[CMD_CLUTCH].max) {
			ax0 = cmd[CMD_CLUTCH].max;
		} else if (ax0 < cmd[CMD_CLUTCH].min) {
			ax0 = cmd[CMD_CLUTCH].min;
		}
		ax0 = ax0 * cmd[CMD_CLUTCH].pow;
		car->_clutchCmd = pow(fabs(ax0), cmd[CMD_CLUTCH].sens)
				/ (1.0 + cmd[CMD_CLUTCH].spdSens * car->_speed_x / 10.0);
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_JOY_BUT:
		car->_clutchCmd = joyInfo->levelup[cmd[CMD_CLUTCH].val];
		break;
	case GFCTRL_TYPE_MOUSE_BUT:
		car->_clutchCmd = mouseInfo->button[cmd[CMD_CLUTCH].val];
		break;
	case GFCTRL_TYPE_KEYBOARD:
		car->_clutchCmd = keyInfo[cmd[CMD_CLUTCH].val].state;
		break;
	case GFCTRL_TYPE_SKEYBOARD:
		car->_clutchCmd = skeyInfo[cmd[CMD_CLUTCH].val].state;
		break;
	default:
		car->_clutchCmd = 0;
		break;
	}
	float atan_accel;
	// if player's used the clutch manually then we dispense with autoClutch
	if (car->_clutchCmd != 0.0f)
		HCtx[idx]->autoClutch = 0;

	/* Throttle Value */

	switch (cmd[CMD_THROTTLE].type) {
	case GFCTRL_TYPE_JOY_AXIS:
		throttle = joyInfo->ax[cmd[CMD_THROTTLE].val];
		if (throttle > cmd[CMD_THROTTLE].max) {
			throttle = cmd[CMD_THROTTLE].max;
		} else if (throttle < cmd[CMD_THROTTLE].min) {
			throttle = cmd[CMD_THROTTLE].min;
		}
		car->_accelCmd =
				fabs(
						cmd[CMD_THROTTLE].pow
								* pow(
										fabs(
												(throttle
														- cmd[CMD_THROTTLE].minVal)
														/ (cmd[CMD_THROTTLE].max
																- cmd[CMD_THROTTLE].min)),
										cmd[CMD_THROTTLE].sens));

		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_MOUSE_AXIS:
		ax0 = mouseInfo->ax[cmd[CMD_THROTTLE].val] - cmd[CMD_THROTTLE].deadZone;

		if (ax0 > cmd[CMD_THROTTLE].max) {
			ax0 = cmd[CMD_THROTTLE].max;
		} else if (ax0 < cmd[CMD_THROTTLE].min) {
			ax0 = cmd[CMD_THROTTLE].min;
		}
		ax0 = ax0 * cmd[CMD_THROTTLE].pow;
		car->_accelCmd = pow(fabs(ax0), cmd[CMD_THROTTLE].sens)
				/ (1.0 + cmd[CMD_THROTTLE].spdSens * car->_speed_x / 10.0);
		if (isnan(car->_accelCmd)) {
			car->_accelCmd = 0;
		}
		/* for K7 */
		accel_value = *ptr_accel;
		accel_value = MIN(MAX(610,accel_value), 3515);

		atan_accel = (float)(atan(((accel_value-610)/726))/(PI/2));
		printf("accel : %d\n", atan_accel);
		car->_accelCmd = atan_accel;
		/* for K7 */

		/* CC Mode On */
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_accelCmd = calculate_CC(true, car);
		/* printf("  axO:%f  accelCmd:%f\n", ax0, car->_accelCmd); */
		if (car->_brakeCmd > 0)
			car->_accelCmd = 0;
		break;
		/* TODO : K7 Mapping Part */
	case GFCTRL_TYPE_JOY_BUT:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_accelCmd = calculate_CC(true, car);
		else
			car->_accelCmd = joyInfo->levelup[cmd[CMD_THROTTLE].val];
		break;
	case GFCTRL_TYPE_MOUSE_BUT:
		car->_accelCmd = mouseInfo->button[cmd[CMD_THROTTLE].val];
		break;
	case GFCTRL_TYPE_KEYBOARD:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_accelCmd = calculate_CC(true, car);
		else
			car->_accelCmd = keyInfo[cmd[CMD_THROTTLE].val].state;
		break;
	case GFCTRL_TYPE_SKEYBOARD:
		if ((onoff_Mode & (short) 2) == (short) 2)
			car->_accelCmd = calculate_CC(true, car);
		else
			car->_accelCmd = skeyInfo[cmd[CMD_THROTTLE].val].state;
		break;
	default:
		car->_accelCmd = 0;
		break;
	}

	/* Hwancheol */
	if((onoff_Mode & (short) 2) != (short) 2) {
			temp_target_speed = -1;
		}
	/******Data Logging Part******/

	//printf("속력 : %fkm/h\n", car->pub.speed * 3.6);
	//printf("스티어링 : %f%\n", car->_steerCmd * 100);
	pre_speed = cur_speed;
	cur_speed = car->pub.speed;
	pre_rc_sig = cur_rc_sig;
	cur_rc_sig = car->pub.record_signal;
	if(pre_rc_sig != cur_rc_sig) {
		record_count = RECORD_COUNT;
	}
	if (car->pub.record_signal) {
		struct timeval val;
		gettimeofday(&val, NULL);
		prev_time = current_time;
		current_time = val.tv_usec;
		prev_time /= 100000;
		current_time /= 100000;
		if (current_time - prev_time == 2) {
			double accelerate_ = cur_speed - pre_speed;
			if (record_count == RECORD_COUNT) {
				f_output_string = "";
				f_output_string.append("#");
			}
			f_output_string.append(to_string(car->pub.speed * 3600 / 1000)).append(" ").append(
					to_string(accelerate_*100)).append(" ").append(
					to_string(car->_enginerpm)).append(" ").append(
					to_string(car->_accelCmd*100)).append(" ").append(
					to_string(car->_steerCmd*100)).append(" ").append(
					to_string(car->_yaw*100)).append(" ");
			if (mycar->isonLeft) {
				f_output_string.append("1 ").append(
						to_string(car->pub.trkPos.toLeft*100)).append(" ").append(
						to_string(car->pub.trkPos.toMiddle*100)).append(" ");
			} else {
				f_output_string.append("2 ").append(
						to_string(car->pub.trkPos.toMiddle*100)).append(" ").append(
						to_string(car->pub.trkPos.toRight*100)).append(" ");
			}
			f_output_string.append(to_string(*dist_to_ocar * 100)).append(" ").append(
					to_string(*dist_to_ocar_dlane * 100)).append(" ");
			if (car->pub.lc_signal)
				f_output_string.append("1");
			else
				f_output_string.append("2");
			f_output_string.append("\n");
			record_count--;
			if(record_count == 0) {
				f_output << f_output_string.c_str() << endl;
				record_count = RECORD_COUNT;
			}
		}
	}
//
//	cout << "#";
//	cout << s_t 		   << endl;
//	cout << car->pub.speed << " ";
//	cout << car->_enginerpm << " ";
//	cout << car->_accelCmd << " ";
//	cout << car->_steerCmd << " ";
//	cout << car->_yaw << " ";
//	if(mycar->isonLeft) {
//		cout << "1" << " ";
//		cout << car->pub.trkPos.toLeft << " ";
//		cout << car->pub.trkPos.toMiddle << " ";
//	}
//	else {
//		cout << "2" << " ";
//		cout << car->pub.trkPos.toMiddle << " ";
//		cout << car->pub.trkPos.toRight << " ";
//	}
//	if(mycar->isonLeft)
//		printf("left \n");
//	else
//		printf("right \n");
//	printf("%f %f\n", *dist_to_ocar, *dist_to_ocar_dlane);

	/* Memo : Common_drive의 호출 주기 0.02s ~ 0.022s
	 /* Hwancheol */

//	/*NaYeon*/
//			/*receive data setting */
//			*rec_steer = car->_steerCmd * 180* (-1);
//			*rec_speed = car->pub.speed * 3.6; //m/s-> km/h convert
//			*rec_rpm = car->_enginerpm;
////	/* Hwancheol */
////			*rec_acc = (onoff_Mode & (short) 2);
//			*rec_lkas = (onoff_Mode & (short) 1);
			//*rec_targetspeed = car->pub.target_speed * 3.6;
//			printf("ACC : %d\n", *rec_acc);
//			printf("LKAS : %d\n", *rec_lkas);
//			printf("TARGETSPEED: %d\n", *rec_targetspeed);
	if (s->currentTime > 1.0) {
		// thanks Christos for the following: gradual accel/brake changes for on/off controls.
		const tdble inc_rate = 0.2f;
		tdble d_brake = car->_brakeCmd - HCtx[idx]->pbrake;
		if (fabs(d_brake) > inc_rate
				&& car->_brakeCmd > HCtx[idx]->pbrake) {
			car->_brakeCmd = MIN(car->_brakeCmd,
					HCtx[idx]->pbrake + inc_rate * d_brake / fabs(d_brake));
		}
		HCtx[idx]->pbrake = car->_brakeCmd;


		if (cmd[CMD_BRAKE].type == GFCTRL_TYPE_JOY_BUT
				|| cmd[CMD_BRAKE].type == GFCTRL_TYPE_MOUSE_BUT
				|| cmd[CMD_BRAKE].type == GFCTRL_TYPE_KEYBOARD
				|| cmd[CMD_BRAKE].type == GFCTRL_TYPE_SKEYBOARD) {
			tdble d_brake = car->_brakeCmd - HCtx[idx]->pbrake;
			if (fabs(d_brake) > inc_rate
					&& car->_brakeCmd > HCtx[idx]->pbrake) {
				car->_brakeCmd = MIN(car->_brakeCmd,
						HCtx[idx]->pbrake + inc_rate * d_brake / fabs(d_brake));
			}
			HCtx[idx]->pbrake = car->_brakeCmd;
		}

		if (cmd[CMD_THROTTLE].type == GFCTRL_TYPE_JOY_BUT
				|| cmd[CMD_THROTTLE].type == GFCTRL_TYPE_MOUSE_BUT
				|| cmd[CMD_THROTTLE].type == GFCTRL_TYPE_KEYBOARD
				|| cmd[CMD_THROTTLE].type == GFCTRL_TYPE_SKEYBOARD) {
			tdble d_accel = car->_accelCmd - HCtx[idx]->paccel;
			if (fabs(d_accel) > inc_rate
					&& car->_accelCmd > HCtx[idx]->paccel) {
				car->_accelCmd = MIN(car->_accelCmd,
						HCtx[idx]->paccel + inc_rate * d_accel / fabs(d_accel));
			}
			HCtx[idx]->paccel = car->_accelCmd;
		}
	}

	if (HCtx[idx]->AutoReverseEngaged) {
		/* swap brake and throttle */
		brake = car->_brakeCmd;
		car->_brakeCmd = car->_accelCmd;
		car->_accelCmd = brake;
	}

	if (HCtx[idx]->ParamAbs) {
		if (fabs(car->_speed_x) > 10.0) {
			int i;

			tdble skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
			NORM_PI_PI(skidAng);

			if (car->_speed_x > 5 && fabs(skidAng) > 0.2)
				car->_brakeCmd = MIN(car->_brakeCmd,
						0.10 + 0.70 * cos(skidAng));

			if (fabs(car->_steerCmd) > 0.1) {
				tdble decel = ((fabs(car->_steerCmd) - 0.1)
						* (1.0 + fabs(car->_steerCmd)) * 0.6);
				car->_brakeCmd = MIN(car->_brakeCmd, MAX(0.35, 1.0 - decel));
			}

			const tdble abs_slip = 2.5;
			const tdble abs_range = 5.0;

			slip = 0;
			for (i = 0; i < 4; i++) {
				slip += car->_wheelSpinVel(i)* car->_wheelRadius(i);}
			slip = car->_speed_x - slip / 4.0f;

			if (slip > abs_slip)
				car->_brakeCmd =
						car->_brakeCmd
								- MIN(car->_brakeCmd*0.8,
										(slip - abs_slip) / abs_range);
		}
	}

	if (HCtx[idx]->ParamAsr) {
		tdble trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
		tdble angle = trackangle - car->_yaw;
		NORM_PI_PI(angle);

		tdble maxaccel = 0.0;
		if (car->_trkPos.seg->type == TR_STR)
			maxaccel = MIN(car->_accelCmd, 0.2);
		else if (car->_trkPos.seg->type == TR_LFT && angle < 0.0)
			maxaccel = MIN(car->_accelCmd, MIN(0.6, -angle));
		else if (car->_trkPos.seg->type == TR_RGT && angle > 0.0)
			maxaccel = MIN(car->_accelCmd, MIN(0.6, angle));

		tdble origaccel = car->_accelCmd;
		tdble skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
		NORM_PI_PI(skidAng);

		if (car->_speed_x > 5 && fabs(skidAng) > 0.2) {
			car->_accelCmd = MIN(car->_accelCmd, 0.15 + 0.70 * cos(skidAng));
			car->_accelCmd = MAX(car->_accelCmd, maxaccel);
		}

		if (fabs(car->_steerCmd) > 0.1) {
			tdble decel = ((fabs(car->_steerCmd) - 0.1)
					* (1.0 + fabs(car->_steerCmd)) * 0.8);
			car->_accelCmd = MIN(car->_accelCmd, MAX(0.35, 1.0 - decel));
		}

		switch (HCtx[idx]->drivetrain) {
		case D4WD:
			drivespeed = ((car->_wheelSpinVel(FRNT_RGT)+ car->_wheelSpinVel(
					FRNT_LFT)) *
			car->_wheelRadius(FRNT_LFT)+
			(car->_wheelSpinVel(REAR_RGT)+ car->_wheelSpinVel(
					REAR_LFT)) *
			car->_wheelRadius(REAR_LFT)) / 4.0;
			break;
			case DFWD:
			drivespeed = (car->_wheelSpinVel(FRNT_RGT)+ car->_wheelSpinVel(
			FRNT_LFT)) *
			car->_wheelRadius(FRNT_LFT)/ 2.0;
			break;
			default:
			drivespeed = (car->_wheelSpinVel(REAR_RGT)+ car->_wheelSpinVel(
			REAR_LFT)) *
			car->_wheelRadius(REAR_LFT)/ 2.0;
			break;

		}
		tdble slip = drivespeed - fabs(car->_speed_x);
		if (slip > 2.0)
			car->_accelCmd = MIN(car->_accelCmd,
					origaccel - MIN(origaccel-0.1, ((slip - 2.0)/10.0)));
	}

	if (speedLimiter) {
		tdble Dv;
		if (Vtarget != 0) {
			Dv = Vtarget - car->_speed_x;
			if (Dv > 0.0) {
				car->_accelCmd = MIN(car->_accelCmd, fabs(Dv / 6.0));
			} else {
				car->_brakeCmd = MAX(car->_brakeCmd, fabs(Dv / 5.0));
				car->_accelCmd = 0;
			}
		}
	}

#ifndef WIN32
#ifdef TELEMETRY
	if ((car->_laps > 1) && (car->_laps < 5)) {
		if (HCtx[idx]->lap == 1) {
			RtTelemStartMonitoring("Player");
		}
		RtTelemUpdate(car->_curLapTime);
	}
	if (car->_laps == 5) {
		if (HCtx[idx]->lap == 4) {
			RtTelemShutdown();
		}
	}
#endif

#endif

	HCtx[idx]->lap = car->_laps;

}

static tdble getAutoClutch(int idx, int gear, int newgear, tCarElt *car) {
	if (newgear != 0 && newgear < car->_gearNb) {
		if (newgear != gear) {
			HCtx[idx]->clutchtime = 0.332f - ((tdble) newgear / 65.0f);
		}

		if (HCtx[idx]->clutchtime > 0.0f)
			HCtx[idx]->clutchtime -= RCM_MAX_DT_ROBOTS;
		return 2.0f * HCtx[idx]->clutchtime;
	}

	return 0.0f;
}

/*
 * Function
 *
 *
 * Description
 *
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *	
 */
static void drive_mt(int index, tCarElt* car, tSituation *s) {
	int i;
	int idx = index - 1;
	tControlCmd *cmd = HCtx[idx]->CmdControl;

	common_drive(index, car, s);
	car->_gearCmd = car->_gear;
	/* manual shift sequential */
	if (((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT)
			&& joyInfo->edgeup[cmd[CMD_UP_SHFT].val])
			|| ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_MOUSE_BUT)
					&& mouseInfo->edgeup[cmd[CMD_UP_SHFT].val])
			|| ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD)
					&& keyInfo[cmd[CMD_UP_SHFT].val].edgeUp)
			|| ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_SKEYBOARD)
					&& skeyInfo[cmd[CMD_UP_SHFT].val].edgeUp)) {
		car->_gearCmd++;
	}

	if (((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT)
			&& joyInfo->edgeup[cmd[CMD_DN_SHFT].val])
			|| ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_MOUSE_BUT)
					&& mouseInfo->edgeup[cmd[CMD_DN_SHFT].val])
			|| ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD)
					&& keyInfo[cmd[CMD_DN_SHFT].val].edgeUp)
			|| ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_SKEYBOARD)
					&& skeyInfo[cmd[CMD_DN_SHFT].val].edgeUp)) {
		if (HCtx[idx]->SeqShftAllowNeutral || (car->_gearCmd > 1)) {
			car->_gearCmd--;
		}
	}

	/* manual shift direct */
	if (HCtx[idx]->RelButNeutral) {
		for (i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
			if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT)
					&& joyInfo->edgedn[cmd[i].val])
					|| ((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT)
							&& mouseInfo->edgedn[cmd[i].val])
					|| ((cmd[i].type == GFCTRL_TYPE_KEYBOARD)
							&& keyInfo[cmd[i].val].edgeDn)
					|| ((cmd[i].type == GFCTRL_TYPE_SKEYBOARD)
							&& skeyInfo[cmd[i].val].edgeDn)) {
				car->_gearCmd = 0;
			}
		}
	}

	for (i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
		if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[i].val])
				|| ((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT)
						&& mouseInfo->edgeup[cmd[i].val])
				|| ((cmd[i].type == GFCTRL_TYPE_KEYBOARD)
						&& keyInfo[cmd[i].val].edgeUp)
				|| ((cmd[i].type == GFCTRL_TYPE_SKEYBOARD)
						&& skeyInfo[cmd[i].val].edgeUp)) {
			car->_gearCmd = i - CMD_GEAR_N;
		}
	}

	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
		car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);

}
/*
 * Function
 *
 *
 * Description
 *
 *
 * Parameters
 *
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void drive_at(int index, tCarElt* car, tSituation *s) {
	int gear, i;
	int idx = index - 1;
	tControlCmd *cmd = HCtx[idx]->CmdControl;

	common_drive(index, car, s);
	//drive(index, car, s);
	/* shift */
	gear = car->_gear;

	if (gear > 0) {
		/* return to auto-shift */
		HCtx[idx]->manual = 0;
	}
	gear += car->_gearOffset;
	car->_gearCmd = car->_gear;

	if (!HCtx[idx]->AutoReverse) {
		/* manual shift */
		if (((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT)
				&& joyInfo->edgeup[cmd[CMD_UP_SHFT].val])
				|| ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD)
						&& keyInfo[cmd[CMD_UP_SHFT].val].edgeUp)
				|| ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_SKEYBOARD)
						&& skeyInfo[cmd[CMD_UP_SHFT].val].edgeUp)) {
			car->_gearCmd++;
			HCtx[idx]->manual = 1;
		}

		if (((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT)
				&& joyInfo->edgeup[cmd[CMD_DN_SHFT].val])
				|| ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD)
						&& keyInfo[cmd[CMD_DN_SHFT].val].edgeUp)
				|| ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_SKEYBOARD)
						&& skeyInfo[cmd[CMD_DN_SHFT].val].edgeUp)) {
			car->_gearCmd--;
			HCtx[idx]->manual = 1;
		}

		/* manual shift direct */
		if (HCtx[idx]->RelButNeutral) {
			for (i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
				if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT)
						&& joyInfo->edgedn[cmd[i].val])
						|| ((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT)
								&& mouseInfo->edgedn[cmd[i].val])
						|| ((cmd[i].type == GFCTRL_TYPE_KEYBOARD)
								&& keyInfo[cmd[i].val].edgeDn)
						|| ((cmd[i].type == GFCTRL_TYPE_SKEYBOARD)
								&& skeyInfo[cmd[i].val].edgeDn)) {
					car->_gearCmd = 0;
					/* return to auto-shift */
					HCtx[idx]->manual = 0;
				}
			}
		}

		for (i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
			if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT)
					&& joyInfo->edgeup[cmd[i].val])
					|| ((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT)
							&& mouseInfo->edgeup[cmd[i].val])
					|| ((cmd[i].type == GFCTRL_TYPE_KEYBOARD)
							&& keyInfo[cmd[i].val].edgeUp)
					|| ((cmd[i].type == GFCTRL_TYPE_SKEYBOARD)
							&& skeyInfo[cmd[i].val].edgeUp)) {
				car->_gearCmd = i - CMD_GEAR_N;
				HCtx[idx]->manual = 1;
			}
		}
	}

	/* auto shift */
	if (!HCtx[idx]->manual && !HCtx[idx]->AutoReverseEngaged) {
		tdble omega = car->_enginerpmRedLine * car->_wheelRadius(2)* 0.95;
		tdble shiftThld = 10000.0f;
		if (car->_gearRatio[gear] != 0) {
			shiftThld = omega / car->_gearRatio[gear];
		}

		if (car->pub.speed > shiftThld) {
			car->_gearCmd++;
		} else if (car->_gearCmd > 1) {
			if (car->pub.speed < (omega / car->_gearRatio[gear-1] - 4.0)) {
				car->_gearCmd--;
			}
		}

		if (car->_gearCmd <= 0) {
			car->_gearCmd++;
		}
	}

	if (HCtx[idx]->AutoReverse) {
		/* Automatic Reverse Gear Mode */
		if (!HCtx[idx]->AutoReverseEngaged) {
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x < 1.0)) {
				HCtx[idx]->AutoReverseEngaged = 1;
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		} else {
			/* currently in autoreverse mode */
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x > -1.0)
					&& (car->_speed_x < 1.0)) {
				HCtx[idx]->AutoReverseEngaged = 0;
				car->_gearCmd = CMD_GEAR_1 - CMD_GEAR_N;
			} else {
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		}
	}

	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
		car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);
}

static int pitcmd(int index, tCarElt* car, tSituation *s) {
	tdble f1, f2;
	tdble ns;if (car->_brakeCmd > 0)
		car->_accelCmd = 0;
	int idx = index - 1;

	HCtx[idx]->NbPitStops++;
	f1 = car->_tank - car->_fuel;
	if (HCtx[idx]->NbPitStopProg < HCtx[idx]->NbPitStops) {
		ns = 1.0;
	} else {
		ns = 1.0 + (HCtx[idx]->NbPitStopProg - HCtx[idx]->NbPitStops);
	}

	f2 = 0.00065
			* (curTrack->length * car->_remainingLaps
					+ car->_trkPos.seg->lgfromstart) / ns - car->_fuel;

	car->_pitFuel = MAX(MIN(f1, f2), 0);

	HCtx[idx]->LastPitStopLap = car->_laps;

	car->_pitRepair = (int) car->_dammage;

	int i;
	int key;
	tControlCmd *cmd;

	if (HCtx[idx]) {
		cmd = HCtx[idx]->CmdControl;
		for (i = 0; i < nbCmdControl; i++) {
			if (cmd[i].type == GFCTRL_TYPE_KEYBOARD
					|| cmd[i].type == GFCTRL_TYPE_SKEYBOARD) {
				key = cmd[i].val;
				keyInfo[key].state = GFUI_KEY_UP;
				keyInfo[key].edgeDn = 0;
				keyInfo[key].edgeUp = 0;
				skeyInfo[key].state = GFUI_KEY_UP;
				skeyInfo[key].edgeDn = 0;
				skeyInfo[key].edgeUp = 0;
				currentKey[key] = GFUI_KEY_UP;
				currentSKey[key] = GFUI_KEY_UP;
			}
		}
	}
	return ROB_PIT_MENU; /* The player is able to modify the value by menu */
}

/* Hwancheol */
static double calculate_CC(bool updown, tCarElt* car) {
	const double KP = 0.5;
	const double KP_2 = 1.0;
	const double TARGET_DIST = 15;

	if(temp_target_speed == -1)
		temp_target_speed = car->pub.target_speed;
	double speed_kmph = car_speed * 3600 / 1000; // convert mps to kmph 
	if (speed_kmph >= 120.0) {
		temp_target_speed = 120.0 * 1000 / 3600;
	} 
	double error = car_speed - temp_target_speed;
	double error_2 = 0.0;
	double pid = error * KP;
	/* Adaptive Cruise Control */
	if (*dist_to_ocar <= 300 && *dist_to_ocar > 0) {
		error_2 = TARGET_DIST - *dist_to_ocar;
		pid = pow(3, fabs(error_2) * KP_2) / 2;
	}
	if (updown) {
		if (error_2 < 0.0 || error < 0.0) {
			if (error_2 < 0.0 && *speed_ocar * 1.3 > car_speed) {
				float y = (-12.5) * *dist_to_ocar + 1250;
				y = max(y, 0.0f);
				temp_target_speed += *dist_to_ocar * (1 / (y + 1));
			}
			return MIN(fabs(pid), 1.0);
		}
		return 0;
	} else {
		if (error_2 > 0.0 || error > 0.0) {
			if (error_2 > 0.0) {
				temp_target_speed -= 0.1;
			}
			return min(fabs(pid), 1.0);
		}
		return 0;
	}
}
/* LDWS RETURN CODE DEFINE */
#define LDWS_BUFFER_RESET 	 0
#define LDWS_CALCULATING	 1

#define LDWS_ON_LEVEL0       2
#define LDWS_ON_LEVEL1 	 	 3
#define LDWS_ON_LEVEL2 	 	 4
#define LDWS_ON_LEVEL3 	 	 5

#define LDWS_ERROR 		     6

/* LDWS CONSTANT DEFINE */
#define INTEGRAL_TH_1	     1.0
#define INTEGRAL_TH_2	     1.5
#define INTEGRAL_TH_3	     2.0

static int ldws(bool isOnleft, double dist_to_left, double dist_to_right, double dist_to_middle) {
	double track_width = dist_to_left + dist_to_right;
	bool change_flag_l;
	bool change_flag_r;
	pre_dist_l = cur_dist_l;
	pre_dist_r = cur_dist_r;

	if(isOnleft) {
//		printf("LEFT");
		cur_dist_l = dist_to_left / (track_width / 2);
		cur_dist_r = dist_to_middle / (track_width / 2);
	}
	else {
//		printf("RIGHT");
		cur_dist_l = -dist_to_middle / (track_width / 2);
		cur_dist_r = dist_to_right / (track_width / 2);
	}
	if(fabs(cur_dist_l-pre_dist_l)/pre_dist_l*100 <= 1 || cur_dist_l >= pre_dist_l)
		change_count_l++;
	else
		change_count_l = 0;
	if(fabs(cur_dist_r-pre_dist_r)/pre_dist_r*100 <= 1 || cur_dist_r >= pre_dist_r)
		change_count_r++;
	else
		change_count_r = 0;

	if(change_count_l == 5) {
		change_count_l = 0;
		sum_error_l = 0;
		change_flag_l = true;
	}
	if(change_count_r == 5) {
		change_count_r = 0;
		sum_error_r = 0;
		change_flag_r = true;
	}


	double error_ldws_l = fabs(0.5 - cur_dist_l);
	sum_error_l += error_ldws_l;
	double error_ldws_r = fabs(0.5 - cur_dist_r);
	sum_error_r += error_ldws_r;
	if(sum_error_l >= INTEGRAL_TH_3 || sum_error_r >= INTEGRAL_TH_3) {
		printf("LDWS - level 3 ON!!!!\n");
		return LDWS_ON_LEVEL3;
	}
	else if(sum_error_l >= INTEGRAL_TH_2 || sum_error_r >= INTEGRAL_TH_2) {
		printf("LDWS - level 2 ON!!!!\n");
		return LDWS_ON_LEVEL2;
	}
	else if(sum_error_l >= INTEGRAL_TH_1 || sum_error_r >= INTEGRAL_TH_1) {
		printf("LDWS - level 1 ON!!!!\n");
		return LDWS_ON_LEVEL1;
	}
	else {
		printf("================LDWS - level 0 Stable===============\n");
		return LDWS_ON_LEVEL0;
	}
//	printf("dist(left) : %f\n", cur_dist_l);
//	printf("error sum(left) : %f\n", sum_error_l);
//	printf("dist(right) : %f\n", cur_dist_r);
//	printf("error sum(right) : %f\n", sum_error_r);
//	printf("LDWS_CALCULATING\n");
	return LDWS_CALCULATING;

}
/* Hwancheol */
