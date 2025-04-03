#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app.h"
#include "common.h"
#include "driver/gpio.h"
#include "mqtt_client.h"

/********************************************************************************************************
								Macros
********************************************************************************************************/
 

#define 	WIFI_CONNECTED_MODE_LED_ON_TIME				100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second
#define 	WIFI_CONNECTED_MODE_LED_OFF_TIME			100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second

#define 	WIFI_CONNECTED_FAILED_MODE_LED_ON_TIME				100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second
#define 	WIFI_CONNECTED_FAILED_MODE_LED_OFF_TIME				100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second

#define 	WATER_LEVEL_HIGH_MODE_LED_ON_TIME    		1000/TASK_RATE		// 1 second
#define 	WATER_LEVEL_HIGH_MODE_LED_OFF_TIME		1000/TASK_RATE		// 1 second

#define 	SOIL_HUMI_HIGH_LED_ON_TIME    		100/TASK_RATE		// 0.10Seconds //500/TASK_RATE// 0.5 second
#define 	SOIL_HUMI_HIGH_LED_OFF_TIME			2000/TASK_RATE		// 2 Seconds //1500/TASK_RATE// 1.5 second

#define 	MQTT_CONNECTED_MODE_LED_ON_TIME    		1000/TASK_RATE		// 1 second
#define 	MQTT_CONNECTED_MODE_LED_OFF_TIME		1000/TASK_RATE		// 1 second


#define 	MQTT_CONNECTION_FAILED_MODE_LED_ON_TIME				100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second
#define 	MQTT_CONNECTION_FAILED_MODE_LED_OFF_TIME			100/TASK_RATE		// 0.10 Seconds // 0.250Seconds //500/TASK_RATE// 0.5 second


/********************************************************************************************************
								Constants
********************************************************************************************************/


typedef enum led_states
{
	LED_IDLE_STATE = 0,
	WIFI_CONNECTED_MODE_LED,
	WIFI_CONNECTION_FAILED_MODE_LED,
	WATER_LEVEL_HIGH_MODE_LED,
	SOIL_HUMI_HIGH_LED,
	MQTT_CONNECTED_MODE_LED,
	MQTT_CONNECTION_FAILED_MODE_LED
} led_sm_states;

typedef enum led_blink_modes
{
	IDLE_STATE,
	WIFI_CONNECTED,
	WIFI_CONNECTION_FAILED,
	WATER_LEVEL_HIGH,
	SOIL_HUMI_HIGH,
	MQTT_CONNECTED,
	MQTT_CONNECTION_FAILED

} led_blink_patterns;


const uint8_t led_blink_pattern_table[][2] =
	{
		{WIFI_CONNECTED_MODE_LED_ON_TIME, 	WIFI_CONNECTED_MODE_LED_OFF_TIME},
		{WIFI_CONNECTED_FAILED_MODE_LED_ON_TIME,	WIFI_CONNECTED_FAILED_MODE_LED_OFF_TIME},
		{WATER_LEVEL_HIGH_MODE_LED_ON_TIME,		WATER_LEVEL_HIGH_MODE_LED_OFF_TIME	},
 		{SOIL_HUMI_HIGH_LED_ON_TIME,			SOIL_HUMI_HIGH_LED_OFF_TIME},
		{MQTT_CONNECTED_MODE_LED_ON_TIME,		MQTT_CONNECTED_MODE_LED_OFF_TIME},
		{MQTT_CONNECTION_FAILED_MODE_LED_ON_TIME,	MQTT_CONNECTION_FAILED_MODE_LED_OFF_TIME}

	};

/********************************************************************************************************
								variables
********************************************************************************************************/

uint8_t push_button_long_press_det;

uint32_t push_button_sense_timer;

  
static uint32_t led_control_timer;
static led_sm_states led_control_state;


extern bool level_sensor_status;
extern bool soil_sensor_status;
uint8_t wdi_state;

extern  esp_mqtt_client_handle_t client;
 
/********************************************************************************************************
 *									avt pb_led_status_control
********************************************************************************************************/
// void avt_pb_led_status_control(void)
// {

// 	switch(led_control_state)
// 	{
// 		case LED_IDLE_STATE:

// 			if (true == 1)//is_wifi_status_connected())
// 			{
// 				//LED made off
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_LOW);
// 				// load led OFF time
// 				//led_control_timer = BLE_MODE_LED_OFF_TIME;
// 				//move BLE Contorl Mode
// 				//led_control_state = BLE_LED_BLINK_STATE1;
// 			}

// 			else if (true == 2)// alarm_led_control_flag)
// 			{
// 				//LED made off
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_LOW);
// 				// load led OFF time
// 				//led_control_timer = ALARM_MODE_LED_OFF_TIME;
// 				//move BLE Contorl Mode
// 			//	led_control_state = ALARM_LED_BLINK_STATE1;
// 				#ifdef __DEBUG_AVT_APP__
// 				printf("AVT ALARM LED OFF *******\n");
// 				#endif
// 			}
// 			else if (0x01 == push_button_long_press_det)
// 			{
// 				//LED made off
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_LOW);
// 			}

// 			else
// 			{
// 				//LED made ON
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_HIGH);
// 				// do nothing
// 			}
// 		break;
// 		case WIFI_CONNECTED:
// 			if(0x00 != led_control_timer)
// 			{
// 				led_control_timer--;
// 			}
// 			if (0x0000 == led_control_timer)
// 			{
// 				//LED made on
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_HIGH);
// 				// load led ON Time
// 				//led_control_timer = ALARM_MODE_LED_ON_TIME;
// 				//move Alarm Contorl Mode2
// 				//led_control_state = ALARM_LED_BLINK_STATE2;
// 				#ifdef __DEBUG_AVT_APP__
// 				printf("AVT ALARM LED ON *******\n");
// 				#endif
// 			}

