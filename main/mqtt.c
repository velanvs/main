/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
// #include <stdint.h>
// #include <stddef.h>
// #include <string.h>
// #include "esp_wifi.h"
// #include "esp_system.h"
// #include "esp_event.h"
// #include "esp_netif.h"
 

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

// #include "lwip/sockets.h"
// #include "lwip/dns.h"
// //#include "lwip/netdb.h"

// #include "esp_log.h"
// #include "mqtt_client.h"
// #include "esp_mac.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
 

#include "esp_log.h"
#include "mqtt_client.h"

//static const char *TAG = "MQTT_EXAMPLE";


extern char mqtt_humidity_buffer[12] ;
extern char mqtt_soil_sensor_adc_bufffer[12]; 
 
esp_mqtt_client_handle_t client;
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
     //   ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
   printf( "Event dispatched from event loop base=%s, event_id=%ld", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
      printf( "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "velanv5144@gmail.com/rx", "data_3", 0, 1, 0);
      //  printf( "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "velanv5144@gmail.com/", 0);
        printf( "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "velanv5144@gmail.com/button", 0);
        printf( "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_publish(client, "velanv5144@gmail.com/ota_prog", "0", 0, 1, 0);
      //  printf( "sent publish successful, msg_id=%d", msg_id);

        // msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        // printf( "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        printf( "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        printf( "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "velanv5144@gmail.com/test", "data", 0, 0, 0);
      //  printf( "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        printf( "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        printf( "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        printf( "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        printf( "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            printf( "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        printf( "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_fun()
{

    while (1)
    {

        
        printf("\n");
    
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }

}
void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
       
        .broker.address.uri =(const char *)"mqtt://www.maqiatto.com:1883",// CONFIG_BROKER_URL,
        .credentials.username =(const char *)"velanv5144@gmail.com",
        .credentials.authentication.password =(const char *)"12345678",
        .broker.address.port =1883,

    };
#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt_cfg.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);


    xTaskCreate(mqtt_fun,"mqtt", 1024 *2 ,NULL,24,NULL);

}




/********************************************************************************************************
 *							Revision History
 *
 * Rev.     SCR        	 Date         By            Description
 *-----   -------      --------      ----       ---------------------------
 * 1.0             	  31 JULY 2023    Velan		Initial version

 ********************************************************************************************************/
