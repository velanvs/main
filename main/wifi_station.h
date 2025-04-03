/********************************************************************************************************
 * FILE: wifi_station.h
 ********************************************************************************************************
 * Copyright 2019 ACO Systems, Inc., All Rights Reserved.
 * ACO System Confidential
 ********************************************************************************************************
 * DESCRIPTION: This is header file for wifi station.
 *
 * ABBREVIATIONS: None
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *   Requirements Document(s):
 *
 * DEVIATIONS FROM STANDARDS:None
*********************************************************************************************************/
#ifndef WIFI_STATION_H_
#define WIFI_STATION_H_


/*********************************************************************************************************
								Macros
*********************************************************************************************************/
/* The WiFi configuration can be set via project configuration menu
	if not just change the below entries to strings with the config you want
	example #define EXAMPLE_WIFI_SSID 	"mywifissid"
*/
 
void wifi_init_sta(void);

#endif /* WIFI_STATION */
/********************************************************************************************************
 *							Revision History
 *
 * Rev.     SCR        	 Date         By            Description
 *-----   -------      --------      ----       ---------------------------
 * 1.0             	  20 Nov 2020    TNK		Initial version

 ********************************************************************************************************/
