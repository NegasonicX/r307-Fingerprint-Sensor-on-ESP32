#include <stdint.h>
#include "string.h"

#include "esp_log.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "r307.h"

#define TXD_PIN (GPIO_NUM_17)                   //++ TX & RX pins for UART 1 on ESP32 Devkit v1
#define RXD_PIN (GPIO_NUM_16)

static const int RX_BUF_SIZE = 2048;            //++ UART RX Buffer Size
static const char *R307_TX = "R307_TX";         //++ UART RX TAG

void r307_response_parser(char instruction_code[], uint8_t received_package[]);

void r307_init(void)                          
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 57600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

uint8_t r307_reponse(char instruction_code[])
{
    uint8_t received_confirmation_code = 0;
    uint8_t *received_package = (uint8_t *)malloc(RX_BUF_SIZE + 1);
    const int rxBytes = uart_read_bytes(UART_NUM_1, received_package, RX_BUF_SIZE, 300 / portTICK_RATE_MS);
    if (rxBytes > 0)
    {
        received_package[rxBytes] = 0;
        // ESP_LOGI("R307_RX", "Read %d bytes: '%s'", rxBytes, received_package);
        ESP_LOG_BUFFER_HEXDUMP("R307_RX", received_package, rxBytes, ESP_LOG_INFO);

        r307_response_parser(instruction_code, received_package);
        received_confirmation_code = received_package[9];
    }
    free(received_package);

    return received_confirmation_code;
}

uint16_t check_sum(char tx_cmd_data[], char r307_data[])
{
    uint16_t result = 0;
    if(r307_data[0] == '#')
    {
        char length[4];
        sprintf(length,"%c%c",r307_data[1],r307_data[2]);

        for(int i=0; i<atoi(length) - 8; i++)
        {
            result = result + tx_cmd_data[i+6];
        }
    }
    else
    {
        for(int i=0; i<4; i++)
        {
            result = result + tx_cmd_data[i+6];
            if(i<sizeof(r307_data))
            {
                result = result + r307_data[i];
            }
        }
    }

    if(result <= 256)
    {    
        result = result % 256;
    }

    return result;
}

