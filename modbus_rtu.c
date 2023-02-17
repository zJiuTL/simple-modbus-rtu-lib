#include "modbus_rtu.h"
#include <string.h>



#define MODBUS_RTU_CRC16                         0xA001


static modbus_rtu_t *pRtu = NULL;
static uint8_t m_devAddr = 0x01;
static uint8_t m_buffer[MODBUS_RTU_BUFF_SIZE];
static uint16_t m_bufferIdx;
static uint8_t m_bAutoFilled;

/*--------------------------------------------------------------------------
* @brief      modbus rtu init function
* @param   rtu : struct callback point
* @return   0 for invalid ,else for valid
*/
int rtu_init(const modbus_rtu_t *rtu)
{
    if((MODBUS_RTU_BUFF_SIZE < 8) || (rtu->send_datas_callback == NULL)){
        return (0);
    }else{
        pRtu = (modbus_rtu_t *)rtu;
        return (1);
    }
}

/*--------------------------------------------------------------------------
* @brief      set slaver addr function
* @param  addr : device address
* @return   none
*/
void rtu_set_slaver_addr(uint8_t addr)
{
    m_devAddr = addr;
}

/*--------------------------------------------------------------------------
* @brief      Calculate CRC16 function
* @param   datas : input datas point
* @param   dataslen : input datas size
* @return   crc16 data
*/
static uint16_t rtu_cal_crc16(const uint8_t *datas, uint16_t dataslen)
{
    uint16_t crc16 = 0xFFFF ,n ,m;
    
    for(n = 0 ; n < dataslen; n++){
        crc16 ^= datas[n];
        for(m = 0; m < 8; m++){
            if(crc16 & 0x01){
                crc16 >>= 1;
                crc16 ^= MODBUS_RTU_CRC16;
            }else{
                crc16 >>= 1;
            }
        }
    }
    
    return crc16;
}

/*--------------------------------------------------------------------------
* @brief      Plug data into the send m_bufferer
* @param   data : input single data
* @return   0 for invalid ,else for valid
*/
int rtu_push_resp_data(uint8_t data)
{
    if(m_bAutoFilled){
        m_bAutoFilled = 0;
        m_bufferIdx = 2;
    }
    
    if((m_bufferIdx + 3) >= MODBUS_RTU_BUFF_SIZE){
        return (0);
    }else{
        m_buffer[m_bufferIdx] = data;
        m_bufferIdx++;
        return (1);
    }
}

/*--------------------------------------------------------------------------
* @brief      packet buffer for sending.
* @param   none.
* @return   0 for invalid ,else for valid
*/
static int rtu_packet_for_send(void)
{
    uint16_t crc16;
    
    if(m_bufferIdx <= 2){
        return (0);
    }else{
        crc16 = rtu_cal_crc16(m_buffer ,m_bufferIdx);
        m_buffer[m_bufferIdx++] = ((crc16 >> 0) & 0xFF);
        m_buffer[m_bufferIdx++] = ((crc16  >> 8) & 0xFF);
        return m_bufferIdx;
    }
}

