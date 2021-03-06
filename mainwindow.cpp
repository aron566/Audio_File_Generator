/**
  ******************************************************************************
  * @file    MainWindow.cpp
  * @author  aron566
  * @details v1.1.5 优化协议栈，优化串口数据处理满时丢弃，修复UI界面显示标题缺失问题.
  * @version v1.1.5
  * @date    2021-09-28
  * @brief   None.
  ******************************************************************************
  */
/* Header includes -----------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
/* Macro definitions ---------------------------------------------------------*/
#define PC_SOFTWARE_VERSION         "v1.1.5"
/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
/* Function definitions ------------------------------------------------------*/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Apply style sheet */
    QFile file(":/qdarkstyle/dark/style.qss");
    if(file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        this->setStyleSheet(file.readAll());
        file.close();
    }

    /*装载字体*/
    font_file_load();

    /*设定软件标题*/
    this->setWindowTitle(tr("Audio File Generator Tool ")+tr(PC_SOFTWARE_VERSION)+tr(" By aron566"));

    /*建立wav文件对象*/
    wav_obj_creator();

    /*初始化定时器*/
    timer_init();
}

MainWindow::~MainWindow()
{
    delete ui;

    delete serial_obj;

    delete wav_obj;

    delete timer;
}

/**
 * @brief MainWindow::font_file_load
 */
void MainWindow::font_file_load()
{
    if(!fontDb.families().contains("FontAwesome"))
    {
        int fontId = fontDb.addApplicationFont(":/ico_ttf/fontawesome-webfont.ttf");
        QStringList fontName = fontDb.applicationFontFamilies(fontId);
        if(fontName.count() == 0)
        {
            qDebug() << "load fontawesome-webfont.ttf error";
        }
    }
}

/**
 * @brief MainWindow::set_push_button_style
 * @param font_size
 */
void MainWindow::set_push_button_style(int font_size)
{
    QFont menu_font;
    if(fontDb.families().contains("FontAwesome"))
    {
        menu_font = QFont("FontAwesome");
        menu_font.setHintingPreference(QFont::PreferNoHinting);
    }
    else
    {
        qDebug() << "the FontAwesome ttf file load faild.";
        return;
    }
    menu_font.setPixelSize(font_size);

    ui->CONNECTpushButton->setFont(menu_font);
    if(connect_dev_state == false)
    {
        ui->CONNECTpushButton->setText(QString("%1%2").arg(QChar(0xf127)).arg(tr(" 连接设备")));
        ui->CONNECTpushButton->setStyleSheet("color:white;");
    }
    else
    {
        ui->CONNECTpushButton->setText(QString("%1%2").arg(QChar(0xf0c1)).arg(tr(" 断开连接")));
        ui->CONNECTpushButton->setStyleSheet("color:red;");
    }

    ui->STARTpushButton->setFont(menu_font);
    ui->STARTpushButton->setText(QString("%1%2").arg(QChar(0xf016)).arg(tr(" 启动录制")));
    ui->STARTpushButton->setStyleSheet("color:white;");
}


/**
 * @brief MainWindow::resizeEvent
 * @param event
 */
void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    QFont menu_font;
    if(fontDb.families().contains("FontAwesome"))
    {
       menu_font = QFont("FontAwesome");
       menu_font.setHintingPreference(QFont::PreferNoHinting);
    }
    else
    {
       qDebug() << "the FontAwesome ttf file load faild.";
       return;
    }

    if(window()->width() >= 1600 && window()->height() >= 900)
    {
       set_push_button_style(18);
    }
    else
    {
       set_push_button_style(16);
    }
}

/**
 * @brief MainWindow::serial_obj_creator
 */
