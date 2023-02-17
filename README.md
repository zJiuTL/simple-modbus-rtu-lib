# simple-modbus-rtu-lib
Yet ,the simplest Modbus RTU library.
Refer to the web page https://www.simplymodbus.ca/index.html. 

Example:
 
step 1 ,set callback funtion: /*---------------*/
 
static void send_datas_callback(const uint8_t *datas ,uint16_t dataslen)
{
  uart_write(datas ,dataslen);
}

static int fc01_04_callback(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t registersCnt)
{
    int res = 0;
    
    switch(func)
    {
    case 0x01:
        {
            rtu_push_resp_data(0x05);/*dataslen*/
            rtu_push_resp_data(0xCD);
            rtu_push_resp_data(0x6B);
            rtu_push_resp_data(0xB2);
            rtu_push_resp_data(0x0E);
            rtu_push_resp_data(0x1B);
            res = 1;
        }
        break;
    case 0x02:
        {
            rtu_push_resp_data(0x03);/*dataslen*/
            rtu_push_resp_data(0xAC);
            rtu_push_resp_data(0xDB);
            rtu_push_resp_data(0x35);
            res = 1;
        }
        break;
    case 0x03:
        {
            rtu_push_resp_data(0x06);/*dataslen*/
            rtu_push_resp_data(0xAE);
            rtu_push_resp_data(0x41);
            rtu_push_resp_data(0x56);
            rtu_push_resp_data(0x52);
            rtu_push_resp_data(0x43);
            rtu_push_resp_data(0x40);
            res = 1;
        }
        break;
    case 0x04:
        {
            rtu_push_resp_data(0x02);/*dataslen*/
            rtu_push_resp_data(0x00);
            rtu_push_resp_data(0x0A);
            res = 1;
        }
        break;
    default:
        break;
    }
    
    return res;
}

static int fc05_06_callback(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t status)
{
    int res = 0;
    
    switch(func)
    {
    case 0x05:
        {
            res = 1;
        }
        break;
    case 0x06:
        {
            res = 1;
        }
        break;
    default:
        break;
    }
    
    return res;
}

static int fc15_16_callback(uint8_t addr ,uint8_t func ,uint16_t registers ,uint16_t registersCnt ,uint8_t dataslen ,const uint8_t *coilsDatas)
{
    int res = 0;
    
    switch(func)
    {
    case 0x0F:
        {
            res = 1;
        }
        break;
    case 0x10:
        {
            res = -1;
        }
        break;
    default:
        break;
    }
    
    return res;
}

static const modbus_rtu_t m_rtu = {
    .request_fc01_04_callback = fc01_04_callback ,
    .request_fc05_06_callback = fc05_06_callback ,
    .request_fc15_16_callback = fc15_16_callback ,
    .send_datas_callback = send_datas_callback 
};
  
 step 2 ,init slaver param: /*---------------*/
 
       static void init(void)
      {
        /* Init rtu lib */
          rtu_init(&m_rtu);
         rtu_set_slaver_addr(0x01);
      }
    
step 3 ,data coming callback: /*---------------*/
    rtu_incoming_data_callback(m_buff ,m_idx) 
    
Done!

Enjoy it!