/*--------------------------------------------------------------------------
* @brief      parse master datas
* @param   datas : input datas point
* @param   dataslen : input datas size
* @param   offset : datas offset
* @return   valid length
*/
static int rtu_parse_master_datas(const uint8_t *datas ,uint16_t dataslen ,uint16_t offset)
{
    int res = -1 ,resp;
    uint16_t crc16 ,temp;
#define RTU_REG           temp
#define RTU_REG_CNT  crc16
#define RTU_REG_STA  crc16
    
    if(datas == NULL || dataslen <= 0 || dataslen < offset || (dataslen - offset) < 8/*Minimal data size.*/ || pRtu == NULL){
        return (res);
    }
    
    if((datas[offset] == m_devAddr || datas[offset] == 0x00 || datas[offset] == 0xFF) 
       && (((datas[offset + 1] & 0x80) == 0)) /* Exception responses */){
           switch(datas[offset + 1])
           {
           case 0x01:
           case 0x02:
           case 0x03:
           case 0x04:
           case 0x05:
           case 0x06:
               {
                   crc16 = rtu_cal_crc16(datas + offset ,6);
                   if(crc16 == (uint16_t)((datas[offset + 7] << 8) | datas[offset + 6])){
                       res = 8;
                       /* User function callbacks */
                       RTU_REG = (datas[offset + 2] << 8) | datas[offset + 3];
                       if(datas[offset + 1] <= 0x04){
                           m_buffer[0] = m_devAddr;
                           m_buffer[1] = datas[offset + 1];
                           m_bufferIdx = 2;
                           m_bAutoFilled = 0;
                           RTU_REG_CNT = (datas[offset + 4] << 8) | datas[offset + 5];
                           if(pRtu->request_fc01_04_callback == NULL){ 
                               rtu_resp_error(0x04);
                           }else{
                               resp = pRtu->request_fc01_04_callback(datas[offset] ,datas[offset + 1] ,RTU_REG ,RTU_REG_CNT);
                               if(resp <= 0){
                                   rtu_resp_error(resp < 0 ? 0x01 : 0x03);
                               }
                           }
                       }else{
                           memcpy(m_buffer ,datas + offset ,6);
                           m_buffer[0] = m_devAddr;
                           m_bufferIdx = 6;
                           m_bAutoFilled = 1;
                           RTU_REG_STA = (datas[offset + 4] << 8) | datas[offset + 5];
                           if(pRtu->request_fc05_06_callback == NULL){
                               rtu_resp_error(0x04);
                           }else{
                               resp = pRtu->request_fc05_06_callback(datas[offset] ,datas[offset + 1] ,RTU_REG ,RTU_REG_STA);
                               if(resp <= 0){
                                   rtu_resp_error(resp < 0 ? 0x01 : 0x03);
                               }
                           }
                       }
                   }
               }
               break;
           case 0x0F:
           case 0x10:
               {
                   temp = (8 + 1 + datas[offset + 6] /*The number of data bytes to follow*/);
                   if(((offset + temp) <= dataslen) && (datas[offset + 6] > 0)){
                       crc16 = rtu_cal_crc16(datas + offset ,(temp - 2));
                       if(crc16 == (uint16_t)((datas[offset + temp - 1] << 8) | datas[offset + temp - 2])){
                           res = temp;
                           /* User function callbacks */
                           memcpy(m_buffer ,datas + offset ,6);
                           m_buffer[0] = m_devAddr;
                           m_bufferIdx = 6;
                           m_bAutoFilled = 1;
                           RTU_REG = (datas[offset + 2] << 8) | datas[offset + 3];
                           RTU_REG_CNT = (datas[offset + 4] << 8) | datas[offset + 5];
                           
                           if(pRtu->request_fc15_16_callback == NULL){
                               rtu_resp_error(0x04);
                           }else{
                               resp = pRtu->request_fc15_16_callback(datas[offset] ,datas[offset + 1] ,RTU_REG ,RTU_REG_CNT ,datas[offset + 6] ,(datas+ offset + 7));
                               if(resp <= 0){
                                   rtu_resp_error(resp < 0 ? 0x01 : 0x03);
                               }
                           }
                       }
                   }
               }
               break;
           default:/* Do not support yet */
               {
                   rtu_resp_error(0x01);
                   res = dataslen;
               }
               break;
           }
       }
    
    /*valid datas*/
    if(res > 0){
        pRtu->send_datas_callback(m_buffer ,rtu_packet_for_send());
    }
    
    return (res);
}

/*--------------------------------------------------------------------------
* @brief      User data entry. Incoming data callback
* @param   datas : input datas point
* @param   dataslen : input datas size
* @return   none
*/
void rtu_incoming_data_callback(const uint8_t *datas ,uint16_t dataslen)
{
    int res = -1;
    uint16_t offset = 0;
    
    if(pRtu == NULL){
        return;
    }
    
    do{
        res = rtu_parse_master_datas(datas ,dataslen ,offset);
        if(res <= 0){
            break;
        }else{
            offset += res;
        }
    }while(dataslen > offset);
    
    return ;
}

/*--------------------------------------------------------------------------
* @brief      Exception Responses
* @param   excepCode : error code
* @return   none
*/
int rtu_resp_error(uint8_t excepCode)
{
    m_buffer[1] |= 0x80;
    return rtu_push_resp_data(excepCode);
}

