#include <stdio.h>
#include <string.h>
#include "rs485.h"
#include "modbus_api.h"
#include "modbus_core.h"
#include "cmsis_os.h"
modbus_data modbus;

/*获取缓冲数据*/
modbus_data *get_modbus(void) {
  return &modbus;
}

/*清空缓冲数据*/
void clear_modbus_buf(void) {
  memset(&modbus, 0x00, sizeof(modbus_data));
}

// 发送数据回调函数指针
void modbus_send_func(void *data, uint16_t len) {
  osKernelLock();
  rs485_send_data(data, len);
  osKernelUnlock();
}
// 接收数据回调函数指针
uint16_t modbus_recv_func(void *recDataBuf) {
  uint16_t ret_len = 0;
  memcpy(recDataBuf, modbus.buff, modbus.len);
  ret_len = modbus.len;
  clear_modbus_buf();
  return ret_len;
}
/*注册回调函数*/
void init_modbus(void) {
  Modbus_RegistrySendCallBack(&modbus_send_func);

  Modbus_RegistryRecCallBack(&modbus_recv_func);
}
