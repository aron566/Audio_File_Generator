/**
  ******************************************************************************
  * @file    serial_opt.cpp
  * @author  Copyright (c) 2021 aron566 <aron566@163.com>.
  * @version v1.0
  * @date    2020-12-30
  * @brief   串口操作类.
  ******************************************************************************
  */
/* Header includes -----------------------------------------------------------*/
#include "serial_opt.h"
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QCoreApplication>
/* Macro definitions ---------------------------------------------------------*/
#define DEBUG_SERIAL_PRINTF(str) qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << str
/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
/* Function definitions ------------------------------------------------------*/
/**
 * @brief serial_opt::serial_opt
 * @param parent
 */
serial_opt::serial_opt(QObject *parent) : QObject(parent)
{
    /*设置本地编码对象*/
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
}

/**
 * @brief serial_opt::serial_opt
 * @param port_name
 * @param baud_rate
 */
serial_opt::serial_opt(const QString &port_name, QSerialPort::BaudRate baud_rate)
{
    /*设置本地编码对象*/
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    serial = new QSerialPort;
    if(serial == nullptr)
    {
        return;
    }
    serial->setPortName(port_name);
    serial->setBaudRate(baud_rate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
}

/**
 * @brief serial_opt::~serial_opt
 */
serial_opt::~serial_opt()
{
    if(CQ_Buf_Obj != nullptr)
    {
        delete CQ_Buf_Obj;
    }
    serial->close();
    delete serial;
}

/**
 * @brief serial_opt::set_port_name
 * @param port_name
 */
void serial_opt::set_port_name(const QString &port_name)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setPortName(port_name);
}

/**
 * @brief serial_opt::set_baud_rate
 * @param baud_rate
 */
void serial_opt::set_baud_rate(qint32 baud_rate)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setBaudRate(baud_rate);
}

/**
 * @brief serial_opt::set_data_bits
 * @param data_bits
 */
void serial_opt::set_data_bits(QSerialPort::DataBits data_bits)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setDataBits(data_bits);
}

/**
 * @brief serial_opt::set_parity
 * @param parity
 */
void serial_opt::set_parity(QSerialPort::Parity parity)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setParity(parity);
}

/**
 * @brief serial_opt::set_stop_bits
 * @param stop_bits
 */
void serial_opt::set_stop_bits(QSerialPort::StopBits stop_bits)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setStopBits(stop_bits);
}

/**
 * @brief serial_opt::set_flow_control
 * @param flow_control
 */
void serial_opt::set_flow_control(QSerialPort::FlowControl flow_control)
{
    if(serial == nullptr)
    {
        return;
    }
    serial->setFlowControl(flow_control);
}

/**
 * @brief serial_opt::scan_serial_port
 */
void serial_opt::scan_serial_port()
{
    port_name_list.clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        port_name_list.append(info.portName());
    }
    if(port_name_list.size() <= 0)
    {
       return;
    }
    emit signal_scan_serial_port(port_name_list);
}

/**
 * @brief serial_opt::open
 */
bool serial_opt::open()
{
    if(serial == nullptr)
    {
        return false;
    }
    if(serial->isOpen() == true)
    {
        QMessageBox message(QMessageBox::Warning, tr("警告"), tr("<font size='18' color='red'>串口打开失败！已连接状态！</font>"), QMessageBox::Yes | QMessageBox::No, nullptr);
        /*阻塞*/
        message.exec();
        return true;
    }
    bool isok = serial->open(QIODevice::ReadWrite);
    if(isok == false)
    {
        DEBUG_SERIAL_PRINTF("open the serial dev error.");
        QMessageBox message(QMessageBox::Warning, tr("警告"), tr("<font size='18' color='red'>串口打开失败！请确认连接状态</font>"), QMessageBox::Yes | QMessageBox::No, nullptr);
        message.exec();
        return false;
    }
    connect(serial, &QSerialPort::readyRead, this, &serial_opt::readyRead);
    connect_state = true;
    return isok;
}

/**
 * @brief serial_opt::readyRead
 */
void serial_opt::readyRead()
{
    if(serial == nullptr)
    {
        return;
    }
    if(serial->isOpen() == false)
    {
        return;
    }
    if(serial->bytesAvailable() != 0)
    {
        read_data.clear();
        read_data = serial->readAll();
        emit signal_read_serial_data(read_data);
        if(have_cq_buf == true)
        {
            debug_print(reinterpret_cast<const uint8_t *>(read_data.data()), static_cast<uint32_t>(read_data.size()));
            CircularQueue::CQ_putData(CQ_Buf_Obj->get_cq_handle(), reinterpret_cast<const uint8_t *>(read_data.data()), static_cast<uint32_t>(read_data.size()));
        }
    }
}

/**
 * @brief serial_opt::close
 */