void MainWindow::serial_obj_creator()
{
    serial_obj = new serial_opt(QString("com1"));
    connect(serial_obj, &serial_opt::signal_read_serial_data, this, &MainWindow::slot_read_serial_data);
    connect(serial_obj, &serial_opt::signal_scan_serial_port, this, &MainWindow::slot_scan_serial_port);
    serial_obj->scan_serial_port();

    /* 建立环形缓冲区 */
    serial_obj->create_cq_buf(CircularQueue::CQ_BUF_1M);

    /* 建立协议栈 */
    protocol_obj = new MultiChannel_Protocol(serial_obj, serial_obj->CQ_Buf_Obj, wav_obj);
    connect(protocol_obj, &MultiChannel_Protocol::signal_post_data, this, &MainWindow::slot_post_data);
    connect(protocol_obj, &MultiChannel_Protocol::signal_post_error, this, &MainWindow::slot_post_error);
}

/**
 * @brief MainWindow::wav_obj_creator
 */
void MainWindow::wav_obj_creator()
{
    wav_obj = new wav_opt;
    connect(wav_obj, &wav_opt::signal_write_complete, this, &MainWindow::slot_write_complete);

    /*建立串口*/
    serial_obj_creator();
}

/**
 * @brief MainWindow::timer_init
 */
void MainWindow::timer_init()
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::slot_timeout);
    timer->setInterval(1);
}

/**
 * @brief MainWindow::slot_scan_serial_port
 * @param port_name_list
 */
void MainWindow::slot_scan_serial_port(const QStringList &port_name_list)
{
   ui->COM_LISTcomboBox->clear();
   for(int index = 0; index < port_name_list.size(); index++)
   {
       ui->COM_LISTcomboBox->addItem(port_name_list.value(index));
   }
}

/**
 * @brief MainWindow::slot_read_serial_data
 * @param data
 */
void MainWindow::slot_read_serial_data(const QByteArray &data)
{
    if(wav_obj == nullptr)
    {
        return;
    }
    if(wav_obj->run_state == false)
    {
        return;
    }

    /* 数据校验开关检测 */
    if(ui->CRC_checkBox->isChecked() == false)
    {
        wav_obj->write_file_data(reinterpret_cast<const uint8_t *>(data.data()), static_cast<uint32_t>(data.size()));
    }
    /* 数据视口开关检测 */
    if(ui->DATA_VIEWcheckBox->isChecked() == false)
    {
      return;
    }
    ui->DATA_VIEWtextBrowser->append(QString(tr("Rec:")));
    ui->DATA_VIEWtextBrowser->insertPlainText(data.toHex());
}

/**
 * @brief MainWindow::slot_post_data
 * @param data
 * @param data_len
 */
void MainWindow::slot_post_data(const quint8 *data, quint16 data_len)
{
    /* 数据校验开关检测 */
    if(ui->CRC_checkBox->isChecked() == false)
    {
        return;
    }
    wav_obj->write_file_data(data, data_len);
}

/**
 * @brief MainWindow::slot_post_error
 * @param data
 * @param data_len
 */
void MainWindow::slot_post_error(quint8 type)
{
  if(type == 0)
  {
    ui->statusbar->showMessage((QString(tr("crc err!"))), 500);
  }
}

/**
 * @brief MainWindow::slot_write_complete
 */
void MainWindow::slot_write_complete()
{
    protocol_obj->stop();
    ui->progressBar->setValue(ui->progressBar->maximum());
    ui->STARTpushButton->setText(QString("%1%2").arg(QChar(0xf016)).arg(tr(" 启动录制")));
    ui->STARTpushButton->setStyleSheet("color:white;");
    QMessageBox message(QMessageBox::Information, tr("通知"), tr("<font size='10' color='green'>录音完成！</font>"), QMessageBox::Yes, nullptr);
    message.exec();
}

/**
 * @brief MainWindow::slot_timeout
 */
void MainWindow::slot_timeout()
{
   if(wav_obj->run_state == false)
   {
       return;
   }
//   qDebug() << "current_file_size:" << wav_obj->current_file_size <<"/" << ui->progressBar->maximum();
   ui->progressBar->setValue(static_cast<int>(wav_obj->current_file_size));
}

/**
 * @brief MainWindow::on_SCANpushButton_clicked
 */
