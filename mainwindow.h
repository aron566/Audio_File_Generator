/**
 *  @file .h
 *
 *  @date 2021-
 *
 *  @author aron566
 *
 *  @brief None
 *
 *  @version V1.0
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QMainWindow>
#include <serial_opt/serial_opt.h>
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

public:
    /*ref:https://www.cnblogs.com/zoneofmine/p/10850465.html*/
    typedef struct WAVFILEHEADER
    {
        /*RIFF头*/
        char RiffName[4];       /**< 文件标识字符串 "RIFF"*/
        quint32 nRiffLength;    /**< 头后文件长度 = 文件总长 - 8*/

        /*数据类型标识符*/
        char WavName[4];        /**< 波形文件标识字符串 "WAVE"*/

        /*格式块中的块头*/
        char FmtName[4];        /**< 格式标识字符串 "fmt"*/
        quint32 nFmtLength;     /**< 头后块长度 16 or 18*/
        /*
            一般为16 或者 18 也有 大于18的数值表示格式块中块数据大小，通常为16，
            为18时表示格式块中块数据有附加信息（即扩展域大小-nAppendMessage）
            主要由一些软件制成的wav格式中含有该2个字节，当大于18时，nFmtLength - 18 （或者用nAppendMessage代替）
            即为扩展域信息数据所占的字节数
        */

        /*格式块中的块数据*/
        quint16 nAudioFormat;   /**< 格式标识 PCM为1 其他压缩>1*/
        quint16 nChannleNumber; /**< 声道标识 1单声道 2双声道*/
        quint32 nSampleRate;    /**< 采样率 点/s*/
        quint32 nBytesPerSecond;/**< 平均字节率 Bytes/s = 采样频率 × 音频通道数 × 每次采样得到的样本位数 / 8*/
        quint16 nBytesPerSample;/**< 每个采样需要的字节数 Bytes/sample once = 通道数 × 每次采样得到的样本数据位值／8*/
        quint16 nBitsPerSample; /**< 每次采样得到的样本数据位数值 采样位宽 8位/16位/24位/32位*/

        /*
        扩展域数据，nFmtLength >= 18存在
        quint16 nAppendSize;        固定两个字节描述扩展区域数据长度
        AppendMessage[nAppendSize];
         */

        /*数据块中的块头*/
        char DATANAME[4];
        quint32 nDataLength;
    }WAVFILEHEADER_Typedef_t;

public:
    serial_opt *serial_obj = nullptr;

private:
    void serial_obj_creator();

private slots:
    /**
     * @brief slot_scan_serial_port
     * @param port_name_list
     */
    void slot_scan_serial_port(const QStringList &port_name_list);

    /**
     * @brief on_SCANpushButton_clicked
     */
    void on_SCANpushButton_clicked();
    void on_CONNECTpushButton_clicked();
    void on_OPEN_FILEpushButton_clicked();
    void on_STARTpushButton_clicked();

public:
    bool connect_dev_state = false;
};
#endif // MAINWINDOW_H

/******************************** End of file *********************************/
