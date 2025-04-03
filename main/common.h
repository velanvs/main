#include <stdio.h>


#define WATER_LEVEL_ALERM  1

#define WATER_LEVEL_NORMAL 0  

#define 	GPIO_OUTPUT_PUMP_INT1			12		//Pin Number 9
#define 	GPIO_OUTPUT_PUMP_INT2			13		//Pin Number 9

#define     GPIO_OUTPUT_WATER_VALUE_INT1     26     
#define     GPIO_OUTPUT_WATER_VALUE_INT2     27    

#define ENABLE_MOTOR 1
#define DISABLE_MOTOR 0

#define GPIO_HIGH				1
#define GPIO_LOW				0

void gpio_init(void);
void servo_init(void);

void app_init(void);

void soil_mosture_init(void);