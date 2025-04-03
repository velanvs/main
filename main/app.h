/********************************************************************************************************
 * FILE: avt_app.h
 ********************************************************************************************************
 * Copyright 2019 ACO Systems, Inc., All Rights Reserved.
 * ACO System Confidential
 ********************************************************************************************************
 * DESCRIPTION: This is header file for avt_app.
 *
 * ABBREVIATIONS: None
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *   Requirements Document(s):
 *
 * DEVIATIONS FROM STANDARDS:None
*********************************************************************************************************/
#ifndef AVT_APP_H_
#define AVT_APP_H_

/*********************************************************************************************************
								Macros
*********************************************************************************************************/


#define SW_ACTIVE				1
#define SW_DEACTIVE				0

#define SW_ERROR_ACTIVE			1
#define SW_ERROR_DEACTIVE		0

#define TASK_RATE				10			// 50 ms Task rate

#define	PB_BUTTON_PRESS_LIMIT		10000/TASK_RATE	// value is 10 seconds
#define	PB_BUTTON_ERROR_DET_LIMIT	60000/TASK_RATE	// value is 60 seconds

#define LVL_SWITCH_ERROR_DET_LIMIT	60000/TASK_RATE	// value is 60 seconds

/*********************************************************************************************************
								Global variable declaration
*********************************************************************************************************/

 
 
/*********************************************************************************************************
								Global function declaration
*********************************************************************************************************/
extern void gpio_init(void);
 

extern void app_init(void);
 
extern void flush_operation_Control(void);
extern void pb_led_status_control(void);
extern void update_flush_progress_status(void);
 
extern void update_pb_long_press_detection(uint8_t val);
 
extern void toggle_wdi_output(void);

 
 

#endif /* AVT_APP_H_ */
/********************************************************************************************************
 *							Revision History
 *
 * Rev.     SCR        	 Date         By            Description
 *-----   -------      --------      ----       ---------------------------
 * 1.0             	  24 Sep 2019    TNK		Initial version

 ********************************************************************************************************/
