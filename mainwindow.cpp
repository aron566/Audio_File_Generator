/**
  ******************************************************************************
  * @file    MainWindow.cpp
  * @author  aron566
  * @version v1.0
  * @date    2021-09-28
  * @brief   None.
  ******************************************************************************
  */
/* Header includes -----------------------------------------------------------*/
#include "mainwindow.h"
#include "ui_mainwindow.h"

/* Macro definitions ---------------------------------------------------------*/
#define PC_SOFTWARE_VERSION         "v1.0"
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

    /*设定软件标题*/
    this->setWindowTitle(tr("Audio File Generator Tool ")+tr(PC_SOFTWARE_VERSION));

    /*建立串口*/
    serial_obj_creator();
}

MainWindow::~MainWindow()
{
    delete ui;

    delete serial_obj;
}

/**
 * @brief MainWindow::serial_obj_creator
 */
void MainWindow::serial_obj_creator()
{
    serial_obj = new serial_opt(QString("com1"));

    connect(serial_obj, &serial_opt::signal_scan_serial_port, this, &MainWindow::slot_scan_serial_port);
    serial_obj->scan_serial_port();
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
        connect_dev_state = serial_obj->open();
    }
    else
    {
        serial_obj->close();
        connect_dev_state = false;
    }
}

/**
 * @brief MainWindow::on_OPEN_FILEpushButton_clicked
 */
void MainWindow::on_OPEN_FILEpushButton_clicked()
{

}

/**
 * @brief MainWindow::on_STARTpushButton_clicked
 */
void MainWindow::on_STARTpushButton_clicked()
{

}

/* ---------------------------- end of file ----------------------------------*/
