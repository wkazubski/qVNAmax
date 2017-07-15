#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#if QT_VERSION >= 0x050000
#include <QtSerialPort/QSerialPortInfo>
#else
#include <serialportinfo.h>
#endif
#include <iostream>

using namespace std;

#if QT_VERSION >= 0x050000
QT_USE_NAMESPACE

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");
#else
QT_USE_NAMESPACE_SERIALPORT
#endif

AnalyzerStruct SettingsDialog::analyzer[N_HW];
AnalyzerStruct analyzer_new[N_HW];

int i;
int n = 0;
bool ready;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->DeviceSpinBox->setMaximum(N_HW);
    for (i=0; i<N_HW; i++)
    {
        analyzer_new[i].configured = false;
        analyzer_new[i].mode = analyzer[i].mode;
        analyzer_new[i].modeN = analyzer[i].modeN;
        analyzer_new[i].port = analyzer[i].port;
        analyzer_new[i].portN = analyzer[i].portN;
        analyzer_new[i].serialport = analyzer[i].serialport;
        analyzer_new[i].serialportN = analyzer[i].serialportN;
        analyzer_new[i].mode40dB = analyzer[i].mode40dB;
        analyzer_new[i].transmitance_enabled = analyzer[i].transmitance_enabled;
        analyzer_new[i].DDSclock = analyzer[i].DDSclock;
        analyzer_new[i].fmax = analyzer[i].fmax;
        analyzer_new[i].fmin = analyzer[i].fmin;
        analyzer_new[i].transmitance_offset = analyzer[i].transmitance_offset;
        analyzer_new[i].reflectance_offset = analyzer[i].reflectance_offset;
        analyzer_new[i].detectors = analyzer[i].detectors;
        analyzer_new[i].det1_gain = analyzer[i].det1_gain;
        analyzer_new[i].det2_gain = analyzer[i].det2_gain;
        analyzer_new[i].det3_gain = analyzer[i].det3_gain;
        analyzer_new[i].det1_offset = analyzer[i].det1_offset;
        analyzer_new[i].det2_offset = analyzer[i].det2_offset;
        analyzer_new[i].det3_offset = analyzer[i].det3_offset;
    }
    n = 0;
    SetInitialValues(n);
    analyzer_new[n].configured = true;
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_buttonBox_accepted()
{
    for (i=0; i<N_HW; i++)
    {
        if (analyzer_new[i].configured)
        {
            analyzer[i].configured = analyzer_new[i].configured;
            analyzer[i].mode = analyzer_new[i].mode;
            analyzer[i].modeN = analyzer_new[i].modeN;
            analyzer[i].port = analyzer_new[i].port;
            analyzer[i].portN = analyzer_new[i].portN;
            analyzer[i].serialport = analyzer_new[i].serialport;
            analyzer[i].serialportN = analyzer_new[i].serialportN;
            analyzer[i].mode40dB = analyzer_new[i].mode40dB;
            analyzer[i].transmitance_enabled = analyzer_new[i].transmitance_enabled;
            analyzer[i].DDSclock = analyzer_new[i].DDSclock;
            analyzer[i].fmax = analyzer_new[i].fmax;
            analyzer[i].fmin = analyzer_new[i].fmin;
            analyzer[i].transmitance_offset = analyzer_new[i].transmitance_offset;
            analyzer[i].reflectance_offset = analyzer_new[i].reflectance_offset;
            analyzer[i].detectors = analyzer_new[i].detectors;
            analyzer[i].det1_gain = analyzer_new[i].det1_gain;
            analyzer[i].det2_gain = analyzer_new[i].det2_gain;
            analyzer[i].det3_gain = analyzer_new[i].det3_gain;
            analyzer[i].det1_offset = analyzer_new[i].det1_offset;
            analyzer[i].det2_offset = analyzer_new[i].det2_offset;
            analyzer[i].det3_offset = analyzer_new[i].det3_offset;
        }
    }
    MainWindow *mdiparent = qobject_cast<MainWindow*>(this->parentWidget());
    mdiparent->UpdateHWLimits();
}

void SettingsDialog::on_DDSclockDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].DDSclock = ui->DDSclockDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_MinFrequencyDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].fmin = ui->MinFrequencyDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_MaxFrequencyDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].fmax = ui->MaxFrequencyDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_DeviceSpinBox_valueChanged(int arg1)
{
    n = arg1 - 1;
    SetInitialValues(n);
}

void SettingsDialog::SetInitialValues(int n)
{
    ready = false;
    ui->TypeComboBox->setCurrentIndex(analyzer_new[n].modeN);
    ui->TransmitanceCheckBox->setChecked(analyzer_new[n].transmitance_enabled);
    ui->DRCheckBox->setChecked(analyzer_new[n].mode40dB);
    ui->PortComboBox->setCurrentIndex(analyzer_new[n].portN);
    ui->SerialComboBox->setCurrentIndex(fillPortsInfo());
    analyzer[n].serialport = ui->SerialComboBox->currentText();
    ui->DDSclockDoubleSpinBox->setValue(analyzer_new[n].DDSclock);
    ui->MaxFrequencyDoubleSpinBox->setValue(analyzer_new[n].fmax);
    ui->MinFrequencyDoubleSpinBox->setValue(analyzer_new[n].fmin);
    ui->TransmitanceOffsetDoubleSpinBox->setValue(analyzer_new[n].transmitance_offset);
    ui->ReflectanceOffsetDoubleSpinBox->setValue(analyzer_new[n].reflectance_offset);
    ui->DetectorsSpinBox->setValue(analyzer_new[n].detectors);
    ui->Det1GainDoubleSpinBox->setValue(analyzer_new[n].det1_gain);
    ui->Det2GainDoubleSpinBox->setValue(analyzer_new[n].det2_gain);
    ui->Det3GainDoubleSpinBox->setValue(analyzer_new[n].det3_gain);
    ui->Det1OffsetDoubleSpinBox->setValue(analyzer_new[n].det1_offset);
    ui->Det2OffsetDoubleSpinBox->setValue(analyzer_new[n].det2_offset);
    ui->Det3OffsetDoubleSpinBox->setValue(analyzer_new[n].det3_offset);
    UpdateWindow();
    ready = true;
}

