/***************************************************************************

    file        : raceengine.h
    created     : Sat Nov 23 09:35:21 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: raceengine.h,v 1.4 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: raceengine.h,v 1.4 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACEENGINE_H_
#define _RACEENGINE_H_
#include <raceman.h>


extern void ReStart(void);
extern void ReStop(void);
extern int  ReUpdate(void);
extern void ReTimeMod (void *vcmd);

extern void changeMode_LKAS(void *);
extern void changeMode_CC(void *);
extern void turn_lc_signal(void *);
extern void record(void *);
extern void berniw_speed_up(void *);
extern void berniw_speed_down(void *);
extern void select_car_2(void*);
extern void select_car_3(void*);
extern void turn_signal_l(void*);
extern void turn_signal_r(void*);
extern short onoff_Mode;
extern tRmInfo *ReInfo;

/* Hanieum */
extern int capture_count;
extern unsigned char *img_arr[2];
extern int ldws_value;
/* Hanieum */
extern float steering;
#endif /* _RACEENGINE_H_ */ 



