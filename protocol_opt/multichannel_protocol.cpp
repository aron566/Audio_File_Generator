/**
  ******************************************************************************
  * @file    multichannel_protocol.cpp
  * @author  aron566
  * @version v1.0
  * @date    2021-11-17
  * @brief   None.
  ******************************************************************************
  */
/* Header includes -----------------------------------------------------------*/
#include "multichannel_protocol.h"
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
/* Macro definitions ---------------------------------------------------------*/
/*
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
#define FRAME_HEADER        0x0566
#define FRAME_MIN_SIZE      8U          /**< 最小帧长 */
#define FRAME_SIZE_CALC(data_len)   (2U+2U+data_len+2U)
#define FRAME_DATA_OFFSET   4

/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
/* Function definitions ------------------------------------------------------*/
/**
 * @brief MultiChannel_Protocol::MultiChannel_Protocol
 * @param parent
 */
MultiChannel_Protocol::MultiChannel_Protocol(QObject *parent) : QObject(parent)
{

}

/**
 * @brief MultiChannel_Protocol::MultiChannel_Protocol
 * @param parent
 * @param CQ_Buf_Obj
 */
MultiChannel_Protocol::MultiChannel_Protocol(QObject *parent, CircularQueue *CQ_Handle, wav_opt *wav_obj)
    : QObject(parent),
      CQ_Buf_Obj(CQ_Handle),
      wav_obj(wav_obj)
{
    /* 初始化定时器 */
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MultiChannel_Protocol::slot_timeout);
    timer->setInterval(1);
    timer->start();
}


void MultiChannel_Protocol::protocol_start()
{
  /* 清空缓冲区数据 */
  CQ_Buf_Obj->CQ_emptyData(CQ_Buf_Obj->get_cq_handle());

  qDebug() << "启动协议栈";

  quint32 len = 0;
  while(run_state)
  {
      QCoreApplication::processEvents();

      len = CQ_Buf_Obj->CQ_getLength(CQ_Buf_Obj->get_cq_handle());

      /* 检测是否满足最小帧长 */
      if(len < FRAME_MIN_SIZE)
      {
        continue;
      }

      /* 检测帧头 */
      if((len = CQ_Buf_Obj->CQ_skipInvaildModbusU16Header(CQ_Buf_Obj->get_cq_handle(), FRAME_HEADER)) < FRAME_MIN_SIZE)
      {
        continue;
      }

      /* 获取数据长度 */
      quint16 data_len = static_cast<quint16>(CQ_Buf_Obj->CQ_ManualGet_Offset_Data(CQ_Buf_Obj->get_cq_handle(), 3));
      data_len <<= 8;
      data_len |= CQ_Buf_Obj->CQ_ManualGet_Offset_Data(CQ_Buf_Obj->get_cq_handle(), 2);
      quint16 frame_len = FRAME_SIZE_CALC(data_len);
      if(len < frame_len)
      {
        continue;
      }

      /* 校验数据 */
      if(frame_len > sizeof(frame_buf))
      {
        CQ_Buf_Obj->CQ_ManualOffsetInc(CQ_Buf_Obj->get_cq_handle(), 1);
        continue;
      }

      CQ_Buf_Obj->CQ_ManualGetData(CQ_Buf_Obj->get_cq_handle(), frame_buf, frame_len);
      if(crc_obj.modbus_get_crc_result(frame_buf, frame_len - 2) == false)
      {
          qDebug() << "crc error.";
          //serial_opt::debug_print(frame_buf, frame_len);

          CQ_Buf_Obj->CQ_ManualOffsetInc(CQ_Buf_Obj->get_cq_handle(), 1);

          emit signal_post_error(0);
          continue;
      }

      /* 完成一帧解析 */
      CQ_Buf_Obj->CQ_ManualOffsetInc(CQ_Buf_Obj->get_cq_handle(), frame_len);

      /* 发出数据 */
      emit signal_post_data(frame_buf + FRAME_DATA_OFFSET, data_len);
  }

//    while(run_state)
//    {
//        QCoreApplication::processEvents();

//        quint32 len = CQ_Buf_Obj->CQ_getLength(CQ_Buf_Obj->get_cq_handle());

//        /* 检测是否满足最小帧长 */
//        if(len < FRAME_MIN_SIZE)
//        {
//            continue;
//        }

//        len = len >= CircularQueue::CQ_BUF_SIZE_ENUM_TypeDef::CQ_BUF_8K?CircularQueue::CQ_BUF_SIZE_ENUM_TypeDef::CQ_BUF_8K:len;

//        CQ_Buf_Obj->CQ_ManualGetData(CQ_Buf_Obj->get_cq_handle(), frame_buf, len);

//        /* 完成一帧解析 */
//        CQ_Buf_Obj->CQ_ManualOffsetInc(CQ_Buf_Obj->get_cq_handle(), len);

//        /* 发出数据 */
//        wav_obj->write_file_data(frame_buf, len);
//        emit signal_post_data(frame_buf, len);
//    }
    qDebug() << "退出协议栈";
}

/**
 * @brief MultiChannel_Protocol::slot_timeout
 */
void MultiChannel_Protocol::slot_timeout()
{

}

/* ---------------------------- end of file ----------------------------------*/
