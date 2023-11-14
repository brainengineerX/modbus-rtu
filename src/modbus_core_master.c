#include "modbus_core.h"
#include "cmsis_os2.h"
/*这是Modbus发送指令后 等待的延时时间 毫秒
  等待的时间包括：从设备响应时间 + 串口接收10字节延时(波特率9600)
  太短了  没法收到完整的设备回复包  会导致上报的命令回复为异常*/
#define MODBUS_WAIT_ECHO_TIME (100)

// 错误指令响应长度
#define ERROR_RESP_DATA_LEN   (5)
// 错误响应码下标位置
#define ERROR_RESP_CODE_IDX   (2)
#define MODBUS_PACKET_BUF_LEN (110)

// modbus 包缓存数组
static uint8_t modbus_packet_buf[MODBUS_PACKET_BUF_LEN] = {0};

static uint8_t core_build_packet(uint8_t slaveAddr, uint16_t regAddr, uint16_t val, uint8_t funcCode, uint8_t *retBuf);
/**
 * 从机地址(1)-功能码(1)-寄存器地址(2)--数据(2)-CRC(2)
 * @brief 写一个寄存器
 * @param slaveAddr 从机地址
 * @param regAddr 寄存器地址
 * @param val 值
 * @param errorCode 错误码
 * @return
 */
bool Master_WriteOneRegister(uint8_t slaveAddr, uint16_t regAddr, uint16_t val, uint8_t *errorCode) {
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t idx = core_build_packet(slaveAddr, regAddr, val, WriteOneReg, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, idx);
  if (modbusRecCallBack != NULL) {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
    } else {
      retFlag = false;
      if (errorCode != NULL) {
        if (recLen == ERROR_RESP_DATA_LEN) {
          *errorCode = modbus_packet_buf[ERROR_RESP_CODE_IDX];
        } else {
          *errorCode = RecDataLenErrorCode; // 接收数据格式不争取
        }
      }
    }
  }

  return retFlag;
}
/**
 * 从机地址(1)-功能码(1)-寄存器地址(2)-寄存器个数(2)-字节数(1)-数据(n)-CRC(2)
 * @brief  串口协议包(0x10) 写多个寄存器
 * @param slaveAddr 从机地址
 * @param regAddr 寄存器地址
 * @param data 数据（长度为2的倍数）
 * @param dataLen 数据长度()
 * @param errorCode 错误码
 * @return
 */