uint8_t VfyPwd(char r307_address[], char vfy_password[])
{
    char tx_cmd_data[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x13};
    char check_sum_data[2] = {0x00, 0x1B};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, vfy_password);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        tx_cmd_data[i+10] = vfy_password[i];
        if(i<2)
        {
            tx_cmd_data[i+14] = check_sum_data[i];
        }
    }

    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);

    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t SetPwd(char r307_address[], char new_password[])
{
    char tx_cmd_data[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x12};
    char check_sum_data[2] = {0x00, 0x1A};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, new_password);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        tx_cmd_data[i+10] = new_password[i];
        if(i<2)
        {
            tx_cmd_data[i+14] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);

    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t SetAdder(char r307_address[], char new_address[])
{
    char tx_cmd_data[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x15};
    char check_sum_data[2] = {0x04, 0x19};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, new_address);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        tx_cmd_data[i+10] = new_address[i];
        if(i<2)
        {
            tx_cmd_data[i+14] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t PortControl(char r307_address[], char control_code[])
{
    char tx_cmd_data[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x17};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, control_code);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = control_code[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+11] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t ReadSysPara(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0F, 0x00, 0x13};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t TempleteNum(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x1D, 0x00, 0x21};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t GR_Auto(char r307_address[])
{
    char tx_cmd_data[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x32, 0x20, 0x00, 0x00, 0x00, 0x00};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#17");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+15] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t GR_Identify(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x34};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t GenImg(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x01};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t UpImage(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0A};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t DownImage(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0B};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t Img2Tz(char r307_address[], char buffer_id[])
{
    char tx_cmd_data[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x02};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, buffer_id);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+11] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t RegModel(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x05};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t UpChar(char r307_address[], char buffer_id[])
{
    char tx_cmd_data[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x08};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, buffer_id);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+11] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t DownChar(char r307_address[], char buffer_id[])
{
    char tx_cmd_data[13] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x04, 0x09};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, buffer_id);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+11] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t Store(char r307_address[], char buffer_id[], char page_id[])
{
    char tx_cmd_data[15] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x06};
    char check_sum_data[2] = {0x00, 0x00};
    char combined_data[3];
    uint8_t confirmation_code = 0;

    memset(combined_data, 0, strlen(combined_data));
    strcat(combined_data, buffer_id);
    strcat(combined_data, page_id);

    uint16_t checksum_value = check_sum(tx_cmd_data, combined_data);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+13] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    ESP_LOG_BUFFER_HEXDUMP("R307_TX", tx_cmd_data, package_length, ESP_LOG_INFO);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t LoadChar(char r307_address[], char buffer_id[], char page_id[])
{
    char tx_cmd_data[15] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x06, 0x07};
    char check_sum_data[2] = {0x00, 0x00};
    char combined_data[3];
    uint8_t confirmation_code = 0;

    memset(combined_data, 0, strlen(combined_data));
    combined_data[0] = buffer_id[0];
    combined_data[1] = page_id[0];
    combined_data[2] = page_id[1];

    uint16_t checksum_value = check_sum(tx_cmd_data, combined_data);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+13] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    ESP_LOG_BUFFER_HEXDUMP("R307_TX", tx_cmd_data, package_length, ESP_LOG_INFO);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t DeletChar(char r307_address[], char page_id[], char number_of_templates[])
{
    char tx_cmd_data[16] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x07, 0x0C, 0x00, 0x00, 0x00, 0x01, 0x00, 0x15};
    char check_sum_data[2] = {0x00, 0x00};
    char combined_data[4];
    uint8_t confirmation_code = 0;

    memset(combined_data, 0, strlen(combined_data));
    for(int i=0; i<2; i++)
    {
        combined_data[i] = page_id[i];
        combined_data[i+2] = number_of_templates[i];
    }

    uint16_t checksum_value = check_sum(tx_cmd_data, combined_data);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = page_id[i];
            tx_cmd_data[i+12] = number_of_templates[i];
            tx_cmd_data[i+14] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    ESP_LOG_BUFFER_HEXDUMP("R307_TX", tx_cmd_data, package_length, ESP_LOG_INFO);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t Empty(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x0D};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t Match(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x03};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t Search(char r307_address[], char buffer_id[], char start_page[], char page_number[])
{
    char tx_cmd_data[17] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0D};
    char check_sum_data[2] = {0x00, 0x00};
    char combined_data[5];
    uint8_t confirmation_code = 0;

    memset(combined_data, 0, strlen(combined_data));
    for(int i=0; i<2; i++)
    {
        combined_data[0] = buffer_id[0];
        combined_data[i+1] = start_page[i];
        combined_data[i+3] = page_number[i];
    }

    uint16_t checksum_value = check_sum(tx_cmd_data, combined_data);
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    tx_cmd_data[10] = buffer_id[0];
    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+11] = start_page[i];
            tx_cmd_data[i+13] = page_number[i];
            tx_cmd_data[i+15] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(500 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

uint8_t GetRandomCode(char r307_address[])
{
    char tx_cmd_data[12] = {0xEF, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x03, 0x14};
    char check_sum_data[2] = {0x00, 0x00};
    uint8_t confirmation_code = 0;

    uint16_t checksum_value = check_sum(tx_cmd_data, "#12");
    check_sum_data[0] = (checksum_value >> 8) & (0xFF);
    check_sum_data[1] = checksum_value & (0xFF);

    for(int i=0; i<4; i++)
    {
        tx_cmd_data[i+2] = r307_address[i];
        if(i<2)
        {
            tx_cmd_data[i+10] = check_sum_data[i];
        }
    }
    
    char instruction_code;

    instruction_code = tx_cmd_data[9];

    const int package_length = sizeof(tx_cmd_data);
    const int txBytes = uart_write_bytes(UART_NUM_1, tx_cmd_data, package_length);
    ESP_LOGI(R307_TX, "Wrote %d bytes", txBytes);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    confirmation_code = r307_reponse(instruction_code);

    return confirmation_code;
}

void r307_response_parser(char instruction_code[], uint8_t received_package[])
{
    uint8_t confirmation_code = received_package[9];

    if(instruction_code == 0x13)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("VfyPwd", "(0x00H) CORRECT PASSWORD\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("VfyPwd", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x13)
        {
            ESP_LOGE("VfyPwd", "(0x13H) WRONG PASSWORD\n");
        }
    }

    if(instruction_code == 0x12)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("SetPwd", "(0x00H) NEW PASSWORD COMPLETE\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("SetPwd", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
    }

    if(instruction_code == 0x15)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("SetAdder", "(0x00H) ADDRESS SETTING COMPLETE\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("SetAdder", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
    }

    if(instruction_code == 0x17)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("PortControl", "(0x00H) PORT OPERATION COMPLETE\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("PortControl", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x1D)
        {
            ESP_LOGE("PortControl", "(0x01DH) FAIL TO OPERATE PORT\n");
        }
    }

    if(instruction_code == 0x0F)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("ReadSysPara", "(0x00H) SYSTEM READ COMPLETE");

            ESP_LOGW("SYSTEM PARAMETER", "Library Size - %x:%x", received_package[14],received_package[15]);
            ESP_LOGW("SYSTEM PARAMETER", "Security Level - %x", received_package[17]);
            ESP_LOGW("SYSTEM PARAMETER", "32bit Address - %x:%x:%x:%x", received_package[18], received_package[19], received_package[20], received_package[21]);
            ESP_LOGW("SYSTEM PARAMETER", "Size Code - %x", received_package[23]);
            ESP_LOGW("SYSTEM PARAMETER", "N - %x\n", received_package[25]);
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("ReadSysPara", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
    }

    if(instruction_code == 0x1D)
    {
        if(confirmation_code == 0x00)
        {
            uint16_t template_number = 0;
            ESP_LOGI("TempleteNum", "(0x00H) READ COMPLETE");

            template_number = received_package[10] + received_package[11];
            ESP_LOGW("SYSTEM PARAMETER", "Template Number - %d\n", template_number);
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("TempleteNum", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
    }

    if(instruction_code == 0x32 || instruction_code == 0x34)
    {
        if(confirmation_code == 0x00)
        {
            uint16_t page_id = 0;
            uint16_t match_score = 0;
            ESP_LOGI("GR_Auto", "(0x00H) READ COMPLETE");

            page_id = received_package[10] + received_package[11];
            match_score = received_package[12] + received_package[13];
            ESP_LOGW("SYSTEM PARAMETER", "Page ID - %d", page_id);
            ESP_LOGW("SYSTEM PARAMETER", "Match Score - %d\n", match_score);
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("GR_Auto/GR_Identify", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x06)
        {
            ESP_LOGE("GR_Auto/GR_Identify", "(0x06H) FINGERPRINT IMAGE GENERATION FAIL\n");
        }
        else if(confirmation_code == 0x07)
        {
            ESP_LOGE("GR_Auto/GR_Identify", "(0x07H) FINGERPRINT IMAGE GENERATION FAIL\n");
        }
        else if(confirmation_code == 0x09)
        {
            ESP_LOGE("GR_Auto/GR_Identify", "(0x09H) NO MATCHING FINGERPRINT\n");
        }
    }

    if(instruction_code == 0x01)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("GenImg", "(0x00H) FINGER COLLECTION SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("GenImg", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x02)
        {
            ESP_LOGE("GenImg", "(0x02H) NO FINGER DETECED\n");
        }
        else if(confirmation_code == 0x03)
        {
            ESP_LOGE("GenImg", "(0x03H) FAIL TO COLLECT FINGER\n");
        }
    }

    if(instruction_code == 0x0A)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("UpImage", "(0x00H) READY TO TRANSFER PACKET\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("UpImage", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0F)
        {
            ESP_LOGE("UpImage", "(0x0FH) FAILED TO TRANSFER PACKET\n");
        }
    }

    if(instruction_code == 0x0B)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("DownImage", "(0x00H) READY TO TRANSFER PACKET\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("DownImage", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0E)
        {
            ESP_LOGE("DownImage", "(0x0EH) FAILED TO TRANSFER PACKET\n");
        }
    }

    if(instruction_code == 0x02)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("Img2Tz", "(0x00H) GENERATE CHARACTER FILE COMPLETE\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("Img2Tz", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x06)
        {
            ESP_LOGE("Img2Tz", "(0x06H) FAILED TO GENERATE CHARACTER FILE\n");
        }
        else if(confirmation_code == 0x07)
        {
            ESP_LOGE("Img2Tz", "(0x07H) FAILED TO GENERATE CHARACTER FILE\n");
        }
        else if(confirmation_code == 0x15)
        {
            ESP_LOGE("Img2Tz", "(0x15H) FAILED TO GENERATE IMAGE\n");
        }
    }

    if(instruction_code == 0x05)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("RegModel", "(0x00H) OPERATION SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("RegModel", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0A)
        {
            ESP_LOGE("RegModel", "(0x0AH) FAILED TO COMBINE CHARACTER FILES\n");
        }
    }

    if(instruction_code == 0x08)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("UpChar", "(0x00H) READY TO TRANSFER\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("UpChar", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0D)
        {
            ESP_LOGE("UpChar", "(0x0DH) ERROR UPLOADING TEMPLATE\n");
        }
    }

    if(instruction_code == 0x09)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("DownChar", "(0x00H) READY TO TRANSFER\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("DownChar", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0E)
        {
            ESP_LOGE("DownChar", "(0x0EH) FAILED TO RECEIVE PACKAGES\n");
        }
    }

    if(instruction_code == 0x06)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("Store", "(0x00H) STORAGE SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("Store", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0B)
        {
            ESP_LOGE("Store", "(0x0BH) ADDRESSED PAGE ID IS BEYOND LIMIT\n");
        }
        else if(confirmation_code == 0x18)
        {
            ESP_LOGE("Store", "(0x18H) ERROR WRITING FLASH\n");
        }
    }

    if(instruction_code == 0x07)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("LoadChar", "(0x00H) LOAD SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("LoadChar", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x0C)
        {
            ESP_LOGE("LoadChar", "(0x0CH) ERROR READING TEMPLATE FROM LIBRARY\n");
        }
        else if(confirmation_code == 0x0B)
        {
            ESP_LOGE("LoadChar", "(0x0BH) ADDRESSED PAGE ID IS BEYOND LIMIT\n");
        }
    }

    if(instruction_code == 0x0C)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("DeletChar", "(0x00H) DELETE SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("DeletChar", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x10)
        {
            ESP_LOGE("DeletChar", "(0x10H) FAIL TO DELETE TEMPLATE\n");
        }
    }

    if(instruction_code == 0x0D)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("Empty", "(0x00H) EMPTY SUCCESS\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("Empty", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x11)
        {
            ESP_LOGE("Empty", "(0x11H) FAIL TO CLEAR LIBRARY\n");
        }
    }

    if(instruction_code == 0x03)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("Match", "(0x00H) TWO TEMPLATE BUFFERS MATCH\n");
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("Match", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x08)
        {
            ESP_LOGE("Match", "(0x08H) TWO TEMPLATES BUFFER UNMATHED\n");
        }
    }

    if(instruction_code == 0x04)
    {
        if(confirmation_code == 0x00)
        {
            uint16_t page_id = 0;
            uint16_t match_score = 0;
            ESP_LOGI("Search", "(0x00H) FOUND MATCHING FINGER");

            page_id = received_package[10] + received_package[11];
            match_score = received_package[12] + received_package[13];
            ESP_LOGW("SYSTEM PARAMETER", "Page ID - %d", page_id);
            ESP_LOGW("SYSTEM PARAMETER", "Match Score - %d\n", match_score);
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("Search", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
        else if(confirmation_code == 0x09)
        {
            ESP_LOGE("Search", "(0x09H) NO MATCHING FINGER IN LIBRARY\n");
        }
    }

    if(instruction_code == 0x14)
    {
        if(confirmation_code == 0x00)
        {
            ESP_LOGI("GetRandomCode", "(0x00H) GENERATION SUCCESSFUL");

            ESP_LOGW("GetRandomCode", "RANDOMLY GENERATED NUMBER - %x:%x:%x:%x\n", received_package[10], received_package[11], received_package[12],received_package[13]);
        }
        else if(confirmation_code == 0x01)
        {
            ESP_LOGE("GetRandomCode", "(0x01H) ERROR RECEIVING PACKAGE\n");
        }
    }

}

