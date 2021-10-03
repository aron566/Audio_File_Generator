/**
 *  @file wav_opt.h
 *
 *  @date 2021-09-28
 *
 *  @author aron566
 *
 *  @brief None
 *
 *  @version V1.0
 */
#ifndef WAV_OPT_H
#define WAV_OPT_H
/** Includes -----------------------------------------------------------------*/
#include <stdint.h> /**< need definition of uint8_t */
#include <stddef.h> /**< need definition of NULL    */
#include <stdbool.h>/**< need definition of BOOL    */
#include <stdio.h>  /**< if need printf             */
#include <stdlib.h>
#include <string.h>
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QWidget>
#include <QFileDialog>/**< 打开文件对话框*/
#include <QFileInfo>/**< 获取文件信息*/
#include <QDebug>
/** Private defines ----------------------------------------------------------*/

/** Exported typedefines -----------------------------------------------------*/

/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/
class wav_opt : public QWidget
{
    Q_OBJECT
public:
    explicit wav_opt(QWidget *parent = nullptr);
    ~wav_opt();
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

    bool run_state = false;
    quint64 file_size = 44;
    quint64 current_file_size = 44;
    QString file_name;
signals:
    void signal_write_complete();

    void signal_current_progress();
public:
    QString &open_file();
    void set_file_name(const QString &file_name_set);
    void set_wav_info(quint64 file_size_set, quint16 nChannleNumber = 2, quint32 nSampleRate = 16000, quint16 nBitsPerSample = 16)
    {
        file_size = file_size_set < 44?44:file_size_set;

        Wave_Header.nChannleNumber = nChannleNumber;
        Wave_Header.nSampleRate = nSampleRate;
        Wave_Header.nBitsPerSample = nBitsPerSample;

        Wave_Header.nBytesPerSecond = nSampleRate * nChannleNumber * nBitsPerSample / 8;
        Wave_Header.nBytesPerSample = nChannleNumber * nBitsPerSample / 8;

        /*定位至文件写入区域*/
        if(file_obj.isOpen() == false)
        {
            file_obj.open(QIODevice::ReadWrite);
        }
        file_obj.seek(44);

        run_state = true;
    }
    void write_file_data(const uint8_t *data, uint32_t len);
    void stop_write();
private:
    WAVFILEHEADER_Typedef_t Wave_Header =
    {
        .RiffName = {'R','I','F','F'},
        .nRiffLength = 44,
        .WavName = {'W', 'A', 'V', 'E'},
        .FmtName = {'f', 'm', 't', ' '},
        .nFmtLength = 16,
        .nAudioFormat = 1,
        .nChannleNumber = 2,
        .nSampleRate = 16000,
        .nBytesPerSecond = 16000 * 2 * 16 / 8,
        .nBytesPerSample = 4,
        .nBitsPerSample = 16,
        .DATANAME = {'d', 'a', 't', 'a'},
        .nDataLength = 0,
    };

    QFile file_obj;        /**< 文件对象*/
};

#endif // WAV_OPT_H
/******************************** End of file *********************************/
