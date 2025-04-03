 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "app.h"
#include "wifi_station.h"

#include "common.h"
#include "mqtt_client.h"

/********************************************************************************************************
								Macros
 ********************************************************************************************************/
//#define		 __GPIO_FREE_STACK_SPACE___              /*!< By enabling we can find the task free stack space */
#define 	GPIO_INPUT_LEVEL_SENSOR		35   	//Pin Number 7
#define 	GPIO_INPUT_SOIL_BUTTON		32   	//Pin Number 8
 

#define 	MAX_DIGITAL_INPUTS			2


#define 	GPIO_OUTPUT_LED				2		//Pin Number 27



#define 	MAX_DIGITAL_OUTPUTS			5     // 6 outputs + 1 from rs485_DE

#define 	PULL_UP_ENABLE				1
#define 	PULL_UP_DISABLE				0

#define 	PULL_DOWN_ENABLE			1
#define 	PULL_DOWN_DISABLE			0

#define 	ESP_INTR_FLAG_DEFAULT 		0

 
#define GPIO_INPUT_IO_0     35 // CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1     34 //CONFIG_GPIO_INPUT_1

#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
 

bool level_sensor_status=0;
extern  esp_mqtt_client_handle_t client;

/********************************************************************************************************
								Constants
 ********************************************************************************************************/
	/* Input Channel Configuration  */
const uint8_t input_config_table[MAX_DIGITAL_INPUTS][6]=
{
   /*  Input Channel       		Interrupt Enable				pull up,	 	Active State,	#of samples		X 0f n samples	*/
   {GPIO_INPUT_LEVEL_SENSOR,	GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		GPIO_HIGH,			5,				3	},
   {GPIO_INPUT_SOIL_BUTTON,		GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		GPIO_HIGH,			5,				3	},
 
};

	/* output Channel Configuration  */
