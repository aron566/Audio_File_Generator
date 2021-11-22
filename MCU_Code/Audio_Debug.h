/**                                                                             
 *  @file Audio_Debug.h                                                    
 *                                                                              
 *  @date 2021年09月28日 08:54:51 星期二
 *                                                                              
 *  @author Copyright (c) 2021 aron566 <aron566@163.com>.                       
 *                                                                              
 *  @brief 1、MONO_FRAME_SIZE 需为2^n. 
 *         2、定义MONO_FRAME_SIZE 用于设定最大8通道缓冲区大小，以及默认单帧发送大小.  
 *         3、默认发送单帧大小 受制于SendBufSize 大小限制，即初始化参数Buf_Size.                                                         
 *         4、适用于16bit数据格式                                                                     
 *  @version V1.0                                                               
 */                                                                             
#ifndef AUDIO_DEBUG_H                                                          
#define AUDIO_DEBUG_H                                                          
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */                          
#include <stddef.h> /**< need definition of NULL    */                          
#include <stdbool.h>/**< need definition of BOOL    */                        
#include <stdio.h>  /**< if need printf             */                          
#include <stdlib.h>                                                             
#include <string.h>                                                             
#include <limits.h> /**< need variable max value    */ 
#include <stdarg.h>                         
/** Private includes ---------------------------------------------------------*/
                                                                 
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///< use C compiler                                          
extern "C" {                                                                  
#endif                                                                          
/** Private defines ----------------------------------------------------------*/
/*
协议如下：
           /-----------/-----------/-----------/-----------/-----------/
          /   帧头    / DATA_SIZE /    DATA   /   CRC16L  /   CRC16H  /
         /-----------/-----------/-----------/-----------/-----------/
        /   0x0566  /    >=2    /   ....    /   1Byte   /   1Byte   /
       /-----------/-----------/-----------/-----------/-----------/
      /   2Bytes  /   2Bytes  / 2<=Bytes  /   1Byte   /   1Byte   /
     /-----------/-----------/-----------/-----------/-----------/
    /--------------校验区域--------------/
ps:小端模式（高地址高位数据，不包括帧头） 可以直接memcpy
*/

/* 模式设置：
  RX2PC_MODE为1时，用于接收SPI总线数据进行中转到PC端，CRC使能时校验数据后中转，否则直接中转 
  RX2PC_MODE为0时，本机音频数据导出，CRC使能时添加校验数据后导出，否则直接导出 
*/
#define AUDIO_EXPORT_RX2PC_MODE   0 /**< 使能接收数据并传输到PC端模式 */
#define AUDIO_EXPORT_ENABLE_CRC   1 /**< 使能CRC安全传输 开启CRC后将增加6字节空间*/ 
#define AUDIO_EXPORT_CHANNEL_MAX  2 /**< 优化栈空间选项>=2，临时发送缓冲区最大值 当为0不设置时默认支持8通道数据*/
/** Exported typedefines -----------------------------------------------------*/
typedef uint32_t (*SEND_DATA_FUNC_PORT_Typedef_t)(uint8_t *, uint32_t);
typedef bool (*GET_IDEL_STATE_PORT_Typedef_t)(void);
/** Exported constants -------------------------------------------------------*/
                                                                                
/** Exported macros-----------------------------------------------------------*/
/* 提供默认采样点数值 */
#ifndef MONO_FRAME_SIZE
  #define MONO_FRAME_SIZE   (128U)
#endif
#ifndef STEREO_FRAME_SIZE
  #define STEREO_FRAME_SIZE (2U*MONO_FRAME_SIZE)
#endif

#define AUDIO_DEBUG_FRAME_MONO_SIZE   MONO_FRAME_SIZE   /**< 单通道数据每帧点数 */
#define AUDIO_DEBUG_FRAME_STEREO_SIZE STEREO_FRAME_SIZE /**< 双通道数据每帧点数 */
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

/**
 * @brief 音频调试初始化
 * 
 * @param Send_Buf 发送缓冲区地址
 * @param Buf_Size 发送缓冲区大小N个uint16_t
 * @param Send_Data_Func 发送接口回调
 * @param Get_Idel_Func 获取空闲状态回调
 */
void Audio_Debug_Init(uint16_t *Send_Buf, uint32_t Buf_Size, SEND_DATA_FUNC_PORT_Typedef_t Send_Data_Func, GET_IDEL_STATE_PORT_Typedef_t Get_Idel_Func);

/**
 * @brief 音频调试启动
 * 
 * @return true 本次传输状态成功
 * @return false 
 */
bool Audio_Debug_Start(void);

#if AUDIO_EXPORT_RX2PC_MODE
  /**
   * @brief 音频数据接收解码验证
   * 
   * @param Data 数据区地址
   * @param Len 数据字节数
   * @return None.
   */
  void Audio_Debug_Put_Data(const uint8_t *Data, uint32_t Len);
#else
  /**
   * @brief 音频数据打包发送
   * 
   * @param Left_Audio_Data 左通道数据
   * @param Right_Audio_Data 右通道数据
   * @param Channel_Number 通道数量 为大于2时有效
   * @param ... 其他通道数据
   */
  void Audio_Debug_Put_Data(const int16_t *Left_Audio_Data, const int16_t *Right_Audio_Data, uint8_t Channel_Number, ...);
#endif

#ifdef __cplusplus ///<end extern c                                             
}                                                                               
#endif                                                                          
#endif                                                                          
/******************************** End of file *********************************/
