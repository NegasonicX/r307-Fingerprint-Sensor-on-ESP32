#include <stdint.h>

#ifndef r307_H
#define r307_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief INITIALIZE UART FOR R307 FINGERPRINT MODULE
 *
 * @return
 */
void r307_init(void);

/**
 * @brief FUNCITON TO GET RESPONSES FROM R307 FINGERPRINT MODULE
 *
 * @param instruction_code INSTRUCTION CODE FOR EACH COMMAND
 * @return RETURNS CONFIRMATION CODE RECEIVED FROM THE RESPONSE
 */
uint8_t r307_reponse(char instruction_code[]);

/**
 * @brief FUNCTION TO PERFORM CHECKSUM MODULE 256
 *
 * @param tx_cmd_data ENTIRE COMMAND STRING 
 * @param r307_data STRING WITH NECESSARY DATA FOR THE PACKET OR CHECKSUM FLAG
 * @return RETURNS CHECKSUM VALUE ( BOTH HIGHER & LOWER BITS COMBINED )
 */
uint16_t check_sum(char tx_cmd_data[], char r307_data[]);

/**
 * @brief FUNCTION TO VERIFY PASSWORD BY HANDSHAKING
 *
 * @param r307_address CURRENT MODULE ADDRESS  
 * @param vfy_password CURRENT MODULE PASSWORD
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t VfyPwd(char r307_address[], char vfy_password[]);

/**
 * @brief Function to Set New Module Password
 *
 * @param r307_address CURRENT MODULE ADDRESS  
 * @param new_password NEW PASSWORD TO BE SET
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t SetPwd(char r307_address[], char new_password[]);

/**
 * @brief Function to Set New Module Address
 *
 * @param r307_address CURRENT MODULE ADDRESS  
 * @param new_address NEW ADDRESS TO BE SET
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t SetAdder(char r307_address[], char new_address[]);

/**
 * @brief Function to Turn ON/OFF Module Port
 *
 * @param r307_address CURRENT MODULE ADDRESS  
 * @param control_code 0 : PORT OFF | 1 : PORT ON
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t PortControl(char r307_address[], char control_code[]);

/**
 * @brief Function to read Current System Parameters
 *
 * @param r307_address CURRENT MODULE ADDRESS  
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t ReadSysPara(char r307_address[]);

/**
 * @brief FUNCTION TO READ CURRENT VALID TEMPLATE NUMBER
 *
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t TempleteNum(char r307_address[]);

/**
 * @brief FUNCTION TO MATCH CAPTURED FINGER FROM LIBRARY & RETURN RESULTS
 *
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t GR_Auto(char r307_address[]);

/**
 * @brief FUNCTION TO AUTOMATICALLY COLLECT FINGER, MATCH CAPTURED FINGER FROM LIBRARY & RETURN RESULTS
 *
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t GR_Identify(char r307_address[]);

/**
 * @brief FUNCTION TO DETECT FINGER AND STORE IMAGE IN IMAGEBUFFER
 *
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t GenImg(char r307_address[]);

/**
 * @brief FUNCTION TO UPLOAD THE IMAGE IN IMG_BUFFER TO UPPER COMPUTER
 *
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t UpImage(char r307_address[]);

/**
 * @brief FUNCTION TO DOWNLOAD IMAGE FROM UPPER COMPUTER TO IMG_BUFFER
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t DownImage(char r307_address[]);

/**
 * @brief FUNCTION TO GENERATE CHARACTER FILE FROM IMAGE IN IMAGE BUFFER AND STORE IN CHARBUFFER1/CHARBUFFER2
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER ) 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t Img2Tz(char r307_address[], char buffer_id[]);

/**
 * @brief FUNCTION TO COMBINE BOTH CHARACTER FILES AND GENERATE TEMPLATE, STORE IN CHARBUFFER1 & CHARBUFFER2
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t RegModel(char r307_address[]);

/**
 * @brief FUNCTION TO UPLOAD CHARACTER FILE/TEMPLATE OF CHARBUFFER1/CHARBUFFER2 TO UPPER COMPUTER
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER ) 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t UpChar(char r307_address[], char buffer_id[]);

/**
 * @brief FUNCTION TO DOWNLOAD CHARACTER FILE/TEMPLATE OF CHARBUFFER1/CHARBUFFER2 TO UPPER COMPUTER
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER ) 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t DownChar(char r307_address[], char buffer_id[]);

/**
 * @brief FUNCTION TO STORE TEMPLATE TO SPECIFIED BUFFER AT DESIRED FLASH LOCATION
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER ) 
 * @param page_id FLASH LOCATION OF THE TEMPLATE
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t Store(char r307_address[], char buffer_id[], char page_id[]);

/**
 * @brief FUNCTION TO LOAD TEMPLATE FROM DESIRED FLASH LOCAITON TO SPECIFIED BUFFER
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER ) 
 * @param page_id FLASH LOCATION OF THE TEMPLATE
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t LoadChar(char r307_address[], char buffer_id[], char page_id[]);

/**
 * @brief FUNCTION TO DELETE N SEGMENT OF TEMPLATES OF FLASH STARTING FROM DESIRED LOCATION
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param page_id FLASH LOCATION OF THE TEMPLATE
 * @param number_of_templates N : NUMBER OF TEMPLATES TO BE DELETED 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t DeletChar(char r307_address[], char page_id[], char number_of_templates[]);

/**
 * @brief FUNCTION TO DELETE ALL THE TEMPLATES FROM FLASH LIBRARY
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t Empty(char r307_address[]);

/**
 * @brief FUNCTION TO PERFORM PRECISE MATCHING OF TEMPLATES FROM CHARBUFFER1 & CHARBUFFER2
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t Match(char r307_address[]);

/**
 * @brief FUNCTION TO SEARCH WHOLE LIBRARY FOR TEMPLATE THAT MATCHES CHARBUFFER1/CHARBUFFER2
 *
 * @param r307_address CURRENT MODULE ADDRESS
 * @param buffer_id BUFFER ID ( CHARACTER FILE BUFFER NUMBER )
 * @param start_page START ADDRESS FOR SEARCH OPERATION
 * @param page_number SEARCHING NUMBER
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t Search(char r307_address[], char buffer_id[], char start_page[], char page_number[]);

/**
 * @brief FUNCTION TO GENERATE 32-BIT RANDOM NUMBER & RETURN TO UPPER COMPUTER
 * @param r307_address CURRENT MODULE ADDRESS 
 * @return RETURNS RECEIVED CONFIRMATION CODE FROM MODULE
 */
uint8_t GetRandomCode(char r307_address[]);

/**
 * @brief FUNCTION TO PARSE RESPONSES RECEIVED FROM THE MODULE
 * @param instruction_code INSTRUCTION CODE OF THE RECEIVED COMMAND 
 * @param received_package ENTIRE RECEIVED STRING 
 * @return
 */
void r307_response_parser(char instruction_code[], uint8_t received_package[]);

#ifdef __cplusplus
}
#endif

#endif // r307_H
