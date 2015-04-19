#include "colordialog.h"
#include "ui_colordialog.h"
#include "mainwindow.h"
#include "QColorDialog"
#include <iostream>

using namespace std;

AllLinesStruct ColorDialog::line;
AllLinesStruct line_new;
MainWindow *mdiparent;
QColorDialog *colorselect;

bool initialized = false;

ColorDialog::ColorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ColorDialog)
{
    ui->setupUi(this);
    InitializeColors();
    initialized = true;
}

ColorDialog::~ColorDialog()
{
    delete ui;
}

void ColorDialog::InitializeColors()
{
    ui->SWRCheckBox->setChecked(line.SWR.enabled);
    ui->SWRLineSpinBox->setValue(line.SWR.width);
    ui->RLCheckBox->setChecked(line.RL.enabled);
    ui->RLLineSpinBox->setValue(line.RL.width);
    ui->AngleCheckBox->setChecked(line.Angle.enabled);
    ui->AngleLineSpinBox->setValue(line.Angle.width);
    ui->ZCheckBox->setChecked(line.Z.enabled);
    ui->ZLineSpinBox->setValue(line.Z.width);
    ui->RsCheckBox->setChecked(line.Rs.enabled);
    ui->RsLineSpinBox->setValue(line.Rs.width);
    ui->XsCheckBox->setChecked(line.Xs.enabled);
    ui->XsLineSpinBox->setValue(line.Xs.width);
    ui->Gain1CheckBox->setChecked(line.Gain1.enabled);
    ui->Gain1LineSpinBox->setValue(line.Gain1.width);
    ui->Gain2CheckBox->setChecked(line.Gain2.enabled);
    ui->Gain2LineSpinBox->setValue(line.Gain2.width);
    ui->Gain3CheckBox->setChecked(line.Gain3.enabled);
    ui->Gain3LineSpinBox->setValue(line.Gain3.width);

    mdiparent = qobject_cast<MainWindow*>(this->parentWidget());
}

void ColorDialog::on_SWRCheckBox_clicked(bool checked)
{
    mdiparent->UpdateSWR(checked);
}

void ColorDialog::on_RLCheckBox_clicked(bool checked)
{
    mdiparent->UpdateRL(checked);
}

void ColorDialog::on_AngleCheckBox_clicked(bool checked)
{
    mdiparent->UpdateAngle(checked);
}

void ColorDialog::on_ZCheckBox_clicked(bool checked)
{
    mdiparent->UpdateZ(checked);
}

void ColorDialog::on_RsCheckBox_clicked(bool checked)
{
    mdiparent->UpdateRs(checked);
}

void ColorDialog::on_XsCheckBox_clicked(bool checked)
{
    mdiparent->UpdateXs(checked);
}

void ColorDialog::on_ClosePushButton_pressed()
{
    close();
}

void ColorDialog::on_SWRLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.SWR.width = arg1;
        mdiparent->UpdateSWRLineWidth();
    }
}

void ColorDialog::on_RLLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.RL.width = arg1;
        mdiparent->UpdateRLLineWidth();
    }
}

void ColorDialog::on_AngleLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Angle.width = arg1;
        mdiparent->UpdateAngleLineWidth();
    }
}

void ColorDialog::on_ZLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Z.width = arg1;
        mdiparent->UpdateZLineWidth();
    }
}

void ColorDialog::on_RsLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Rs.width = arg1;
        mdiparent->UpdateRsLineWidth();
    }
}

void ColorDialog::on_XsLineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Xs.width = arg1;
        mdiparent->UpdateXsLineWidth();
    }
}

void ColorDialog::on_SWRColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.SWR.color = colorselect->getColor(line.SWR.color);
    mdiparent->UpdateSWRLineWidth();
}

void ColorDialog::on_RLColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.RL.color = colorselect->getColor(line.RL.color);
    mdiparent->UpdateRLLineWidth();
}

void ColorDialog::on_AngleColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Angle.color = colorselect->getColor(line.Angle.color);
    mdiparent->UpdateAngleLineWidth();
}

void ColorDialog::on_ZColorPushButton_2_pressed()
{
     colorselect = new QColorDialog(this);
     line.Z.color = colorselect->getColor(line.Z.color);
     mdiparent->UpdateZLineWidth();
}

void ColorDialog::on_RsColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Rs.color = colorselect->getColor(line.Rs.color);
    mdiparent->UpdateRsLineWidth();
}

void ColorDialog::on_XsColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Xs.color = colorselect->getColor(line.Xs.color);
    mdiparent->UpdateXsLineWidth();
}

void ColorDialog::on_Gain1CheckBox_clicked(bool checked)
{
    mdiparent->UpdateGain1(checked);
}

void ColorDialog::on_Gain2CheckBox_clicked(bool checked)
{
    mdiparent->UpdateGain2(checked);
}

void ColorDialog::on_Gain3CheckBox_clicked(bool checked)
{
    mdiparent->UpdateGain2(checked);
}

void ColorDialog::on_Gain1ColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Gain1.color = colorselect->getColor(line.Gain1.color);
    mdiparent->UpdateGain1LineWidth();
}

void ColorDialog::on_Gain2ColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Gain2.color = colorselect->getColor(line.Gain2.color);
    mdiparent->UpdateGain2LineWidth();
}

void ColorDialog::on_Gain3ColorPushButton_pressed()
{
    colorselect = new QColorDialog(this);
    line.Gain3.color = colorselect->getColor(line.Gain3.color);
    mdiparent->UpdateGain3LineWidth();
}

void ColorDialog::on_Gain1LineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Gain1.width = arg1;
        mdiparent->UpdateGain1LineWidth();
    }
}

void ColorDialog::on_Gain2LineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Gain2.width = arg1;
        mdiparent->UpdateGain2LineWidth();
    }
}

void ColorDialog::on_Gain3LineSpinBox_valueChanged(int arg1)
{
    if (initialized)
    {
        line.Gain3.width = arg1;
        mdiparent->UpdateGain3LineWidth();
    }
}