bool Master_WriteMulRegister(uint8_t slaveAddr, uint16_t regAddr, uint8_t *data, uint16_t dataLen, uint8_t *errorCode) {
  uint8_t idx = 0;
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  modbus_packet_buf[idx++] = slaveAddr;
  modbus_packet_buf[idx++] = WriteMulReg;
  modbus_packet_buf[idx++] = (uint8_t)(regAddr >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(regAddr);
  uint16_t regNum = (uint16_t)(dataLen / 2);
  modbus_packet_buf[idx++] = (uint8_t)(regNum >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(regNum);
  modbus_packet_buf[idx++] = (uint8_t)(dataLen);
  for (int i = 0; i < dataLen; ++i) {
    modbus_packet_buf[idx++] = data[i];
  }
  uint16_t crc = GetCRCData(modbus_packet_buf, idx);
  modbus_packet_buf[idx++] = (uint8_t)(crc >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(crc);
  modbusSendCallBack(modbus_packet_buf, idx);
  if (modbusRecCallBack != NULL) {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
      retFlag = false;
      if (errorCode != NULL) {
        if (recLen == ERROR_RESP_DATA_LEN) {
          *errorCode = modbus_packet_buf[ERROR_RESP_CODE_IDX];
        } else {
          *errorCode = RecDataLenErrorCode; // 接收数据格式不争取
        }
      }
    }
  }
  return retFlag;
}

/**
 * 从机地址(1)-功能码(1)-寄存器地址(2)--数据(2)-CRC(2)
 * @brief 写一个线圈
 * @param slaveAddr 从机地址
 * @param regAddr 寄存器地址
 * @param val 值
 * @param errorCode 错误码
 * @return
 */
bool Master_WriteOneCoil(uint8_t slaveAddr, uint16_t regAddr, bool val, uint8_t *errorCode) {
  uint16_t data = val ? 0xFF00 : 0x0000;
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t idx = core_build_packet(slaveAddr, regAddr, data, WriteOneCoil, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, idx);
  if (modbusRecCallBack != NULL) {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
    } else {
      retFlag = false;
      if (errorCode != NULL) {
        if (recLen == ERROR_RESP_DATA_LEN) {
          *errorCode = modbus_packet_buf[ERROR_RESP_CODE_IDX];
        } else {
          *errorCode = RecDataLenErrorCode; // 接收数据格式不争取
        }
      }
    }
  }
  return retFlag;
}
/**
 * @brief 写多个线圈
 * @param slaveAddr
 * @param regAddr
 * @param val
 * @param dataLen
 * @param errorCode
 * @return
 */
bool Master_WriteMulCoil(uint8_t slaveAddr, uint16_t regAddr, bool *val, uint16_t dataLen, uint8_t *errorCode) {
  uint8_t idx = 0;
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  // 计算数据占几个字节
  uint16_t realDataLen = PackBoolArray(modbus_packet_buf + 7, val, dataLen);
  modbus_packet_buf[idx++] = slaveAddr;
  modbus_packet_buf[idx++] = WriteMulCoilsReg;
  modbus_packet_buf[idx++] = (uint8_t)(regAddr >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(regAddr);
  modbus_packet_buf[idx++] = (uint8_t)(dataLen >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(dataLen);
  modbus_packet_buf[idx++] = (uint8_t)(realDataLen);
  idx += realDataLen;
  uint16_t crc = GetCRCData(modbus_packet_buf, idx);
  modbus_packet_buf[idx++] = (uint8_t)(crc >> 8);
  modbus_packet_buf[idx++] = (uint8_t)(crc);
  modbusSendCallBack(modbus_packet_buf, idx);
  if (modbusRecCallBack != NULL) {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2)) {
      return false;
    }
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
    } else {
      retFlag = false;
      if (errorCode != NULL) {
        if (recLen == ERROR_RESP_DATA_LEN) {
          *errorCode = modbus_packet_buf[ERROR_RESP_CODE_IDX];
        } else {
          *errorCode = RecDataLenErrorCode; // 接收数据格式不争取
        }
      }
    }
  }
  return retFlag;
}
/**
 * @brief 读保持寄存器
 * @param slaveAddr
 * @param regAddr
 * @param regNum
 * @param retData
 * @param errorCode
 * @return
 */
bool Master_ReadHoldRegisters(uint8_t slaveAddr, uint16_t regAddr, uint16_t regNum, uint8_t *retData,
                              uint8_t *errorCode) {
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t packetLen = core_build_packet(slaveAddr, regAddr, regNum, ReadHoldReg, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, packetLen);
  if (modbusRecCallBack == NULL) {
    // todo 回调未注册
    return false;
  } else {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
      if (recLen == ERROR_RESP_DATA_LEN) {
        retFlag = false;
        if (errorCode != NULL)
          *errorCode = modbus_packet_buf[2];
      } else {
        // todo 拷贝数据
        uint8_t cpyDataLen = modbus_packet_buf[2];
        memcpy(retData, modbus_packet_buf + 3, cpyDataLen);
      }
    } else {
      // CRC 不正确
      return false;
    }
  }
  return retFlag;
}
/**
 * @brief
 * @param slaveAddr
 * @param regAddr
 * @param regNum
 * @param retData
 * @param errorCode
 * @return
 */
