/**
 *  @file MultiChannel_Protocol.h
 *
 *  @date 2021-11-17
 *
 *  @author aron566
 *
 *  @brief None
 *
 *  @version V1.0
 */
#ifndef MULTICHANNEL_PROTOCOL_H
#define MULTICHANNEL_PROTOCOL_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QTimer>
#include <QRunnable>
#include "CircularQueue.h"
#include "crc.h"
#include "serial_opt.h"
#include "wav_opt.h"
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/


class MultiChannel_Protocol : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MultiChannel_Protocol(QObject *parent = nullptr);

    MultiChannel_Protocol(QObject *parent = nullptr, CircularQueue *CQ_Handle = nullptr, wav_opt *wav_obj = nullptr);

    ~MultiChannel_Protocol() override
    {
        delete timer;
    }
signals:
    void signal_post_data(const quint8 *data, quint16 data_len);
private slots:
    void slot_timeout();
public:
    virtual void run() override
    {
        run_state = true;
        protocol_start();
    }
    void stop()
    {
        run_state = false;
    }
    void protocol_start();
public:
    CircularQueue *CQ_Buf_Obj = nullptr;
    QTimer *timer = nullptr;
    wav_opt *wav_obj = nullptr;
    crc crc_obj;
private:
    bool run_state = false;

};

#endif // MULTICHANNEL_PROTOCOL_H
/******************************** End of file *********************************/