void SettingsDialog::on_TypeComboBox_currentIndexChanged(int index)
{
    if (ready)
    {
        analyzer_new[n].modeN = index;
        analyzer_new[n].mode = ui->TypeComboBox->currentText();
        UpdateWindow();
        analyzer_new[n].configured = true;
        if (index != 0)
            ui->DetectorsSpinBox->setValue(0);
    }
}

void SettingsDialog::on_PortComboBox_currentIndexChanged(int index)
{
    if (ready)
    {
        analyzer_new[n].portN = index;
        switch(index)
        {
        case 0:
            analyzer_new[n].port = 0x278;
            break;
        case 1:
            analyzer_new[n].port = 0x378;
            break;
        case 2:
            analyzer_new[n].port = 0x3Bc;
            break;
        }
//        cout << "Port:" << analyzer_new[n].port << endl;
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_SerialComboBox_currentIndexChanged(int index)
{
    if (ready)
    {
        analyzer_new[n].serialportN = index;
        analyzer_new[n].serialport = ui->SerialComboBox->currentText();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_TransmitanceCheckBox_clicked()
{
    if (ready)
    {
        analyzer_new[n].transmitance_enabled = ui->TransmitanceCheckBox->isChecked();
        analyzer_new[n].configured = true;
        ui->TransmitanceOffsetDoubleSpinBox->setEnabled(ui->TransmitanceCheckBox->isChecked());
    }
}

void SettingsDialog::on_DRCheckBox_clicked()
{
    if (ready)
    {
        analyzer_new[n].mode40dB = ui->DRCheckBox->isChecked();
        analyzer_new[n].configured = true;
    }
}

int SettingsDialog::fillPortsInfo()
{
    int i, k;
    i = 0;
    k = 0;
    ui->SerialComboBox->clear();
#if QT_VERSION >= 0x050000
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
#else
    foreach (const SerialPortInfo &info, SerialPortInfo::availablePorts()) {
        QStringList list;
        list << info.portName() << info.description()
             << info.manufacturer() << info.systemLocation()
             << info.vendorIdentifier() << info.productIdentifier();
#endif

        ui->SerialComboBox->addItem(list.first(), list);
        if (info.portName() == analyzer_new[n].serialport)
            k=i;
        i++;
    }
    return (k);
}

void SettingsDialog::UpdateWindow()
{
    int x;
    x = ui->TypeComboBox->currentIndex();
    ui->SerialComboBox->setEnabled(x == 0);
    ui->PortComboBox->setEnabled(x != 0);
    ui->TransmitanceOffsetDoubleSpinBox->setEnabled(ui->TransmitanceCheckBox->isChecked());
}

void SettingsDialog::on_ReflectanceOffsetDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].reflectance_offset = ui->ReflectanceOffsetDoubleSpinBox->value();
        analyzer_new[n].configured = true;
//        cout << "Set: " << analyzer_new[n].reflectance_offset << endl;
    }
}

void SettingsDialog::on_TransmitanceOffsetDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].transmitance_offset = ui->TransmitanceOffsetDoubleSpinBox->value();
        analyzer_new[n].configured = true;
//        cout << "Set: " << analyzer_new[n].transmitance_offset << endl;
    }
}

void SettingsDialog::on_DetectorsSpinBox_valueChanged(int arg1)
{
    if (ready)
    {
        analyzer_new[n].detectors = arg1;
        analyzer_new[n].configured = true;
    }

    ui->Det1GainDoubleSpinBox->setEnabled(arg1 > 0);
    ui->Det1OffsetDoubleSpinBox->setEnabled(arg1 > 0);
    ui->Det2GainDoubleSpinBox->setEnabled(arg1 > 1);
    ui->Det2OffsetDoubleSpinBox->setEnabled(arg1 > 1);
    ui->Det3GainDoubleSpinBox->setEnabled(arg1 > 2);
    ui->Det3OffsetDoubleSpinBox->setEnabled(arg1 > 2);
}

void SettingsDialog::on_Det1GainDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det1_gain = ui->Det1GainDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}


void SettingsDialog::on_Det2GainDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det2_gain = ui->Det2GainDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_Det3GainDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det3_gain = ui->Det3GainDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_Det1OffsetDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det1_offset = ui->Det1OffsetDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_Det2OffsetDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det2_offset = ui->Det2OffsetDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}

void SettingsDialog::on_Det3OffsetDoubleSpinBox_editingFinished()
{
    if (ready)
    {
        analyzer_new[n].det3_offset = ui->Det3OffsetDoubleSpinBox->value();
        analyzer_new[n].configured = true;
    }
}