bool Master_ReadInputRegisters(uint8_t slaveAddr, uint16_t regAddr, uint16_t regNum, uint8_t *retData,
                               uint8_t *errorCode) {
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t packetLen = core_build_packet(slaveAddr, regAddr, regNum, ReadInputReg, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, packetLen);
  if (modbusRecCallBack == NULL) {
    // todo 回调未注册
    return false;
  } else {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    // 验证CRC
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
      if (recLen == ERROR_RESP_DATA_LEN) {
        retFlag = false;
        if (errorCode != NULL)
          *errorCode = modbus_packet_buf[2];
      } else {
        // todo 拷贝数据
        uint8_t cpyDataLen = modbus_packet_buf[2];
        memcpy(retData, modbus_packet_buf + 3, cpyDataLen);
      }
    } else {
      // CRC 不正确
      return false;
    }
  }
  return retFlag;
}

bool Master_ReadCoilStatus(uint8_t slaveAddr, uint16_t regAddr, uint16_t regNum, uint8_t *retData, uint8_t *errorCode) {
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t packetLen = core_build_packet(slaveAddr, regAddr, regNum, ReadHoldReg, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, packetLen);
  if (modbusRecCallBack == NULL) //  回调未注册
  {
    return false;
  } else {
    /*延时后处理接收数据*/
    if (0x0D < regNum)     /*分界是参考电量仪的基本信息数量*/
      osDelay(5 * regNum); /*寄存器数量太多时 增加延时 目前主要是电量仪和UPS的多寄存器使用*/
    else {
      osDelay(MODBUS_WAIT_ECHO_TIME);
    }

    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    // 验证CRC
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
      if (recLen == ERROR_RESP_DATA_LEN) {
        retFlag = false;
        if (errorCode != NULL)
          *errorCode = modbus_packet_buf[2];
      } else {
        // todo 拷贝数据
        uint8_t cpyDataLen = modbus_packet_buf[2];
        memcpy(retData, modbus_packet_buf + 3, cpyDataLen);
      }
    } else {
      // CRC 不正确
      return false;
    }
  }
  return retFlag;
}

bool Master_ReadInputStatus(uint8_t slaveAddr, uint16_t regAddr, uint16_t regNum, uint8_t *retData,
                            uint8_t *errorCode) {
  bool retFlag = true;
  uint16_t rtn_crc = 0;
  uint8_t packetLen = core_build_packet(slaveAddr, regAddr, regNum, ReadInputState, modbus_packet_buf);
  modbusSendCallBack(modbus_packet_buf, packetLen);
  if (modbusRecCallBack == NULL) //  回调未注册
  {
    return false;
  } else {
    /*延时后处理接收数据*/
    osDelay(MODBUS_WAIT_ECHO_TIME);
    memset(modbus_packet_buf, 0x00, sizeof(modbus_packet_buf));
    uint16_t recLen = modbusRecCallBack(modbus_packet_buf);
    // 验证CRC
    if (2 >= recLen || MODBUS_PACKET_BUF_LEN < (recLen - 2))
      return false;
    rtn_crc = (modbus_packet_buf[recLen - 2] << 8 | modbus_packet_buf[recLen - 1]);
    if (GetCRCData(modbus_packet_buf, recLen - 2) == rtn_crc) {
      if (recLen == ERROR_RESP_DATA_LEN) {
        retFlag = false;
        if (errorCode != NULL)
          *errorCode = modbus_packet_buf[2];
      } else {
        // todo 拷贝数据
        uint8_t cpyDataLen = modbus_packet_buf[2];
        memcpy(retData, modbus_packet_buf + 3, cpyDataLen);
      }
    } else {
      // CRC 不正确
      return false;
    }
  }
  return retFlag;
}

static uint8_t core_build_packet(uint8_t slaveAddr, uint16_t regAddr, uint16_t val, uint8_t funcCode, uint8_t *retBuf) {
  uint8_t idx = 0;
  retBuf[idx++] = slaveAddr;
  retBuf[idx++] = funcCode;
  retBuf[idx++] = (uint8_t)(regAddr >> 8);
  retBuf[idx++] = (uint8_t)(regAddr);
  retBuf[idx++] = (uint8_t)(val >> 8);
  retBuf[idx++] = (uint8_t)(val);
  uint16_t crc = GetCRCData(retBuf, idx);
  retBuf[idx++] = (uint8_t)(crc >> 8);
  retBuf[idx++] = (uint8_t)(crc);
  return idx;
}
