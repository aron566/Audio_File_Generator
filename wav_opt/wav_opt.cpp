/**
  ******************************************************************************
  * @file    wav_opt.cpp
  * @author  aron566
  * @version v1.0
  * @date    2021-09-28
  * @brief   None.
  ******************************************************************************
  */
/* Header includes -----------------------------------------------------------*/
#include "wav_opt.h"

/* Macro definitions ---------------------------------------------------------*/
/* Type definitions ----------------------------------------------------------*/
/* Variable declarations -----------------------------------------------------*/
/* Variable definitions ------------------------------------------------------*/
/* Function declarations -----------------------------------------------------*/
/* Function definitions ------------------------------------------------------*/
wav_opt::wav_opt(QWidget *parent) : QWidget(parent)
{

}

wav_opt::~wav_opt()
{

}

/**
 * @brief wav_opt::open_file
 */
QString &wav_opt::open_file()
{
    /*选择文件存储区域*/
    /*参数：父对象，标题，默认路径，格式*/
    QString path = QFileDialog::getSaveFileName(this, "SAVE  *.wav", "../", "WAV(*.wav)");
    if(path.isEmpty() == true)
    {
        return file_name;
    }

    file_name = path;
    file_obj.setFileName(file_name);
    bool is_ok = file_obj.open(QIODevice::ReadWrite);
    if(is_ok == false)
    {
        file_name.clear();
        qDebug() << "set save wav file name error.";
    }
    qDebug() << "set save wav file name " << file_name;
    return file_name;
}

/**
 * @brief wav_opt::set_file_name
 * @param file_name_set
 */
void wav_opt::set_file_name(const QString &file_name_set)
{
    file_name = QString(file_name_set);
    if(file_obj.isOpen() == true)
    {
        file_obj.close();
    }
    file_obj.setFileName(file_name);
    bool is_ok = file_obj.open(QIODevice::ReadWrite);
    if(is_ok == false)
    {
        file_name.clear();
        qDebug() << "set save wav file name error.";
    }
}

/**
 * @brief wav_opt::write_file_data
 * @param data
 */
void wav_opt::write_file_data(const uint8_t *data)
{

}
/* ---------------------------- end of file ----------------------------------*/
