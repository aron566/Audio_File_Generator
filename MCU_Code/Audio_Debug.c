/**                                                                             
 *  @file Audio_Debug.c                                                   
 *                                                                              
 *  @date 2021年09月28日 08:55:20 星期二
 *
 *  @author aron566
 *
 *  @copyright Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 音频数据打包发送至上位机，由上位机合并成wav文件.
 *
 *  @details 1、发送区大小应至少满足：AUDIO_DEBUG_FRAME_STEREO_SIZE.
 *           2、发送音频至少为双通道，通过设置将某一通道数据复制到另一通道.
 *           3、最大支持8通道数据传输，发送缓冲区必须随之增大 8*AUDIO_DEBUG_FRAME_MONO_SIZE.
 *           4、数据格式：LEFT RIGHT LEFT RIGHT......
 *           5、多通道下数据格式：CH1 CH2 CH3 .... CH1 CH2 CH3 ....
 *
 *  @version V1.0
 */
/** Includes -----------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "Audio_Debug.h"
#include "CircularQueue.h"
#if AUDIO_EXPORT_ENABLE_CRC
  #include "utilities_crc.h"
#endif
/** Use C compiler -----------------------------------------------------------*/
#ifdef __cplusplus ///< use C compiler
extern "C" {
#endif
/** Private typedef ----------------------------------------------------------*/
/* 发送区音频 */
typedef struct 
{
  uint16_t *Send_Buf_Ptr;
  uint32_t Send_Buf_Size;
  SEND_DATA_FUNC_PORT_Typedef_t Send_Audio_Data;
  GET_IDEL_STATE_PORT_Typedef_t Get_Idel_State;
}SEND_BUF_Typedef_t;

/* 通道使能 */
typedef enum
{
  CHANNEL_0_NONE = 0,
  CHANNEL_1_EN,
  CHANNEL_2_EN,
  CHANNEL_3_EN,
  CHANNEL_4_EN,
  CHANNEL_5_EN,
  CHANNEL_6_EN,
  CHANNEL_7_EN,
  CHANNEL_8_EN,
  CHANNEL_NUMBER_MAX
}AUDIO_DEBUG_CHANNEL_SEL_Typedef_t;                                                     
/** Private macros -----------------------------------------------------------*/
#define AUDIO_DATA_BUF_SIZE CQ_BUF_2KB//(CHANNEL_8_EN*AUDIO_DEBUG_FRAME_MONO_SIZE)/**< 环形缓冲区大小 取2K */      

#if AUDIO_EXPORT_ENABLE_CRC
  #define AUDIO_EXPORT_HEADER_SIZE        6U      /**< 头部+长度+CRC */
  #define AUDIO_EXPORT_FRAME_HEADER       0x0566U /**< 头部 */
  #define AUDIO_EXPORT_DATA_OFFSET        4U      /**< 数据存储偏移 */
  #define AUDIO_EXPORT_FRAME_MIN_SIZE     8U      /**< 最小帧长 */
  #define AUDIO_EXPORT_FRAME_SIZE_CALC(data_len)   (2U+2U+data_len+2U)
#endif
/** Private constants --------------------------------------------------------*/
/** Public variables ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/
/* 音频缓冲区 */
static uint16_t Audio_Data_Buf[AUDIO_DATA_BUF_SIZE];
static CQ_handleTypeDef CQ_Audio_Data_Handle;

/* 发送设置 */
static AUDIO_DEBUG_CHANNEL_SEL_Typedef_t Current_Channel_Sel = CHANNEL_2_EN;
static uint32_t Current_Send_Size = AUDIO_DEBUG_FRAME_STEREO_SIZE;
/* 发送区 */
static SEND_BUF_Typedef_t Send_Region;
/** Private function prototypes ----------------------------------------------*/
                                                                                
/** Private user code --------------------------------------------------------*/
                                                                     
/** Private application code -------------------------------------------------*/
/*******************************************************************************
*                                                                               
*       Static code                                                             
*                                                                               
********************************************************************************
*/                                                                              
/**
  ******************************************************************
  * @brief   音频调试通道使能
  * @param   [in]Ch_Sel 使能通道. 
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-09-28
  ******************************************************************
  */
static void Audio_Debug_Channel_Set(AUDIO_DEBUG_CHANNEL_SEL_Typedef_t Ch_Sel)
{
  if(Current_Channel_Sel == Ch_Sel || Ch_Sel == CHANNEL_0_NONE)
  {
    return;
  }
  Current_Channel_Sel = Ch_Sel;
  uint32_t Number = (uint32_t)Current_Channel_Sel > (uint32_t)CHANNEL_2_EN?(uint32_t)Current_Channel_Sel:(uint32_t)CHANNEL_2_EN;
  Current_Send_Size = Number * AUDIO_DEBUG_FRAME_MONO_SIZE;
  CQ_emptyData(&CQ_Audio_Data_Handle);
}
/** Public application code --------------------------------------------------*/
/*******************************************************************************
*                                                                               
*       Public code                                                             
*                                                                               
********************************************************************************
*/

/**
  ******************************************************************
  * @brief   音频调试启动
  * @param   [in]None. 
  * @return  true 成功
  * @author  aron566
  * @version V1.0
  * @date    2021-09-28
  ******************************************************************
  */
bool Audio_Debug_Start(void)
{
  if(Send_Region.Get_Idel_State() == false)
  {
    return false;
  }  
  uint32_t Len = CQ_getLength(&CQ_Audio_Data_Handle);

  /* 解码验证到PC模式二次中转 */
#if AUDIO_EXPORT_RX2PC_MODE

  /* 检测是否满足最小帧长 */
  if(Len < AUDIO_EXPORT_FRAME_MIN_SIZE)
  {
    return false;
  }

  /* 检测帧头 */
  if((Len = CQ_skipInvaildModbusU16Header(&CQ_Audio_Data_Handle, AUDIO_EXPORT_FRAME_HEADER)) < AUDIO_EXPORT_FRAME_MIN_SIZE)
  {
    return false;
  }

  /* 获取数据长度 */
  uint16_t data_len = (uint16_t)((CQ_ManualGet_Offset_Data(&CQ_Audio_Data_Handle, 3)) << 8);
  data_len |= CQ_ManualGet_Offset_Data(&CQ_Audio_Data_Handle, 2);
  uint16_t frame_len = AUDIO_EXPORT_FRAME_SIZE_CALC(data_len);
  if(Len < frame_len)
  {
    return false;
  }

  /* 校验数据 */
  CQ_ManualGetData(&CQ_Audio_Data_Handle, (uint8_t *)Send_Region.Send_Buf_Ptr, frame_len);
  if(modbus_get_crc_result((uint8_t *)Send_Region.Send_Buf_Ptr, frame_len-2) == false)
  {
    CQ_ManualOffsetInc(&CQ_Audio_Data_Handle, 1);
    return false;
  }

  /* 完成一帧解析 */
  CQ_ManualOffsetInc(&CQ_Audio_Data_Handle, frame_len);

  /* 发出数据 */
  Send_Region.Send_Audio_Data((uint8_t *)Send_Region.Send_Buf_Ptr, frame_len);
  return true;
#else  
  if(Len < Current_Send_Size)
  {
    return false;
  }

    /* 发送格式化带数据校验或者不带 */
  #if AUDIO_EXPORT_ENABLE_CRC
    /* 依据buf大小取出数据，算上协议格式大小 */
    uint32_t Size = (Current_Send_Size+(AUDIO_EXPORT_HEADER_SIZE/sizeof(uint16_t))) > Send_Region.Send_Buf_Size? \
                    Send_Region.Send_Buf_Size:(Current_Send_Size+(AUDIO_EXPORT_HEADER_SIZE/sizeof(uint16_t)));
    /* 数据长度 */
    uint32_t Data_Len = Size-(AUDIO_EXPORT_HEADER_SIZE/sizeof(uint16_t));
    /* 设置数据长度 */
    Send_Region.Send_Buf_Ptr[1] = Data_Len;
    /* 取出音频数据 */
    CQ_16getData(&CQ_Audio_Data_Handle, Send_Region.Send_Buf_Ptr+(AUDIO_EXPORT_DATA_OFFSET/sizeof(uint16_t)), \
                    Data_Len);
    /* 计算CRC */
    uint16_t crc_val = modbus_crc_return_with_table((uint8_t *)Send_Region.Send_Buf_Ptr, Data_Len * sizeof(int16_t) + AUDIO_EXPORT_DATA_OFFSET);
    memcpy(Send_Region.Send_Buf_Ptr + (AUDIO_EXPORT_DATA_OFFSET/sizeof(uint16_t)) + Data_Len, &crc_val, sizeof(uint16_t));
    return true;
  #else
    /* 依据buf大小取出数据 */
    uint32_t Size = Current_Send_Size > Send_Region.Send_Buf_Size?Send_Region.Send_Buf_Size:Current_Send_Size;
    CQ_16getData(&CQ_Audio_Data_Handle, Send_Region.Send_Buf_Ptr, Size);
    Send_Region.Send_Audio_Data((uint8_t *)Send_Region.Send_Buf_Ptr, Size * sizeof(int16_t));
    return true;
  #endif 
#endif
}

#if AUDIO_EXPORT_RX2PC_MODE
  /**
   * @brief 音频数据接收
   * 
   * @param Data 数据区地址
   * @param Len 数据字节数
   * @return None.
   */
  void Audio_Debug_Put_Data(const uint8_t *Data, uint32_t Len)
  {
    CQ_putData(&CQ_Audio_Data_Handle, Data, Len);
  }
#else
  /**
    ******************************************************************
    * @brief   音频数据打包发送
    * @param   [in]Left_Audio_Data 左音频数据
    * @param   [in]Right_Audio_Data 右音频数据
    * @param   [in]Channel_Number 其他通道总数
    * @param   [in]... 其他通道数据
    * @return  None.
    * @author  aron566
    * @version V1.0
    * @date    2021-09-28
    ******************************************************************
    */
  void Audio_Debug_Put_Data(const int16_t *Left_Audio_Data, const int16_t *Right_Audio_Data, uint8_t Channel_Number, ...)
  {
  #if AUDIO_EXPORT_CHANNEL_MAX == 0  
    int16_t Audio_Data[8*AUDIO_DEBUG_FRAME_MONO_SIZE];
  #else
    int16_t Audio_Data[AUDIO_EXPORT_CHANNEL_MAX*AUDIO_DEBUG_FRAME_MONO_SIZE];
  #endif
    va_list args;
    
    uint32_t index = 0;
    /* 更新当前通道 */
    Audio_Debug_Channel_Set((AUDIO_DEBUG_CHANNEL_SEL_Typedef_t) Channel_Number);
    
    for(uint32_t i = 0; i < AUDIO_DEBUG_FRAME_MONO_SIZE; i++)
    {
      switch(Current_Channel_Sel)
      {
        case CHANNEL_1_EN:
          Audio_Data[index++] = Left_Audio_Data[i];
          Audio_Data[index++] = Left_Audio_Data[i];
          break;
        case CHANNEL_2_EN:
          Audio_Data[index++] = Left_Audio_Data[i];
          Audio_Data[index++] = Right_Audio_Data[i];
          break;
        case CHANNEL_3_EN:
        case CHANNEL_4_EN:
        case CHANNEL_5_EN:
        case CHANNEL_6_EN:
        case CHANNEL_7_EN:
        case CHANNEL_8_EN:
        {
          Audio_Data[index++] = Left_Audio_Data[i];
          Audio_Data[index++] = Right_Audio_Data[i];
          
          /* args point to the first variable parameter */
          va_start(args, Channel_Number);
          for(uint8_t Channel_Index = 0; Channel_Index < Channel_Number; Channel_Index++)
          {
            Audio_Data[index++] = (va_arg(args, uint16_t *))[i];
          }
          break; 
        }
        default:
          Audio_Data[index++] = 0;
          Audio_Data[index++] = 0;
          break;
      }
    }
    va_end(args);
    CQ_16putData(&CQ_Audio_Data_Handle, (const uint16_t *)Audio_Data, (2 + Channel_Number) * AUDIO_DEBUG_FRAME_MONO_SIZE);
  }
#endif

/**
  ******************************************************************
  * @brief   音频调试初始化
  * @param   [in]Send_Buf.
  * @param   [in]Buf_Szie 发送缓冲大小.
  * @param   [in]Send_Data_Func 发送数据接口.
  * @param   [in]Get_Idel_Func 获取空闲状态.
  * @return  None.
  * @author  aron566
  * @version V1.0
  * @date    2021-09-28
  ******************************************************************
  */
void Audio_Debug_Init(uint16_t *Send_Buf, uint32_t Buf_Size, SEND_DATA_FUNC_PORT_Typedef_t Send_Data_Func, GET_IDEL_STATE_PORT_Typedef_t Get_Idel_Func)
{
  /* 初始化发送区 */
  Send_Region.Send_Buf_Ptr = Send_Buf;
  Send_Region.Send_Buf_Size = Buf_Size;
  Send_Region.Send_Audio_Data = Send_Data_Func;
  Send_Region.Get_Idel_State = Get_Idel_Func;

#if AUDIO_EXPORT_ENABLE_CRC
  Send_Region.Send_Buf_Ptr[0] = ((uint16_t)(AUDIO_EXPORT_FRAME_HEADER<<8)&0xFF00) | (uint16_t)((AUDIO_EXPORT_FRAME_HEADER>>8)&0x00FF);
#endif

  /* 初始化缓冲区 */
#if AUDIO_EXPORT_RX2PC_MODE
  CQ_init(&CQ_Audio_Data_Handle, (uint8_t *)Audio_Data_Buf, AUDIO_DATA_BUF_SIZE*2);
#else
  CQ_16_init(&CQ_Audio_Data_Handle, Audio_Data_Buf, AUDIO_DATA_BUF_SIZE);
#endif  
}
#ifdef __cplusplus ///<end extern c                                             
}                                                                               
#endif                                                                          
/******************************** End of file *********************************/
