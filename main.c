#include <stdio.h>  
#include "string.h"

#include "esp_system.h"
#include "esp_log.h"

#include "driver/uart.h"

#include "r307.h"

uint8_t esp_chip_id[6];                                     //++ Array to get Chip ID
char mac_address[20];                                       //++ Array to store Mac Address

char default_address[4] = {0xFF, 0xFF, 0xFF, 0xFF};         //++ Default Module Address is FF:FF:FF:FF
char default_password[4] = {0x00, 0x00, 0x00, 0x00};        //++ Default Module Password is 00:00:00:00

void app_main(void)
{
    r307_init();                                            //++ Initializing UART for r307 Module

    esp_efuse_mac_get_default(esp_chip_id);
    sprintf(mac_address, "%02x:%02x:%02x:%02x:%02x:%02x", esp_chip_id[0], esp_chip_id[1], esp_chip_id[2], esp_chip_id[3], esp_chip_id[4], esp_chip_id[5]);
    printf("MAC Address is %s\n", mac_address);             //++ Get the MAC Address of current ESP32
    
    uint8_t confirmation_code = 0;
    confirmation_code = VfyPwd(default_address, default_password);      //++ Performs Password Verification with Fingerprint Module

    if(confirmation_code == 0x00)
    {
        printf("R307 FINGERPRINT MODULE DETECTED\n");
    }

}
