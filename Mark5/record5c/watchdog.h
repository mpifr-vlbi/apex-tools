/***************************************************************************
 *   Copyright (C) 2010 by Walter Brisken                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id: watchdog.h 3854 2011-10-09 03:30:18Z WalterBrisken $
// $HeadURL: https://svn.atnf.csiro.au/difx/applications/mk5daemon/trunk/mk5dir/watchdog.h $
// $LastChangedRevision: 3854 $
// $Author: WalterBrisken $
// $LastChangedDate: 2011-10-09 05:30:18 +0200 (Sun, 09 Oct 2011) $
//
//============================================================================

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include <pthread.h>
#include <time.h>
#include <xlrapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
#define WATCHDOG_PRINT 1
#else
#define WATCHDOG_PRINT 0
#endif

extern time_t watchdogTime;
extern int watchdogVerbose;
extern char watchdogStatement[256];
extern pthread_mutex_t watchdogLock;
extern char watchdogXLRError[XLR_ERROR_LENGTH+1];
extern int watchdogXLRErrorCode;

/* Macro to run "statement" but set a thread to watch to make sure it doesn't take too long */
#define WATCHDOG(statement) \
{ \
	if(WATCHDOG_PRINT) printf("WD[[ %s ]]\n", #statement); \
	pthread_mutex_lock(&watchdogLock); \
	watchdogTime = time(0); \
	strcpy(watchdogStatement, #statement); \
	if(watchdogVerbose > 1) printf("Executing (at time %d): %s\n", (int)(watchdogTime), watchdogStatement); \
	pthread_mutex_unlock(&watchdogLock); \
	statement; \
	pthread_mutex_lock(&watchdogLock); \
	if(watchdogVerbose > 2) printf("Executed (in %d seconds): %s\n", (int)(time(0)-watchdogTime), watchdogStatement); \
	watchdogTime = 0; \
	watchdogStatement[0] = 0; \
	pthread_mutex_unlock(&watchdogLock); \
}

/* same as WATCHDOG but performs a return -1 if the return value is not XLR_SUCCESS */
#define WATCHDOGTEST(statement) \
{ \
	if(WATCHDOG_PRINT) printf("WD[[ %s ]]\n", #statement); \
	XLR_RETURN_CODE watchdogRC; \
	pthread_mutex_lock(&watchdogLock); \
	watchdogTime = time(0); \
	strcpy(watchdogStatement, #statement); \
	if(watchdogVerbose > 1) printf("Executing (at time %d): xlrRC = %s\n", (int)(watchdogTime), watchdogStatement); \
	pthread_mutex_unlock(&watchdogLock); \
	watchdogRC = (statement); \
	pthread_mutex_lock(&watchdogLock); \
	if(watchdogVerbose > 2) printf("Executed (in %d seconds): xlrRC = %s\n", (int)(time(0)-watchdogTime), watchdogStatement); \
	watchdogTime = 0; \
	if(watchdogRC == XLR_SUCCESS) \
	{ \
		watchdogStatement[0] = 0; \
	} \
	pthread_mutex_unlock(&watchdogLock); \
	if(watchdogRC != XLR_SUCCESS) \
	{ \
		watchdogXLRErrorCode = XLRGetLastError(); \
		XLRGetErrorMessage(watchdogXLRError, watchdogXLRErrorCode); \
		fprintf(stderr, "%s failed.\nError: %s\n", #statement, watchdogXLRError); \
		return -1; \
	} \
}

void setWatchdogVerbosity(int v);

void setWatchdogTimeout(int timeout);

int initWatchdog();

void stopWatchdog();

#ifdef __cplusplus
}
#endif

#endif
