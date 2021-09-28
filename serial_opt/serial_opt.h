/**
 *  @file serial_opt.h
 *
 *  @date 2020-12-30
 *
 *  @author Copyright (c) 2021 aron566 <aron566@163.com>.
 *
 *  @brief 使用需添加模块：QT       += serialport
 *
 *  @version V1.0
 */
#ifndef SERIAL_OPT_H
#define SERIAL_OPT_H
/** Includes -----------------------------------------------------------------*/
/** Private includes ---------------------------------------------------------*/
#include <QObject>
#include <QTime>
#include <QtSerialPort/QSerialPort>         ///< 提供访问串口的功能
#include <QtSerialPort/QSerialPortInfo>     ///< 提供系统中存在的串口的信息
#include <serial_opt/CircularQueue.h>       ///< 环形缓冲区
/** Private defines ----------------------------------------------------------*/
#define QT_BUILD_USER_SERIALOPT_LIB
QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_USER_SERIALOPT_LIB)
#    define USER_SERIALOPT_EXPORT Q_DECL_EXPORT
#  else
#    define USER_SERIALOPT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define USER_SERIALOPT_EXPORT
#endif

QT_END_NAMESPACE
/** Exported typedefines -----------------------------------------------------*/
/** Exported constants -------------------------------------------------------*/

/** Exported macros-----------------------------------------------------------*/
/** Exported variables -------------------------------------------------------*/
/** Exported functions prototypes --------------------------------------------*/
/**
 * @brief The serial_opt class
 */
class USER_SERIALOPT_EXPORT serial_opt : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief serial_opt
     * @param parent
     */
    explicit serial_opt(QObject *parent = nullptr);

    /**
     * @brief serial_opt
     * @param port_name
     * @param baud_rate
     */
    serial_opt(const QString &port_name, QSerialPort::BaudRate baud_rate = QSerialPort::Baud115200);

    ~serial_opt();

    /**
     * @brief create_cq_buf
     * @param size
     */
    void create_cq_buf(CircularQueue::CQ_BUF_SIZE_ENUM_TypeDef size)
    {
        CQ_Buf_Obj = new CircularQueue(CircularQueue::DATA_TYPE_Typedef_t::UINT8_DATA_BUF, size);
        if(CQ_Buf_Obj != nullptr)
        {
            have_cq_buf = true;
        }
    }
signals:
    void signal_scan_serial_port(const QStringList &port_name_list);
    void signal_read_serial_data(const QByteArray &data);
public slots:
    void readyRead();
public:
    void set_port_name(const QString &port_name);
    void set_baud_rate(QSerialPort::BaudRate baud_rate = QSerialPort::Baud115200);
    void set_data_bits(QSerialPort::DataBits data_bits = QSerialPort::Data8);
    void set_parity(QSerialPort::Parity parity = QSerialPort::NoParity);
    void set_stop_bits(QSerialPort::StopBits stop_bits = QSerialPort::OneStop);
    void set_flow_control(QSerialPort::FlowControl flow_control = QSerialPort::NoFlowControl);
    void scan_serial_port();
    bool open();
    void close();
    qint64 write(const char *data, qint64 len);
    qint64 write(const QByteArray &data);

    qint32 get_baud_rate();

    static void delay_ms(int ms);
    static void debug_print(const uint8_t *msg, uint32_t msg_len);
    static void warnning_print(const char *str);
    static QByteArray gbk2utf8(const QByteArray&);
    static QByteArray gbk2utf8(const char* gbk_data);
    static QByteArray utf8togbk(const QByteArray&);
    static QByteArray utf8togbk(const char* gbk_data);

    static QString uint2hexstr(char num);
    static char hexchar2char(char num);

public:
    bool connect_state = false;
private:
    QSerialPort *serial = nullptr;

public:
    CircularQueue *CQ_Buf_Obj = nullptr;/**< 可选建立环形缓冲区*/
    bool have_cq_buf = false;
protected:
    QStringList port_name_list;
    QByteArray read_data;
};

#endif // SERIAL_OPT_H
/******************************** End of file *********************************/