void MainWindow::on_SCANpushButton_clicked()
{
    serial_obj->scan_serial_port();
}

/**
 * @brief MainWindow::on_CONNECTpushButton_clicked
 */
void MainWindow::on_CONNECTpushButton_clicked()
{
    if(connect_dev_state == false)
    {
        serial_obj->set_port_name(ui->COM_LISTcomboBox->currentText());
        serial_obj->set_baud_rate(ui->BOUDRATEcomboBox->currentText().toInt());
        connect_dev_state = serial_obj->open();
    }
    else
    {
        serial_obj->close();
        connect_dev_state = false;
    }
    if(connect_dev_state == false)
    {
        ui->CONNECTpushButton->setText(QString("%1%2").arg(QChar(0xf127)).arg(tr(" 连接设备")));
        ui->CONNECTpushButton->setStyleSheet("color:white;");
    }
    else
    {
        ui->CONNECTpushButton->setText(QString("%1%2").arg(QChar(0xf0c1)).arg(tr(" 断开连接")));
        ui->CONNECTpushButton->setStyleSheet("color:red;");
    }
}

/**
 * @brief MainWindow::on_OPEN_FILEpushButton_clicked
 */
void MainWindow::on_OPEN_FILEpushButton_clicked()
{
    wav_obj->run_state = false;
    ui->progressBar->setValue(0);
    ui->FILE_NAMElineEdit->setText(wav_obj->open_file());
}

/**
 * @brief MainWindow::on_STARTpushButton_clicked
 */
void MainWindow::on_STARTpushButton_clicked()
{
    if(wav_obj->run_state == false)
    {
        quint16 nChannleNumber = ui->CHANNEL_NUMcomboBox->currentText().toUShort();
        quint32 nSampleRate = ui->SAMPLERATEcomboBox->currentText().toUInt();
        quint16 nBitsPerSample = ui->AUDIO_BITcomboBox->currentText().toUShort();
        quint64 record_sec = ui->FILE_SIZElineEdit->text().toULongLong();
        record_sec = record_sec < 1?0xFFFF:record_sec;
        qDebug() << "set record sec:" << record_sec << "s";
        if(ui->FILE_NAMElineEdit->text() != wav_obj->file_name)
        {
            wav_obj->set_file_name(ui->FILE_NAMElineEdit->text());
        }
        wav_obj->set_wav_info(record_sec, nChannleNumber, nSampleRate, nBitsPerSample);
        /*设置进度条*/
        ui->progressBar->setMaximum(static_cast<int>(wav_obj->file_size));
        ui->STARTpushButton->setText(QString("%1%2").arg(QChar(0xf016)).arg(tr(" 停止录制")));
        ui->STARTpushButton->setStyleSheet("color:red;");
        ui->DATA_VIEWtextBrowser->clear();
        ui->DATA_VIEWtextBrowser->clearHistory();

        timer->start();

        /* 数据校验开关检测 */
        if(ui->CRC_checkBox->isChecked() == true)
        {
            protocol_obj->run();
        }
    }
    else
    {
        protocol_obj->stop();

        wav_obj->stop_write();
        ui->STARTpushButton->setText(QString("%1%2").arg(QChar(0xf016)).arg(tr(" 启动录制")));
        ui->STARTpushButton->setStyleSheet("color:white;");

        timer->stop();
    }
}

/**
 * @brief MainWindow::on_BOUDRATEcomboBox_currentTextChanged
 * @param arg1
 */
void MainWindow::on_BOUDRATEcomboBox_currentTextChanged(const QString &arg1)
{
    serial_obj->set_baud_rate(arg1.toInt());
}

/**
 * @brief MainWindow::on_COM_LISTcomboBox_currentTextChanged
 * @param arg1
 */
void MainWindow::on_COM_LISTcomboBox_currentTextChanged(const QString &arg1)
{
    serial_obj->set_port_name(arg1);
}
/* ---------------------------- end of file ----------------------------------*/
