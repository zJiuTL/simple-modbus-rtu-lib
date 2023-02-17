#ifndef __MODBUS_RTU__H
#define __MODBUS_RTU__H

#include <stdint.h>


#define MODBUS_RTU_BUFF_SIZE                   255


/* modbus rtu 
Refer to the web page https://www.simplymodbus.ca/index.html. */
typedef struct{
    /*Data sending interface*/
    void (*send_datas_callback)(const uint8_t *datas ,uint16_t dataslen); /* Data output callback */
    /*Read Coil Status (FC=01) .
    This command is requesting the ON/OFF status of discrete coils .*/
    /*Read Input Status (FC=02). 
    This command is requesting the ON/OFF status of discrete inputs .*/
    /* Read Holding Registers (FC=03). 
    This command is requesting the content of analog output holding registers .*/
    /*Read Input Registers (FC=04).
    This command is requesting the content of analog input register .*/
    int (*request_fc01_04_callback)(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t registersCnt); 
    /*Force Single Coil (FC=05).
    This command is writing the contents of discrete coil .*/
    /*Preset Single Register (FC=06).
    This command is writing the contents of analog output holding register .
    The status to write ( FF00 = ON,  0000 = OFF ) */
    int (*request_fc05_06_callback)(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t status);
    /*Force Multiple Coils (FC=15).
    This command is writing the contents of a series of 10 discrete coils .*/
    /*Preset Multiple Registers (FC=16).
    This command is writing the contents of two analog output holding registers .*/
    int (*request_fc15_16_callback)(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t registersCnt ,uint8_t dataslen ,const uint8_t *coilsDatas);
}modbus_rtu_t;


/*----------------------------------------------------------------------------------*/
int rtu_init(const modbus_rtu_t *rtu);
void rtu_set_slaver_addr(uint8_t addr);
/*User data entry. Incoming data callback */
void rtu_incoming_data_callback(const uint8_t *datas ,uint16_t dataslen);
/*Plug data into the send buffer*/
int rtu_push_resp_data(uint8_t data);
int rtu_resp_error(uint8_t excepCode);







#endif //end of __MODBUS_RTU__H
