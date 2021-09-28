/**
 *  @file mainwindow.h
 *
 *  @date 2021-09-28
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
#include <QFontDatabase>
#include <serial_opt/serial_opt.h>
#include <wav_opt/wav_opt.h>
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
signals:
    void signal_write_file_data(const uint8_t *data, uint32_t len);
private:
    Ui::MainWindow *ui;

public:
    serial_opt *serial_obj = nullptr;

    wav_opt *wav_obj = nullptr;

private:
    QFontDatabase fontDb;
private:
    void font_file_load();

    void set_push_button_style(int font_size);

    void serial_obj_creator();

    void wav_obj_creator();
private:
    /*重新绘制界面字体大小*/
    virtual void resizeEvent(QResizeEvent *event) override;
private slots:
    /**
     * @brief slot_scan_serial_port
     * @param port_name_list
     */
    void slot_scan_serial_port(const QStringList &port_name_list);

    /**
     * @brief slot_read_serial_data
     * @param data
     */
    void slot_read_serial_data(const QByteArray &data);

    /**
     * @brief slot_write_complete
     */
    void slot_write_complete();

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