void serial_opt::close()
{
    if(serial == nullptr)
    {
        return;
    }
    if(serial->isOpen() == false)
    {
        return;
    }
    serial->close();
    connect_state = false;
}

/**
 * @brief write
 * @param data
 * @param len
 * @return 写入大小
 */
qint64 serial_opt::write(const char *data, qint64 len)
{
    if(serial == nullptr)
    {
        return -1;
    }
    if(serial->isOpen() == false)
    {
        QMessageBox message(QMessageBox::Warning, tr("警告"), tr("<font size='18' color='red'>串口未打开！请确认连接状态</font>"), QMessageBox::Yes | QMessageBox::No, nullptr);
        message.exec();
        return 0;
    }
    debug_print(reinterpret_cast<const uint8_t *>(data), len);
    return serial->write(data, len);
}

/**
 * @brief write
 * @param data
 * @return 写入大小
 */
qint64 serial_opt::write(const QByteArray &data)
{
    if(serial == nullptr)
    {
        return -1;
    }
    if(serial->isOpen() == false)
    {
        QMessageBox message(QMessageBox::Warning, tr("警告"), tr("<font size='18' color='red'>串口未打开！请确认连接状态</font>"), QMessageBox::Yes | QMessageBox::No, nullptr);
        message.exec();
        return 0;
    }
    return serial->write(data);
}

/**
 * @brief serial_opt::get_baud_rate
 * @return 串口波特率
 */
qint32 serial_opt::get_baud_rate()
{
    return serial->baudRate();
}

/**
 * @brief serial_opt::delay_ms
 * @param ms
 */
void serial_opt::delay_ms(int ms)
{
    QTime t;
    t.restart();
    while(t.elapsed() < ms)
    {
        QCoreApplication::processEvents();
    }
}

/**
 * @brief 16进制格式调试打印
 *
 * @param msg
 * @param msg_len
 */
void serial_opt::debug_print(const uint8_t *msg, uint32_t msg_len)
{
  for(uint32_t i = 0; i < msg_len; i++)
  {
    fprintf(stdout, "%02X ", msg[i]);
    fflush(stdout);
  }
  printf("\n");
  fflush(stderr);
  fflush(stdout);
}

/**
 * @brief warnning_print
 * @param str
 */
void serial_opt::warnning_print(const char *str)
{
    fprintf(stderr, "%s", str);
    fflush(stderr);
}

/**
 * @brief serial_opt::gbk2utf8
 * @return
 */
QByteArray serial_opt::gbk2utf8(const QByteArray& gbk_data)
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec* gbk = QTextCodec::codecForName("GBK");
    QString str = gbk->toUnicode(gbk_data);
    QByteArray utf8_data = utf8->fromUnicode(str);
    return utf8_data;
}

/**
 * @brief serial_opt::gbk2utf8
 * @param chars
 * @return
 */
QByteArray serial_opt::gbk2utf8(const char* gbk_data)
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec* gbk = QTextCodec::codecForName("GBK");
    QString str = gbk->toUnicode(gbk_data);
    QByteArray utf8_data = utf8->fromUnicode(str);
    return utf8_data;
}

/**
 * @brief serial_opt::utf8togbk
 * @return
 */
QByteArray serial_opt::utf8togbk(const QByteArray& utf8_data)
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec* gbk = QTextCodec::codecForName("GBK");
    QString str = utf8->toUnicode(utf8_data);
    QByteArray gbk_data = gbk->fromUnicode(str);
    return gbk_data;
}

/**
 * @brief serial_opt::utf8togbk
 * @param gbk_data
 * @return
 */
QByteArray serial_opt::utf8togbk(const char* utf8_data)
{
    QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
    QTextCodec* gbk = QTextCodec::codecForName("GBK");
    QString str = utf8->toUnicode(utf8_data);
    QByteArray gbk_data = gbk->fromUnicode(str);
    return gbk_data;
}

/**
 * @brief serial_opt::uint2hexstr
 * @param num
 * @return 字符串
 */
QString serial_opt::uint2hexstr(char num)
{
#ifdef USE_NEW_WAY
    QString hexstr = QString("%1").arg(num&0xFF, 2, 16, QLatin1Char('0'));
#else
    QString hexstr;
    hexstr.sprintf("%02X", num);
#endif
    return hexstr;
}

/**
 * @brief serial_opt::char2hexchar
 * @param num
 * @return 字符hex转数字
 */
char serial_opt::hexchar2char(char num)
{
    if((num >= '0') && (num <= '9'))
        return num - 0x30;
    else if((num >= 'A') && (num <= 'F'))
        return num - 'A' +10;
    else if((num >= 'a') && (num <= 'f'))
        return num - 'a' + 10;
    else return (-1);
}

/* ---------------------------- end of file ----------------------------------*/
