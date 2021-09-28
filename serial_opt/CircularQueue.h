/**
 *  @file CircularQueue.h
 *
 *  @date 2021/1/11
 *
 *  @author Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 缓冲区大小需为2的n次方(最高位为1其余为0)
 *
 *  @version v1.4
 */
#ifndef CIRCULARQUEUE_H_
#define CIRCULARQUEUE_H_
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QObject>
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
#define GET_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x) - 1)) == 0))

/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/**
 * @brief The CircularQueue class
 */
class CircularQueue : public QObject
{
    Q_OBJECT
public:
    /** 数据结构体*/
    typedef struct
    {
        union {
            uint8_t 	*data8Buffer;	/**< for 8bit buffer*/
            uint16_t 	*data16Buffer;	/**< for 16bit buffer*/
            uint32_t 	*data32Buffer;	/**< for 32bit buffer*/
            uint64_t 	*data64Buffer;	/**< for 64bit buffer*/
        }Buffer;
        uint32_t size;
        uint32_t entrance;
        uint32_t exit;
        bool is_malloc;
    }CQ_handleTypeDef;

    /*缓冲区长度*/
    typedef enum
    {
        CQ_BUF_128  = 1 << 7,
        CQ_BUF_256  = 1 << 8,
        CQ_BUF_512  = 1 << 9,
        CQ_BUF_1K 	= 1 << (10 * 1), ///< 1*10
        CQ_BUF_2K 	= 1 << 11,
        CQ_BUF_4K 	= 1 << 12,
        CQ_BUF_8K 	= 1 << 13,
        CQ_BUF_1M 	= 1 << (10 * 2), ///< 2*10
        CQ_BUF_1G 	= 1 << (10 * 3), ///< 3*10
        // CQ_BUF_1T 	= 1 << (10 * 4), ///< 4*10
        // CQ_BUF_1P 	= 1 << (10 * 5), ///< 5*10
    }CQ_BUF_SIZE_ENUM_TypeDef;

    typedef enum
    {
        UINT8_DATA_BUF = 0,
        UINT16_DATA_BUF,
        UINT32_DATA_BUF,
        UINT64_DATA_BUF
    }DATA_TYPE_Typedef_t;

public:
    CircularQueue(DATA_TYPE_Typedef_t type, CQ_BUF_SIZE_ENUM_TypeDef size);
    ~CircularQueue()
    {
        delete []  CQ_Handle->Buffer.data64Buffer;/**< 删除缓冲区*/
        delete CQ_Handle;/**< 删除句柄*/
    }
public:
    /*============================= Common ================================*/
    static bool CQ_isEmpty(CQ_handleTypeDef *CircularQueue);
    static bool CQ_isFull(CQ_handleTypeDef *CircularQueue);
    static void CQ_emptyData(CQ_handleTypeDef*CircularQueue);
    static uint32_t CQ_getLength(CQ_handleTypeDef *CircularQueue);
    /*手动缩减缓冲区长度--用作：错误帧偏移-正确帧读取后剔除*/
    static void CQ_ManualOffsetInc(CQ_handleTypeDef *CircularQueue, uint32_t len);

    /*跳转到指定有效数据，用于快速丢弃无效头部数据*/
    static uint32_t CQ_skipInvaildU8Header(CQ_handleTypeDef *cb, uint8_t header_data);
    /*modbus 高位在前-低位在后存储*/
    static uint32_t CQ_skipInvaildModbusU16Header(CQ_handleTypeDef *cb, uint16_t header_data);
    static uint32_t CQ_skipInvaildModbusU32Header(CQ_handleTypeDef *cb, uint32_t header_data);
    /*低位在前-高位在后*/
    static uint32_t CQ_skipInvaildU32Header(CQ_handleTypeDef *cb, uint32_t header_data);
    static uint32_t CQ_skipInvaildU16Header(CQ_handleTypeDef *cb, uint16_t header_data);
    /*删除一个由cb_xxcreate创建的缓冲区*/
    void cb_delete(CQ_handleTypeDef *CircularQueue);

    /*===========================8 Bit Option==============================*/
    static bool CQ_init(CQ_handleTypeDef *CircularQueue, uint8_t *memAdd, uint32_t len);
    /*分配一个缓冲区并进行初始化--替代--CQ_init*/
    static CQ_handleTypeDef *cb_create(uint32_t buffsize);
    static uint32_t CQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);
    static uint32_t CQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len);

    /*修改后的加入数据操作--数据长度作为帧头先存入缓冲区*/
    static uint32_t DQ_putData(CQ_handleTypeDef *CircularQueue, const uint8_t *sourceBuf, uint32_t len);
    /*修改后的取数据操作--会直接读取帧长度信息，依据当前一包长度加入缓冲区*/
    static uint32_t DQ_getData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf);

    /*修改后的获取数据操作--数据读取后不会减小缓冲区长度，需手动减小,目的为了分步取出完整数据*/
    static uint32_t CQ_ManualGetData(CQ_handleTypeDef *CircularQueue, uint8_t *targetBuf, uint32_t len);
    /*修改后的获取数据操作--读取指定偏移的数据，不会减小缓冲区长度,目的为了验证数据，判断帧头等*/
    static uint8_t CQ_ManualGet_Offset_Data(CQ_handleTypeDef *CircularQueue, uint32_t index);

    /*===========================16 Bit Option==============================*/
    /*16bit环形缓冲区初始化*/
    static bool CQ_16_init(CQ_handleTypeDef *CircularQueue, uint16_t *memAdd, uint32_t len);
    /*分配一个缓冲区并进行初始化--替代--CQ_16_init*/
    static CQ_handleTypeDef *cb_16create(uint32_t buffsize);
    /*加入16bit类型数据*/
    static uint32_t CQ_16putData(CQ_handleTypeDef *CircularQueue, const uint16_t * sourceBuf, uint32_t len);
    /*取出16bit类型数据*/
    static uint32_t CQ_16getData(CQ_handleTypeDef *CircularQueue, uint16_t *targetBuf, uint32_t len);

    /*===========================32 Bit Option==============================*/
    /*32bit环形缓冲区初始化*/
    static bool CQ_32_init(CQ_handleTypeDef *CircularQueue, uint32_t *memAdd, uint32_t len);
    /*分配一个缓冲区并进行初始化--替代--CQ_32_init*/
    static CQ_handleTypeDef *cb_32create(uint32_t buffsize);
    /*加入32bit类型数据*/
    static uint32_t CQ_32putData(CQ_handleTypeDef *CircularQueue, const uint32_t * sourceBuf, uint32_t len);
    /*取出32bit类型数据*/
    static uint32_t CQ_32getData(CQ_handleTypeDef *CircularQueue, uint32_t *targetBuf, uint32_t len);

    /*获取CQ_句柄*/
    CQ_handleTypeDef *get_cq_handle()
    {
        return CQ_Handle;
    }
private:
    CQ_handleTypeDef *CQ_Handle = nullptr;
};

#endif
/******************************** End of file *********************************/
