/***************************************************************************

    file        : racemain.h
    created     : Sat Nov 16 12:14:57 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemain.h,v 1.3 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: racemain.h,v 1.3 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACEMAIN_H_
#define _RACEMAIN_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

extern int  ReRaceEventInit(void);
extern int  RePreRace(void);
extern int  ReRaceStart(void);
extern int  ReRaceStop(void);
extern int  ReRaceEnd(void);
extern int  RePostRace(void);
extern int  ReEventShutdown(void);

union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short int *array;
};

extern void init_shared_memory();
extern void delete_shared_memory();
extern int shmid;
extern char* shared_memory[1024];
extern int skey;
extern char* send_data[1024];

extern void set_tcpip();
extern int my_socket;
extern struct sockaddr_in addr;
#define TCPIP_PORT_NUM 6342
#define TCPIP_SERVER_IP "192.168.56.1"
#endif /* _RACEMAIN_H_ */ 



