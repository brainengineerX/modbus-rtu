# modbus-rtu
standard modbus-rtu，use rs-485 bus，simple ，already used in engineering。

modbus-rtu协议 基于rs485 已工程化

# 使用

调用初始化注册接收和发送函数
```c
/*初始化Modbus-rtu协议库 */
  init_modbus();
```
实现发送函数modbus_send_func，如下：
```c
/*发送函数*/
void rs485_send_data(const u8 *string, u8 len) {
  /*发送模式*/
  rs485_send_mode();
  u8 i = 0;
  for (i = 0; i < len; i++) {
    /*等待空闲*/
    while (USART_GetFlagStatus(RS485_USARTx, USART_FLAG_TC) == RESET)
      ;
    USART_SendData(RS485_USARTx, string[i]);
  }
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    ;
  /*发送完接收模式*/
  rs485_rcv_mode();
}
```
以下是接收一个字节到modbus_data内到中断
```c
/**
 * @brief  USART1中断 Modbus协议
 * @param  None
 * @retval None
 */
void USART1_IRQHandler(void) {
  u16 recv_byte = 0;
  modbus_data *p;
  if (USART_GetITStatus(RS485_USARTx, USART_IT_RXNE) != RESET) {
    p = get_modbus();
    recv_byte = USART_ReceiveData(RS485_USARTx); // 将接收的一个字节保存
    p->buff[p->len++] = recv_byte;               // 保存到MODBUS的接收缓存区
  }
}
```
