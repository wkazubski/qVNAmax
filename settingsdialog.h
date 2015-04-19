#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "mainwindow.h"
#include <QDialog>


namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    static AnalyzerStruct analyzer[N_HW];

private slots:
    void on_buttonBox_accepted();

    void on_DeviceSpinBox_valueChanged(int arg1);

    void SetInitialValues(int n);

    void on_TypeComboBox_currentIndexChanged(int index);

    void on_PortComboBox_currentIndexChanged(int index);

    void on_SerialComboBox_currentIndexChanged(int index);

    void on_TransmitanceCheckBox_clicked();

    void on_DRCheckBox_clicked();

    void on_DDSclockDoubleSpinBox_editingFinished();

    void on_MaxFrequencyDoubleSpinBox_editingFinished();

    void on_MinFrequencyDoubleSpinBox_editingFinished();

    void on_ReflectanceOffsetDoubleSpinBox_editingFinished();

    void on_TransmitanceOffsetDoubleSpinBox_editingFinished();

    void on_DetectorsSpinBox_valueChanged(int arg1);

    void on_Det1GainDoubleSpinBox_editingFinished();

    void on_Det2GainDoubleSpinBox_editingFinished();

    void on_Det3GainDoubleSpinBox_editingFinished();

    void on_Det1OffsetDoubleSpinBox_editingFinished();

    void on_Det2OffsetDoubleSpinBox_editingFinished();

    void on_Det3OffsetDoubleSpinBox_editingFinished();

private:
    Ui::SettingsDialog *ui;

    int fillPortsInfo();

    void UpdateWindow();
};

#endif // SETTINGSDIALOG_H
