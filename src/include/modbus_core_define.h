#ifndef PROCOTOCOLBUILDPROJ_MODBUS_CORE_DEFINE_H
#define PROCOTOCOLBUILDPROJ_MODBUS_CORE_DEFINE_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
  /**
   * @brief 读线圈状态
   */
  ReadCoilState = 0x1,
  /**
   * @brief 读输入状态
   */
  ReadInputState = 0x2,
  /**
   * @brief 读保持寄存器
   */
  ReadHoldReg = 0x3,
  /**
   * @brief 读输入寄存器
   */
  ReadInputReg = 0x4,
  /**
   * @brief 写一个线圈
   */
  WriteOneCoil = 0x05,
  /**
   * @brief 写一个寄存器
   */
  WriteOneReg = 0x06,
  /**
   * @brief 写多个寄存器
   */
  WriteMulReg = 0x10,
  /**
   * @brief 写多个线圈
   */
  WriteMulCoilsReg = 0x0F,
} Modbus_FuncCode_TypeDef;

typedef enum {
  // 非法数据
  InvalidDataErrorCode = 0x2,
  // 接收数据长度不正确
  RecDataLenErrorCode = 0xFF,

} Modbus_RecCode_TypeDef;

#define modbus_u8_to_u16(a, b) (uint16_t)((uint16_t)((a) << 8) + (b))
// 发送数据回调函数指针
typedef void (*SwSendDataCallBack)(void *data, uint16_t len);
// 接收数据回调函数指针
typedef uint16_t (*SwRecDataCallBack)(void *recDataBuf);

#endif // PROCOTOCOLBUILDPROJ_MODBUS_CORE_DEFINE_H
