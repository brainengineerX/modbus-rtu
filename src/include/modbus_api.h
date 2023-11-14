#ifndef _MODBUS_API_H
#define _MODBUS_API_H

typedef struct modbus_data_t {
  unsigned char len;
  unsigned char buff[110];
} modbus_data;

/*获取缓冲数据*/
modbus_data *get_modbus(void);

/*清空缓冲数据*/
void clear_modbus_buf(void);

// 发送数据回调函数指针
void modbus_send_func(void *data, uint16_t len);

// 接收数据回调函数指针
uint16_t modbus_recv_func(void *recDataBuf);

void init_modbus(void);
#endif
