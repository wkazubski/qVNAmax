#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include "mainwindow.h"
#include <QDialog>

namespace Ui {
class ColorDialog;
}

class ColorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ColorDialog(QWidget *parent = 0);
    ~ColorDialog();

    static AllLinesStruct line;

private slots:
    void on_SWRCheckBox_clicked(bool checked);

    void on_RLCheckBox_clicked(bool checked);

    void on_AngleCheckBox_clicked(bool checked);

    void on_ZCheckBox_clicked(bool checked);

    void on_RsCheckBox_clicked(bool checked);

    void on_XsCheckBox_clicked(bool checked);

    void on_ClosePushButton_pressed();

    void on_SWRLineSpinBox_valueChanged(int arg1);

    void on_RLLineSpinBox_valueChanged(int arg1);

    void on_AngleLineSpinBox_valueChanged(int arg1);

    void on_ZLineSpinBox_valueChanged(int arg1);

    void on_RsLineSpinBox_valueChanged(int arg1);

    void on_XsLineSpinBox_valueChanged(int arg1);

    void on_SWRColorPushButton_pressed();

    void on_RLColorPushButton_pressed();

    void on_AngleColorPushButton_pressed();

    void on_ZColorPushButton_2_pressed();

    void on_RsColorPushButton_pressed();

    void on_XsColorPushButton_pressed();

    void on_Gain1CheckBox_clicked(bool checked);

    void on_Gain2CheckBox_clicked(bool checked);

    void on_Gain3CheckBox_clicked(bool checked);

    void on_Gain1ColorPushButton_pressed();

    void on_Gain2ColorPushButton_pressed();

    void on_Gain3ColorPushButton_pressed();

    void on_Gain1LineSpinBox_valueChanged(int arg1);

    void on_Gain2LineSpinBox_valueChanged(int arg1);

    void on_Gain3LineSpinBox_valueChanged(int arg1);

private:
    Ui::ColorDialog *ui;

    void InitializeColors();

};

#endif // COLORDIALOG_H
