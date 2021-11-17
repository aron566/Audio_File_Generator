/**
 *  @file crc.h
 *
 *  @date 2021-03-28
 *
 *  @author Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief crc校验
 *
 *  @version V1.0
 */
#ifndef CRC_H
#define CRC_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/

/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/
class crc
{
public:

    /*查表计算CRC16*/
    static uint16_t get_crc(uint8_t *puchMsg, uint16_t usDataLen);
    /*查表计算CRC16并对比数据包中的CRC，返回结果，数据包含crc计算内容+crc结果，否则可能指针越界访问*/
    static bool get_crc_result(uint8_t *puchMsg, uint16_t usDataLen);

    /*计算CRC*/
    static uint16_t modbus_crc_return(uint8_t *data, uint16_t data_len);
    /*查表法计算modbus CRC16*/
    static uint16_t modbus_crc_return_with_table(uint8_t *data, uint16_t data_len);
    /*计算CRC16并对比数据包中的CRC，返回结果，数据包含crc计算内容+crc结果，否则可能指针越界访问*/
    static bool modbus_get_crc_result(uint8_t *msg, uint16_t len);
private:
    static uint16_t modbus_crc_cal(uint16_t Data, uint16_t GenPoly, uint16_t CrcData);
};
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

#endif
/******************************** End of file *********************************/