/* this  "output_config_table" mapped with ENum present in avt_app.h. this is one to one map.
any changes in table, needs update in enum order  */
const uint8_t output_config_table[MAX_DIGITAL_OUTPUTS][5]=
{
   /*  Input Channel       		Interrupt Enable		pull up					Pull Down			init Value	*/
   {GPIO_OUTPUT_PUMP_INT1,		GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
   {GPIO_OUTPUT_PUMP_INT2,      GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
   {GPIO_OUTPUT_LED,		    GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
   {GPIO_OUTPUT_WATER_VALUE_INT1,	GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH/*GPIO_HIGH*/},
   {GPIO_OUTPUT_WATER_VALUE_INT2,	GPIO_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
 //  {RS485_DE_PIN,			GPIO_PIN_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
//    {GPIO_OUTPUT_WDI,		GPIO_PIN_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH},
//    {GPIO_OUTPUT_SV4_REL,	GPIO_PIN_INTR_DISABLE,		PULL_UP_DISABLE,		PULL_DOWN_DISABLE,	GPIO_HIGH}
};

/********************************************************************************************************
								variables
 ********************************************************************************************************/

typedef struct input_pin_struct
{
 	uint8_t pin_num;
  	uint8_t active_sample_count;
	uint8_t sampling_counter;
	uint8_t pin_pre_value;
	uint8_t pin_cur_value;
}input_struct;


typedef struct input_switch_parameters
{
	uint8_t switch_active_Status;
	uint8_t switch_fault_status;
	uint32_t switch_timer;
}ip_sw_param;


input_struct ip_pin_struct[MAX_DIGITAL_INPUTS];


/* variable use to update the stat sequence*/
uint8_t stat_alarm_flag = 0xff;

int last_state=0;

static QueueHandle_t gpio_evt_queue = NULL;
/********************************************************************************************************
								Function Declarations
 ********************************************************************************************************/
void input_pin_status_debounce_read(void);
 
 
 
/********************************************************************************************************
								gpio_task_example
 ********************************************************************************************************/



static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}
void avt_task(void* arg)
{
    while(1)
	{
		input_pin_status_debounce_read();  	// hardware input debounce

		uint32_t io_num;
		for (;;) {
			if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
				printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
				if ( (io_num ==35) && ( 1 == gpio_get_level(io_num)) )
				{
					esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","1", 0, 0, 0);
					level_sensor_status= WATER_LEVEL_ALERM;
				}
				else if ( (io_num ==35) && ( 0 == gpio_get_level(io_num)) )
				{
					esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","0", 0, 0, 0);
					level_sensor_status= WATER_LEVEL_NORMAL;
				}

			}
		}
    }
}

/********************************************************************************************************
								gpio_pins_init Function
********************************************************************************************************/
void gpio_pins_init(void)
{
	uint8_t j;
	gpio_config_t io_conf;
  
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
 
 
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO16/10
    io_conf.pin_bit_mask = ( (1ULL<<GPIO_OUTPUT_PUMP_INT1) |(1ULL<<GPIO_OUTPUT_LED)  |(1ULL<<GPIO_OUTPUT_PUMP_INT2)  |(1ULL <<GPIO_OUTPUT_WATER_VALUE_INT1) |(1ULL <<GPIO_OUTPUT_WATER_VALUE_INT2) )  ;//GPIO_OUTPUT_PIN_SEL;

	//disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;

	gpio_set_level(output_config_table[0][0], output_config_table[0][4]);
	gpio_set_level(output_config_table[1][0], output_config_table[1][4]);
	gpio_set_level(output_config_table[2][0], output_config_table[2][4]);
	gpio_set_level(output_config_table[3][0], output_config_table[3][4]);
	gpio_set_level(output_config_table[4][0], output_config_table[4][4]);
 

    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

/********************************************************************************************************
								input_pin_status_debounce_read
********************************************************************************************************/
void input_pin_status_debounce_read(void)
{

    uint8_t j;

    for (j = 0; j < MAX_DIGITAL_INPUTS; j++)
    {
        ip_pin_struct[j].pin_num = input_config_table[j][0];

        // Read initial pin state
        uint8_t stable_state = gpio_get_level(ip_pin_struct[j].pin_num);
        
        // Introduce a debounce delay
        vTaskDelay(pdMS_TO_TICKS(50));  

        // Read the pin again
        if (stable_state == gpio_get_level(ip_pin_struct[j].pin_num))
        {
            ip_pin_struct[j].pin_cur_value = stable_state;
        }

        if (ip_pin_struct[j].pin_cur_value != ip_pin_struct[j].pin_pre_value)
        {
            if (ip_pin_struct[j].pin_cur_value == GPIO_HIGH)
            {
			
				printf("Switch Pressed (Stable Detection)\n\r");
				level_sensor_status= WATER_LEVEL_ALERM; 
				
				esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","1", 0, 0, 0);
            }
            else
            {
				esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","0", 0, 0, 0);
				level_sensor_status= WATER_LEVEL_NORMAL;
				printf("Switch Released\n\r");
				
            }
            ip_pin_struct[j].pin_pre_value = ip_pin_struct[j].pin_cur_value;
        }
    }

}

/********************************************************************************************************
								gpio_init
********************************************************************************************************/
void gpio_init(void)
{
	gpio_pins_init();

	//start gpio task

    // xTaskCreate((void *)avt_task, "avt_task", 1024*4, NULL, configMAX_PRIORITIES-1, NULL);


	//   //install gpio isr service
	//   gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	//   //hook isr handler for specific gpio pin
	//   gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
	//   gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
  
	//   //remove isr handler for gpio number.
	//   gpio_isr_handler_remove(GPIO_INPUT_IO_0);
	//   //hook isr handler for specific gpio pin again
	//   gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
  
	  printf("Minimum free heap size: %"PRIu32" bytes\n", esp_get_minimum_free_heap_size());
}
 

/********************************************************************************************************
 *							Revision History
 *
 * Rev.     SCR        	 Date         By            Description
 *-----   -------      --------      ----       ---------------------------
 * 1.0             	  18 Sep 2019    TNK		Initial version

 ********************************************************************************************************/