// 		break;

// 		case WIFI_CONNECTION_FAILED:

// 			if(0x00 != led_control_timer)
// 			{
// 				led_control_timer--;
// 			}
// 			if (0x0000 == led_control_timer)
// 			{	
// 				//move to IDLE state to check other inputs
// 				led_control_state = LED_IDLE_STATE;
// 			}

// 		break;

// 		case WATER_LEVEL_HIGH:

// 			if(0x00 != led_control_timer)
// 			{
// 				led_control_timer--;
// 			}
// 			if (0x0000 == led_control_timer)
// 			{
// 				//LED made on
// 				output_control_function(LED_CNTRL_OUTPUT, GPIO_HIGH);
// 				// load led ON Time
// 				led_control_timer = WATER_LEVEL_HIGH_MODE_LED_ON_TIME;
// 				//move BLE Contorl Mode2
// 				led_control_state = WATER_LEVEL_HIGH;
// 			}
// 			/* Monitor alarm LED control flag */
// 			//monitor_alarm_led_control_flag();
// 		break;

// 		case SOIL_HUMI_HIGH:

// 			if(0x00 != led_control_timer)
// 			{
// 				led_control_timer--;
// 			}
// 			if (0x0000 == led_control_timer)
// 			{
			 
// 					//LED made off
// 					output_control_function(LED_CNTRL_OUTPUT, GPIO_LOW);
// 					// load led control timer
// 				//	led_control_timer = BLE_MODE_LED_OFF_TIME;
// 					//move BLE Contorl Mode
// 				//	led_control_state = BLE_LED_BLINK_STATE1;
// 			}
// 			else  // stop BLE led mode
// 			{
// 				//move to IDLE state to check other inputs
// 				led_control_state = LED_IDLE_STATE;
				
// 			}

// 		break;

// 		case MQTT_CONNECTED: // MEMORY_MODE_LED_BLINK_STATE1:
// 			// Need to include logic
// 			led_control_state = LED_IDLE_STATE;
// 		break;

// 		case MQTT_CONNECTION_FAILED: // MEMORY_MODE_LED_BLINK_STATE2:
// 			// Need to include logic
// 			led_control_state = LED_IDLE_STATE;
// 		break;

// 		default:
// 			led_control_state = LED_IDLE_STATE;
// 		break;
//     }
// }
 

/********************************************************************************************************
								appoperation_Control
********************************************************************************************************/
void app_operation_Control()
{
	static uint32_t motor_run_timer = 0;
	static uint32_t motor_in_run_timer =0;
    bool motor_running = false;
	bool water_in_motor_running = false;
   
    while (1)
    {
        // Check if level sensor is HIGH
        if (level_sensor_status && !motor_running)
        {
            printf("Water Level HIGH - Starting Motor\n");
            gpio_set_level(GPIO_OUTPUT_PUMP_INT1, GPIO_HIGH);  // Turn ON the motor
			gpio_set_level(GPIO_OUTPUT_PUMP_INT2, GPIO_LOW);  // Turn ON the motor
			esp_mqtt_client_publish(client, "velanv5144@gmail.com/discharge_motor_status","1", 0, 0, 0);
            motor_running = true;
            motor_run_timer = 200;  // 10 sec / 50ms task interval = 200 iterations
        }

        // Countdown for 10 seconds
        if (motor_running)
        {
            if (motor_run_timer > 0)
            {
                motor_run_timer--;
            }
            else
            {
                printf("Stopping Motor\n");
				esp_mqtt_client_publish(client, "velanv5144@gmail.com/irrigation_motor","0", 0, 0, 0);
                gpio_set_level(GPIO_OUTPUT_PUMP_INT1, GPIO_LOW);  // Turn OFF the motor
				gpio_set_level(GPIO_OUTPUT_PUMP_INT2, GPIO_LOW);  // Turn OFF the motor

				esp_mqtt_client_publish(client, "velanv5144@gmail.com/discharge_motor_status","0", 0, 0, 0);

                motor_running = false;
            }
        }


		// Check if level sensor is HIGH
		if (soil_sensor_status  && !water_in_motor_running )
		{
			printf("soil humdity low feeding water- Starting Motor\n");
			gpio_set_level(GPIO_OUTPUT_WATER_VALUE_INT1, GPIO_HIGH);  // Turn ON the motor
			gpio_set_level(GPIO_OUTPUT_WATER_VALUE_INT2, GPIO_LOW);  // Turn OFF the motor
			esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_in_motor_status","1", 0, 0, 0);
			water_in_motor_running = true;
			motor_in_run_timer = 200;  // 10 sec / 50ms task interval = 200 iterations
		}
		// Countdown for 10 seconds
		if (water_in_motor_running)
		{
			if (motor_in_run_timer > 0)
			{
				motor_in_run_timer--;
			}
			else
			{
				printf("soil humdity feeding water is done Stopping Motor\n");
				esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_in_motor_status","0", 0, 0, 0);
				gpio_set_level(GPIO_OUTPUT_WATER_VALUE_INT1, GPIO_LOW);  // Turn OFF the motor
				gpio_set_level(GPIO_OUTPUT_WATER_VALUE_INT2, GPIO_LOW);  // Turn OFF the motor
				water_in_motor_running = false;
			}
		}

		vTaskDelay( TASK_RATE / portTICK_PERIOD_MS);
    }

}
void app_init(void)
{

	xTaskCreate((void *)app_operation_Control, "avt_app_5s_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);

}



/********************************************************************************************************
 *							Revision History
 *
 * Rev.     SCR        	 Date         By            Description
 *-----   -------      --------      ----       ---------------------------
 * 1.0             	  24 Sep 2019    TNK		Initial version

 ********************************************************************************************************/
