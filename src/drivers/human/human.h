/***************************************************************************

    file        : human.h
    created     : Sat May 10 19:12:46 CEST 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: human.h,v 1.3.2.3 2013/08/29 20:03:44 berniw Exp $                                  

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: human.h,v 1.3.2.3 2013/08/29 20:03:44 berniw Exp $
*/

#ifndef _HUMAN_H_
#define _HUMAN_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "pref.h"
#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>
#include <math.h>
#include "spline.h"
#include "trackdesc.h"
#include "mycar.h"
#include "pathfinder.h"


#define HUMAN_SECT_PRIV	"human private"
#define HUMAN_ATT_PITENTRY	"pitentry"
#define HUMAN_ATT_PITEXIT	"pitexit"
#define HUMAN_ATT_AMAGIC	"caero"
#define HUMAN_ATT_FMAGIC	"cfriction"
#define HUMAN_ATT_FUELPERLAP "fuelperlap"


typedef struct HumanContext
{
	int		NbPitStops;
	int		LastPitStopLap;
	int 	AutoReverseEngaged;
	tdble	Gear;
	tdble	distToStart;
	tdble	clutchtime;
	tdble	clutchdelay;
	tdble	ABS;
	tdble	AntiSlip;
	int		lap;
	tdble	prevLeftSteer;
	tdble	prevRightSteer;
	tdble	paccel;
	tdble	pbrake;
	int		manual;
	int		Transmission;
	int		NbPitStopProg;
	int		ParamAsr;
	int		ParamAbs;
	int		RelButNeutral;
	int		SeqShftAllowNeutral;
	int		AutoReverse;
	int		drivetrain;
	int		autoClutch;
	tControlCmd	*CmdControl;
	int		MouseControlUsed;
	int		lightCmd;

} tHumanContext;


extern tHumanContext *HCtx[];

extern int joyPresent;

inline double queryAngleToTrack(tCarElt * car)
{
	double angle = RtTrackSideTgAngleL(&(car->_trkPos)) - car->_yaw;
	NORM_PI_PI(angle);
	return angle;
}


inline double sign(double number)
{
	return (number >= 0.0) ? 1.0 : -1.0;
}


inline double sqr(double x)
{
	return x*x;
}


/* compute the radius given three points */
inline double radius(double x1, double y1, double x2, double y2, double x3, double y3)
{
	double dx1 = x2 - x1;
	double dy1 = y2 - y1;
	double dx2 = x3 - x2;
	double dy2 = y3 - y2;

	//double z = (dy1*dx2 - dx1*dy2);
	double z = dx1*dy2 - dy1*dx2;

	if (z != 0.0) {
		double k = (dx2*(x3-x1) - dy2*(y1-y3))/z;
		return sign(z)*sqrt((1.0+k*k)*(dx1*dx1+dy1*dy1))/2.0;
	} else {
		return FLT_MAX;
	}
}
#endif /* _HUMAN_H_ */ 



