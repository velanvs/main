 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "common.h"

#include "mqtt_client.h"


/********************************************************************************************************
								Macros
********************************************************************************************************/
#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   128          //Multisampling


/********************************************************************************************************
								enum
********************************************************************************************************/
typedef struct
{
    uint16_t raw_value;
    float calibrated_value;
} CalibrationPoint;


/********************************************************************************************************
								variables
********************************************************************************************************/
esp_adc_cal_characteristics_t *adc_chars;
esp_adc_cal_characteristics_t *adc_chars1;

static const adc_channel_t channel0 = ADC1_CHANNEL_3;     //GPIO39 if ADC1, GPIO14 if ADC2

static const adc_channel_t channel1 = ADC1_CHANNEL_0;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;
 
bool soil_sensor_status =0;

char mqtt_humidity_buffer [12];
char mqtt_soil_sensor_adc_bufffer[12];

char mqtt_water_sensor_adc_bufffer[12];

extern bool level_sensor_status;

extern esp_mqtt_client_handle_t client;
/********************************************************************************************************
 *						read_vacuum_sensor
*******************************************************************************************************/
#define ADC_DRY  3000  // ADC value when soil is dry
#define ADC_WET  1000  // ADC value when soil is fully wet

void read_soil_sensor()
{
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }

    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }

    if (unit == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channel0, atten);
    }

    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
  
    while (1)
    {
        uint32_t adc_reading = 0;
        uint32_t adc_reading2 = 0;

        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)channel0);
            }
        }

        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unit == ADC_UNIT_1) {
                adc_reading2 += adc1_get_raw((adc1_channel_t)channel1);
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        adc_reading2 /= NO_OF_SAMPLES;
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        uint32_t voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars);       
        

        if (adc_reading2 > 1000)
        {
            level_sensor_status =WATER_LEVEL_ALERM;				
			esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","1", 0, 0, 0);
        }
        else if ( adc_reading2 < 1000)
        {
            level_sensor_status =WATER_LEVEL_NORMAL;	
            esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_status","0", 0, 0, 0);
        }
        
        // Ensure adc_reading is withicn the valid range
        if (adc_reading > ADC_DRY)
        {
            adc_reading = ADC_DRY;

        }
        if (adc_reading < ADC_WET)
        {
            adc_reading = ADC_WET;
        }

        // Compute humidity percentage
        int humidity = ((ADC_DRY - adc_reading) * 100) / (ADC_DRY - ADC_WET);
        
        snprintf(mqtt_soil_sensor_adc_bufffer, sizeof(mqtt_soil_sensor_adc_bufffer), "%ld", adc_reading);
        snprintf(mqtt_humidity_buffer, sizeof(mqtt_humidity_buffer), "%d", humidity);

        snprintf(mqtt_water_sensor_adc_bufffer, sizeof(mqtt_water_sensor_adc_bufffer), "%ld", adc_reading2);

        esp_mqtt_client_publish(client, "velanv5144@gmail.com/soil_humidity",(char *)mqtt_humidity_buffer , 0, 0, 0);
        printf("\n");

        esp_mqtt_client_publish(client, "velanv5144@gmail.com/soil_sensor_adc", (char *)mqtt_soil_sensor_adc_bufffer, 0, 0, 0);
        esp_mqtt_client_publish(client, "velanv5144@gmail.com/water_level_sensor_adc", (char *)mqtt_soil_sensor_adc_bufffer, 0, 0, 0);
       
        if(adc_reading >= 3000)
        {
            esp_mqtt_client_publish(client, "velanv5144@gmail.com/soil_humidity_alert","1", 0, 0, 0);
            soil_sensor_status = ENABLE_MOTOR;
        }
        else if(adc_reading < 2500)
        {
            esp_mqtt_client_publish(client, "velanv5144@gmail.com/soil_humidity_alert","0", 0, 0, 0);
            soil_sensor_status = DISABLE_MOTOR;
        }
        printf("Soil Moisture: %d%%, Voltage: %ld mV, ADC Value: %ld \n\r", humidity, voltage, adc_reading );

        printf("water level Voltage: %ld mV, ADC Value 2: %ld  ", voltage2, adc_reading2 );

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void soil_mosture_init(void)
{
    xTaskCreate(read_soil_sensor,"adc_read",1024*2,NULL,24,NULL);
}



