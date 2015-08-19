#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include "colordialog.h"
#include "commentdialog.h"
#include "vna.h"
#include "vna_serial.h"
#ifdef HAVE_AD9851
#include "vna_ad9851.h"
#endif
#ifdef HAVE_AD9951
#include "vna_ad9951.h"
#endif
#ifdef HAVE_TEST
#include "vna_test.h"
#endif
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include <QTimer>
#include <QColorGroup>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextStream>
#include <QTime>
#include <qmath.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#if QWT_VERSION >= 0x060000
#include <qwt_plot_renderer.h>
#include <qwt_picker_machine.h>
#endif
#include <qwt_plot_canvas.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_symbol.h>
#include <qwt_event_pattern.h>
#include <iostream>
#include <unistd.h>
#ifdef HAVE_RMB
#include "qwt_plot_picker2.h"
#include "qwt_picker_machine2.h"
#endif

#define VERSION "0.3.3"
#define LASTYEAR "2015"

#define SWRdefaultColor QColor::red()
#define RLdefaultColor QColor::blue()
#define ZdefaultColor QColor::darkCyan()
#define RsDefaultColor QColor::green()
#define XsDefaultColor QColor::darkGreen()
#define PhaseDefaultColor QColor::darkYellow()
#define Gain1DefaultColor QColor::lightBlue()
#define Gain2DefaultColor QColor::lightCyan()
#define Gain3DefaultColor QColor::lightRed()

#define X2TO32 4294967296
#define R2D 57.2957795131
#define MAXPOINTS 501

using namespace std;

MeasureStruct meas;
MarkerStruct marker1;
MarkerStruct marker2;
MarkerStruct markermin;
FreqMemoryStruct freqmemory[5];
//AllLinesStruct line;

QSettings settings("HamRadio", "qVNAmax");

SettingsDialog *settingsdialog;
ColorDialog *colordialog;
CommentDialog *commentdialog;
Vna *vna;
PlotData plotdata;
PDUT pdut;

QwtPlotCurve *SWRline;
QwtPlotCurve *AngleLine;
QwtPlotCurve *RLline;
QwtPlotCurve *RsLine;
QwtPlotCurve *XsLine;
QwtPlotCurve *Zline;
QwtPlotCurve *Z0Line;
QwtPlotCurve *Gain1Line;
QwtPlotCurve *Gain2Line;
QwtPlotCurve *Gain3Line;
QwtPlotMarker *Marker1;
QwtPlotMarker *Marker2;
QwtPlotMarker *MarkerMin;
QwtPlotGrid *Grid;
QTimer *MeasureTimer;
QPalette pal;

QString hwname[] = {"Hw1", "Hw2", "Hw3", "Hw4"};
quint8 VNA_magic[] = {0x2e, 0xa7, 0x9c, 0x89};
quint32 VNA_magic_number = 2308745006UL;

QString VNA_file_version = "0.1.1";

bool running = false;
bool fullrange = false;
int speed = 1000;
QString RightAxisName;
int freqcounter = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ReadSettings();
    ui->HWSpinBox->setMaximum(N_HW);
    ui->HWSpinBox->setValue(meas.hardware);
    ui->HWSpinBox->setMinimum(1);

    InitPlot();
    ui->ImpedanceRadioButton->setChecked(true);
    on_ImpedanceRadioButton_clicked();
    SetRightScale();

    running = true;
    ui->MinFrequencyDoubleSpinBox->setValue(meas.fmin);
    ui->MaxFrequencyDoubleSpinBox->setValue(meas.fmax);
    running = false;
    ui->PointsSpinBox->setValue(meas.points);
    ui->Marker1CheckBox->setChecked(marker1.enabled);
    ui->Marker1DoubleSpinBox->setValue(marker1.frequency);
    ui->Marker2CheckBox->setChecked(marker2.enabled);
    ui->Marker2DoubleSpinBox->setValue(marker2.frequency);
    ui->MinSWRCheckBox->setChecked(markermin.enabled);
    ui->RLcheckBox->setChecked(colordialog->line.RL.enabled);
    ui->SWRcheckBox->setChecked(colordialog->line.SWR.enabled);
    ui->PhaseCheckBox->setChecked(colordialog->line.Angle.enabled);
    ui->ZcheckBox->setChecked(colordialog->line.Z.enabled);
    ui->RsCheckBox->setChecked(colordialog->line.Rs.enabled);
    ui->XsCheckBox->setChecked(colordialog->line.Xs.enabled);
    ui->Gain1CheckBox->setChecked(colordialog->line.Gain1.enabled);
    ui->Gain2CheckBox->setChecked(colordialog->line.Gain2.enabled);
    ui->Gain3CheckBox->setChecked(colordialog->line.Gain3.enabled);

    RLline->setVisible(colordialog->line.RL.enabled);
    SWRline->setVisible(colordialog->line.SWR.enabled);
    AngleLine->setVisible(colordialog->line.Angle.enabled);
    Zline->setVisible(colordialog->line.Z.enabled);
    RsLine->setVisible(colordialog->line.Rs.enabled);
    XsLine->setVisible(colordialog->line.Xs.enabled);
    Gain1Line->setVisible(colordialog->line.Gain1.enabled);
    Gain2Line->setVisible(colordialog->line.Gain2.enabled);
    Gain3Line->setVisible(colordialog->line.Gain3.enabled);

    MeasureTimer = new QTimer(this);
    connect(MeasureTimer, SIGNAL(timeout()), this, SLOT(Measure()));

    QwtPlotPicker* m_picker = new QwtPlotPicker(ui->MainQwtPlot->canvas());
#ifdef HAVE_RMB
    QwtPlotPicker2* m2_picker = new QwtPlotPicker2(ui->MainQwtPlot->canvas());
#endif
    // emit the position of clicks on widget
#if QWT_VERSION >= 0x060000
    m_picker->setStateMachine(new QwtPickerClickPointMachine());
    connect(m_picker, SIGNAL(selected(const QPointF &)), this, SLOT(OnPickerPointSelected(const QPointF &)));
#ifdef HAVE_RMB
    m2_picker->setStateMachine(new QwtPicker2ClickPointMachine());
    connect(m2_picker, SIGNAL(selected(const QPointF &)), this, SLOT(OnPicker2PointSelected(const QPointF &)));
#else
    connect(m_picker, SIGNAL(selected(const QPointF &)), this, SLOT(OnPicker2PointSelected(const QPointF &)));
#endif
#else
    m_picker->setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
    connect(m_picker, SIGNAL(selected(const QwtDoublePoint &)), this, SLOT(OnPickerPointSelected(const QwtDoublePoint &)));
#endif
}

MainWindow::~MainWindow()
{
//    MainWindow::WriteSettings();
    delete ui;
}

void MainWindow::on_actionPrint_triggered()
{
    QPrinter printer(QPrinter::HighResolution);

    QString docName = ui->MainQwtPlot->title().text();
    if ( !docName.isEmpty() )
    {
        docName.replace (QRegExp (QString::fromLatin1 ("\n")), tr (" -- "));
        printer.setDocName (docName);
    }

    printer.setCreator("Vector Network Anlyzer Plot");
    printer.setOrientation(QPrinter::Landscape);

    QPrintDialog dialog(&printer);
    if ( dialog.exec() )
    {
#if QWT_VERSION >= 0x060000
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag(QwtPlotRenderer::DiscardCanvasBackground);
            renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
        }

        renderer.renderTo(ui->MainQwtPlot, printer);
#else
        QwtPlotPrintFilter filter;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
//            filter.setOptions ( QwtPlotPrintFilter::PrintAll & ~QwtPlotPrintFilter::PrintCanvasBackground );
        }

        ui->MainQwtPlot->print( printer, filter );
#endif
    }
}

void MainWindow::on_actionExit_triggered()
{
    MainWindow::close();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about ( this, tr("About qVNAmax"),
    tr("This is qVNAmax v ") +
    VERSION +
    tr(", the software to control \n"
    "the IW2HEV antenna analyzer.\n\n"
    "(C)2012-") +
    LASTYEAR +
    tr(" Wojciech Kazubski <wk@ire.pw.edu.pl\n\n"
    "Licensed under GNU GPL v3 " ));
}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt ( this, tr("About Qt"));
}

void MainWindow::on_actionSetup_triggered()
{
    if (running)
        StopMeasurement();
    settingsdialog = new SettingsDialog(this);
    settingsdialog->show();
}

void MainWindow::on_MinFrequencyDoubleSpinBox_valueChanged(double arg1)
{
    if (arg1 <= meas.fmax)
        meas.fmin = arg1;
//    cout << "Min Set:" << arg1 << "//" << meas.fmin << "/" << meas.fmax << endl;
    if (!running)
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);
        ui->MainQwtPlot->replot();
    }
}

void MainWindow::on_MinFrequencyDoubleSpinBox_editingFinished()
{
    double f = ui->MinFrequencyDoubleSpinBox->value();
    if (f > meas.fmax)
    {
        ui->MaxFrequencyDoubleSpinBox->setValue(f + 1e-6);
        meas.fmax = f + 1e-6;
        meas.fmin = f;
    }
    if (!running)
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);
        ui->MainQwtPlot->replot();
    }
    CalculateStep();
}

void MainWindow::on_MaxFrequencyDoubleSpinBox_valueChanged(double arg1)
{
    if (arg1 >= meas.fmin)
        meas.fmax = arg1;
//    cout << "Max Set:" << arg1 << "//" << meas.fmin << "/" << meas.fmax << endl;
    if ((meas.measure_mode == 10) && (running))
        StartGenerator(meas.fmax);
    if (!running)
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);
        ui->MainQwtPlot->replot();
    }
}

void MainWindow::on_MaxFrequencyDoubleSpinBox_editingFinished()
{
    double f = ui->MaxFrequencyDoubleSpinBox->value();
    if (f < meas.fmin)
    {
        ui->MinFrequencyDoubleSpinBox->setValue(f - 1e-6);
        meas.fmax = f;
        meas.fmin = f - 1e-6;
    }
    if (!running)
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);
        ui->MainQwtPlot->replot();
    }
    CalculateStep();
}

void MainWindow::on_PointsSpinBox_valueChanged(int arg1)
{
    meas.points = arg1;
    CalculateStep();
}

void MainWindow::on_QuitPushButton_clicked()
{
    MainWindow::WriteSettings();
    MainWindow::close();
}

void MainWindow::CalculateStep()
{
    meas.step = (meas.fmax - meas.fmin) / (meas.points - 1);
}

void MainWindow::ReadSettings()
{
    settings.beginGroup("Main");
    meas.fmin = settings.value("Min_Frequency", 1.00).toDouble();
    meas.fmax = settings.value("Max_Frequency", 50.00).toDouble();
    meas.points = settings.value("Points", 501).toInt();
    meas.fullscale = settings.value("Full_range", false).toBool();
    meas.hardware = settings.value("Hardware",1).toInt();
    commentdialog->ask_for_comment = settings.value("Ask_for comment",true).toBool();
    settings.endGroup();

    settings.beginGroup("Marker1");
    marker1.enabled = settings.value("Active", true).toBool();
    marker1.frequency = settings.value("Frequency", 10.00).toDouble();
    colordialog->line.Marker1.color = settings.value("Color").value<QColor>();
    colordialog->line.Marker1.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Marker1.set)
        colordialog->line.Marker1.color.setRgb(255,0,0);
    colordialog->line.Marker1.width = settings.value("Width",0).toInt();
    pal = ui->Marker1CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Marker1.color);
    ui->Marker1CheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Marker2");
    marker2.enabled = settings.value("Active", true).toBool();
    marker2.frequency = settings.value("Frequency", 20.00).toDouble();
    colordialog->line.Marker2.color = settings.value("Color").value<QColor>();
    colordialog->line.Marker2.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Marker2.set)
        colordialog->line.Marker2.color.setRgb(0,0,255);
    colordialog->line.Marker2.width = settings.value("Width",0).toInt();
    pal = ui->Marker2CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Marker2.color);
    ui->Marker2CheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Marker_Min_SWR");
    markermin.enabled = settings.value("Active", true).toBool();
    colordialog->line.MarkerMin.color = settings.value("Color").value<QColor>();
    colordialog->line.MarkerMin.set = settings.value("Set",false).toBool();
    if (!colordialog->line.MarkerMin.set)
        colordialog->line.MarkerMin.color.setRgb(0,0,0);
    colordialog->line.MarkerMin.width = settings.value("Width",0).toInt();
    pal = ui->MinSWRCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.MarkerMin.color);
    ui->MinSWRCheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_SWR");
    colordialog->line.SWR.enabled = settings.value("Active", true).toBool();
    colordialog->line.SWR.color = settings.value("Color").value<QColor>();
    colordialog->line.SWR.set = settings.value("Set",false).toBool();
    if (!colordialog->line.SWR.set)
        colordialog->line.SWR.color.setRgb(255,0,0);
    colordialog->line.SWR.width = settings.value("Width",0).toInt();
    pal = ui->SWRcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.SWR.color);
    ui->SWRcheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_RL");
    colordialog->line.RL.enabled = settings.value("Active", true).toBool();
    colordialog->line.RL.color = settings.value("Color").value<QColor>();
    colordialog->line.RL.set = settings.value("Set",false).toBool();
    if (!colordialog->line.RL.set)
        colordialog->line.RL.color.setRgb(0,0,255);
    colordialog->line.RL.width = settings.value("Width",0).toInt();
    pal = ui->RLcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.RL.color);
    ui->RLcheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Z");
    colordialog->line.Z.enabled = settings.value("Active", true).toBool();
    colordialog->line.Z.color = settings.value("Color").value<QColor>();
    colordialog->line.Z.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Z.set)
        colordialog->line.Z.color.setRgb(0,127,127);
    colordialog->line.Z.width = settings.value("Width",0).toInt();
    pal = ui->ZcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Z.color);
    ui->ZcheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Rs");
    colordialog->line.Rs.enabled = settings.value("Active", true).toBool();
    colordialog->line.Rs.color = settings.value("Color").value<QColor>();
    colordialog->line.Rs.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Rs.set)
        colordialog->line.Rs.color.setRgb(0,255,0);
    colordialog->line.Rs.width = settings.value("Width",0).toInt();
    pal = ui->RsCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Rs.color);
    ui->RsCheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Xs");
    colordialog->line.Xs.enabled = settings.value("Active", true).toBool();
    colordialog->line.Xs.color = settings.value("Color").value<QColor>();
    colordialog->line.Xs.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Xs.set)
        colordialog->line.Xs.color.setRgb(0,127,0);
    colordialog->line.Xs.width = settings.value("Width",0).toInt();
    pal = ui->XsCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Xs.color);
    ui->XsCheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Phase");
    colordialog->line.Angle.enabled = settings.value("Active", true).toBool();
    colordialog->line.Angle.color = settings.value("Color").value<QColor>();
    colordialog->line.Angle.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Angle.set)
        colordialog->line.Angle.color.setRgb(127,127,0);
    colordialog->line.Angle.width = settings.value("Width",0).toInt();
    pal = ui->PhaseCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Angle.color);
    ui->PhaseCheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Gain1");
    colordialog->line.Gain1.enabled = settings.value("Active", true).toBool();
    colordialog->line.Gain1.color = settings.value("Color").value<QColor>();
    colordialog->line.Gain1.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Gain1.set)
        colordialog->line.Gain1.color.setRgb(63, 63, 255);
    colordialog->line.Gain1.width = settings.value("Width",0).toInt();
    pal = ui->Gain1CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain1.color);
    ui->Gain1CheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Gain2");
    colordialog->line.Gain2.enabled = settings.value("Active", true).toBool();
    colordialog->line.Gain2.color = settings.value("Color").value<QColor>();
    colordialog->line.Gain2.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Gain2.set)
        colordialog->line.Gain2.color.setRgb(255, 63, 63);
    colordialog->line.Gain2.width = settings.value("Width",0).toInt();
    pal = ui->Gain2CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain2.color);
    ui->Gain2CheckBox->setPalette(pal);
    settings.endGroup();

    settings.beginGroup("Line_Gain3");
    colordialog->line.Gain3.enabled = settings.value("Active", true).toBool();
    colordialog->line.Gain3.color = settings.value("Color").value<QColor>();
    colordialog->line.Gain3.set = settings.value("Set",false).toBool();
    if (!colordialog->line.Gain3.set)
        colordialog->line.Gain3.color.setRgb(63, 255, 63);
    colordialog->line.Gain3.width = settings.value("Width",0).toInt();
    pal = ui->Gain3CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain3.color);
    ui->Gain3CheckBox->setPalette(pal);
    settings.endGroup();

    int i;
    for (i=0;i<N_HW;i++)
    {
        settings.beginGroup(hwname[i]);
        settingsdialog->analyzer[i].DDSclock = settings.value("DDS_Clock", 180.0).toDouble();
        settingsdialog->analyzer[i].fmax = settings.value("Max_Frequency", 70.0).toDouble();
        settingsdialog->analyzer[i].fmin = settings.value("Min_Frequency", 0.10).toDouble();
        settingsdialog->analyzer[i].transmitance_enabled = settings.value("Transmitance_measurement", false).toBool();
        settingsdialog->analyzer[i].mode40dB = settings.value("40dB_Mode", true).toBool();
        settingsdialog->analyzer[i].mode = settings.value("Device_Mode", "").toString();
        settingsdialog->analyzer[i].modeN = settings.value("Device_Mode_Index", 0).toInt();
        settingsdialog->analyzer[i].port = settings.value("IO_ADdress", 0x278).toInt();
        settingsdialog->analyzer[i].portN = settings.value("IO_ADdress_Index", 0).toInt();
        settingsdialog->analyzer[i].serialportN = settings.value("Serial_Port_Index", 0).toInt();
        settingsdialog->analyzer[i].serialport = settings.value("Serial_Port", "ttyS0").toString();
        settingsdialog->analyzer[i].configured = settings.value("Configured", false).toBool();
        settingsdialog->analyzer[i].transmitance_offset = settings.value("Transmitance_offset", -15.0).toDouble();
        settingsdialog->analyzer[i].reflectance_offset = settings.value("Reflectance_offset", 0.0).toDouble();
        settingsdialog->analyzer[i].detectors = settings.value("Number_of_detectors", 0).toInt();
        settingsdialog->analyzer[i].det1_gain = settings.value("Detector_1_gain", 0.0586).toDouble();
        settingsdialog->analyzer[i].det2_gain = settings.value("Detector_2_gain", 0.0586).toDouble();
        settingsdialog->analyzer[i].det3_gain = settings.value("Detector_3_gain", 0.0586).toDouble();
        settingsdialog->analyzer[i].det1_offset = settings.value("Detector_1_offset", 0.0).toDouble();
        settingsdialog->analyzer[i].det2_offset = settings.value("Detector_2_offset", 0.0).toDouble();
        settingsdialog->analyzer[i].det3_offset = settings.value("Detector_3_offset", 0.0).toDouble();
        settings.endGroup();
    }
}

void MainWindow::WriteSettings()
{
    settings.beginGroup("Main");
    settings.setValue("Min_Frequency", meas.fmin);
    settings.setValue("Max_Frequency", meas.fmax);
    settings.setValue("Points", meas.points);
    settings.setValue("Full_range", meas.fullscale);
    settings.setValue("Hardware", meas.hardware);
    settings.setValue("Ask_for_comment", commentdialog->ask_for_comment);
    settings.endGroup();

    settings.beginGroup("Marker1");
    settings.setValue("Active", marker1.enabled);
    settings.setValue("Frequency", marker1.frequency);
    settings.setValue("Set", colordialog->line.Marker1.set);
    if (colordialog->line.Marker1.set)
    {
        settings.setValue("Color", colordialog->line.Marker1.color);
        settings.setValue("Width", colordialog->line.Marker1.width);
    }
    settings.endGroup();

    settings.beginGroup("Marker2");
    settings.setValue("Active", marker2.enabled);
    settings.setValue("Frequency", marker2.frequency);
    settings.setValue("Set", colordialog->line.Marker2.set);
    if (colordialog->line.Marker2.set)
    {
        settings.setValue("Color", colordialog->line.Marker2.color);
        settings.setValue("Width", colordialog->line.Marker1.width);
    }
    settings.endGroup();

    settings.beginGroup("Marker_Min_SWR");
    settings.setValue("Active", markermin.enabled);
    settings.setValue("Set", colordialog->line.MarkerMin.set);
    if (colordialog->line.MarkerMin.set)
    {
        settings.setValue("Color", colordialog->line.MarkerMin.color);
        settings.setValue("Width", colordialog->line.MarkerMin.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_SWR");
    settings.setValue("Active", colordialog->line.SWR.enabled);
    settings.setValue("Set", colordialog->line.SWR.set);
    if (colordialog->line.SWR.set)
    {
        settings.setValue("Color", colordialog->line.SWR.color);
        settings.setValue("Width", colordialog->line.SWR.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_RL");
    settings.setValue("Active", colordialog->line.RL.enabled);
    settings.setValue("Set", colordialog->line.RL.set);
    if (colordialog->line.RL.set)
    {
        settings.setValue("Color", colordialog->line.RL.color);
        settings.setValue("Width", colordialog->line.RL.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Z");
    settings.setValue("Active", colordialog->line.Z.enabled);
    settings.setValue("Set", colordialog->line.Z.set);
    if (colordialog->line.Z.set)
    {
        settings.setValue("Color", colordialog->line.Z.color);
        settings.setValue("Width", colordialog->line.Z.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Rs");
    settings.setValue("Active", colordialog->line.Rs.enabled);
    settings.setValue("Set", colordialog->line.Rs.set);
    if (colordialog->line.Rs.set)
    {
        settings.setValue("Color", colordialog->line.Rs.color);
        settings.setValue("Width", colordialog->line.Rs.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Xs");
    settings.setValue("Active", colordialog->line.Xs.enabled);
    settings.setValue("Set", colordialog->line.Xs.set);
    if (colordialog->line.Xs.set)
    {
        settings.setValue("Color", colordialog->line.Xs.color);
        settings.setValue("Width", colordialog->line.Xs.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Angle");
    settings.setValue("Active", colordialog->line.Angle.enabled);
    settings.setValue("Set", colordialog->line.Angle.set);
    if (colordialog->line.Angle.set)
    {
        settings.setValue("Color", colordialog->line.Angle.color);
        settings.setValue("Width", colordialog->line.Angle.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Gain1");
    settings.setValue("Active", colordialog->line.Gain1.enabled);
    settings.setValue("Set", colordialog->line.Gain1.set);
    if (colordialog->line.Gain1.set)
    {
        settings.setValue("Color", colordialog->line.Gain1.color);
        settings.setValue("Width", colordialog->line.Gain1.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Gain2");
    settings.setValue("Active", colordialog->line.Gain2.enabled);
    settings.setValue("Set", colordialog->line.Gain2.set);
    if (colordialog->line.Gain2.set)
    {
        settings.setValue("Color", colordialog->line.Gain2.color);
        settings.setValue("Width", colordialog->line.Gain2.width);
    }
    settings.endGroup();

    settings.beginGroup("Line_Gain3");
    settings.setValue("Active", colordialog->line.Gain3.enabled);
    settings.setValue("Set", colordialog->line.Gain3.set);
    if (colordialog->line.Gain3.set)
    {
        settings.setValue("Color", colordialog->line.Gain3.color);
        settings.setValue("Width", colordialog->line.Gain3.width);
    }
    settings.endGroup();

    int i;
    for (i=0;i<N_HW;i++)
    {
        settings.beginGroup(hwname[i]);
        settings.setValue("Configured", settingsdialog->analyzer[i].configured);
        if (settingsdialog->analyzer[i].configured)
        {
            settings.setValue("DDS_Clock", settingsdialog->analyzer[i].DDSclock);
            settings.setValue("Max_Frequency", settingsdialog->analyzer[i].fmax);
            settings.setValue("Min_Frequency", settingsdialog->analyzer[i].fmin);
            settings.setValue("Device_Mode_Index", settingsdialog->analyzer[i].modeN);
            settings.setValue("Device_Mode", settingsdialog->analyzer[i].mode);
            settings.setValue("IO_Address_Index", settingsdialog->analyzer[i].portN);
            settings.setValue("IO_Address", settingsdialog->analyzer[i].port);
            settings.setValue("Serial_Port_Index", settingsdialog->analyzer[i].serialportN);
            settings.setValue("Serial_Port", settingsdialog->analyzer[i].serialport);
            settings.setValue("40dB_Mode", settingsdialog->analyzer[i].mode40dB);
            settings.setValue("Transmitance_measurement", settingsdialog->analyzer[i].transmitance_enabled);
            settings.setValue("Transmitance_offset", settingsdialog->analyzer[i].transmitance_offset);
            settings.setValue("Reflectance_offset", settingsdialog->analyzer[i].reflectance_offset);
            settings.setValue("Number_of_detectors", settingsdialog->analyzer[i].detectors);
            settings.setValue("Detector_1_gain", settingsdialog->analyzer[i].det1_gain);
            settings.setValue("Detector_2_gain", settingsdialog->analyzer[i].det2_gain);
            settings.setValue("Detector_3_gain", settingsdialog->analyzer[i].det3_gain);
            settings.setValue("Detector_1_offset", settingsdialog->analyzer[i].det1_offset);
            settings.setValue("Detector_2_offset", settingsdialog->analyzer[i].det2_offset);
            settings.setValue("Detector_3_offset", settingsdialog->analyzer[i].det3_offset);
        }
        settings.endGroup();
    }
}

void MainWindow::InitPlot()
{
    ui->MainQwtPlot->setCanvasBackground(QColor(Qt::white));
#if QWT_VERSION < 0x060100
    ui->MainQwtPlot->setCanvasLineWidth(0);
#endif
    ui->MainQwtPlot->enableAxis(QwtPlot::yRight, true);
    SetLeftScale();
    SetRightScale();
    ui->MainQwtPlot->setAxisTitle(QwtPlot::xBottom,tr("Frequency [MHz]"));
    ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);

    SWRline = new QwtPlotCurve("SWR");
    SWRline->setRenderHint(QwtPlotItem::RenderAntialiased);
    SWRline->setPen(QPen(colordialog->line.SWR.color, colordialog->line.SWR.width));
#if QWT_VERSION >= 0x060000
    SWRline->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.SWR.color), QColor(colordialog->line.SWR.color), QSize(4,4)));
#else
    SWRline->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.SWR.color), QColor(colordialog->line.SWR.color), QSize(4,4)));
#endif
    SWRline->attach(ui->MainQwtPlot);

    RLline = new QwtPlotCurve("RL");
    RLline->setRenderHint(QwtPlotItem::RenderAntialiased);
    RLline->setPen(QPen(colordialog->line.RL.color, colordialog->line.RL.width));
#if QWT_VERSION >= 0x060000
    RLline->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.RL.color), QColor(colordialog->line.RL.color), QSize(4,4)));
#else
    RLline->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.RL.color), QColor(colordialog->line.RL.color), QSize(4,4)));
#endif
    RLline->setYAxis(QwtPlot::yRight);
    RLline->attach(ui->MainQwtPlot);

    Zline = new QwtPlotCurve("Z");
    Zline->setRenderHint(QwtPlotItem::RenderAntialiased);
    Zline->setPen(QPen(colordialog->line.Z.color, colordialog->line.Z.width));
#if QWT_VERSION >= 0x060000
    Zline->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Z.color), QColor(colordialog->line.Z.color), QSize(4,4)));
#else
    Zline->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Z.color), QColor(colordialog->line.Z.color), QSize(4,4)));
#endif
    Zline->setYAxis(QwtPlot::yRight);
    Zline->attach(ui->MainQwtPlot);

    RsLine = new QwtPlotCurve("Z");
    RsLine->setRenderHint(QwtPlotItem::RenderAntialiased);
    RsLine->setPen(QPen(colordialog->line.Rs.color, colordialog->line.Rs.width));
#if QWT_VERSION >= 0x060000
    RsLine->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Rs.color), QColor(colordialog->line.Rs.color), QSize(4,4)));
#else
    RsLine->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Rs.color), QColor(colordialog->line.Rs.color), QSize(4,4)));
#endif
    RsLine->setYAxis(QwtPlot::yRight);
    RsLine->attach(ui->MainQwtPlot);

    XsLine = new QwtPlotCurve("Xs");
    XsLine->setRenderHint(QwtPlotItem::RenderAntialiased);
    XsLine->setPen(QPen(colordialog->line.Xs.color, colordialog->line.Xs.width));
#if QWT_VERSION >= 0x060000
    XsLine->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Xs.color), QColor(colordialog->line.Xs.color), QSize(4,4)));
#else
    XsLine->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Xs.color), QColor(colordialog->line.Xs.color), QSize(4,4)));
#endif
    XsLine->setYAxis(QwtPlot::yRight);
    XsLine->attach(ui->MainQwtPlot);

    Gain1Line = new QwtPlotCurve("Gain1");
    Gain1Line->setRenderHint(QwtPlotItem::RenderAntialiased);
    Gain1Line->setPen(QPen(colordialog->line.Gain1.color, colordialog->line.Gain1.width));
#if QWT_VERSION >= 0x060000
    Gain1Line->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain1.color), QColor(colordialog->line.Gain1.color), QSize(4,4)));
#else
    Gain1Line->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain1.color), QColor(colordialog->line.Gain1.color), QSize(4,4)));
#endif
    Gain1Line->setYAxis(QwtPlot::yRight);
    Gain1Line->attach(ui->MainQwtPlot);

    Gain2Line = new QwtPlotCurve("Gain2");
    Gain2Line->setRenderHint(QwtPlotItem::RenderAntialiased);
    Gain2Line->setPen(QPen(colordialog->line.Gain2.color, colordialog->line.Gain2.width));
#if QWT_VERSION >= 0x060000
    Gain2Line->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain2.color), QColor(colordialog->line.Gain2.color), QSize(4,4)));
#else
    Gain2Line->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain2.color), QColor(colordialog->line.Gain2.color), QSize(4,4)));
#endif
    Gain2Line->setYAxis(QwtPlot::yRight);
    Gain2Line->attach(ui->MainQwtPlot);

    Gain3Line = new QwtPlotCurve("Gain3");
    Gain3Line->setRenderHint(QwtPlotItem::RenderAntialiased);
    Gain3Line->setPen(QPen(colordialog->line.Gain3.color, colordialog->line.Gain3.width));
#if QWT_VERSION >= 0x060000
    Gain3Line->setSymbol(new QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain3.color), QColor(colordialog->line.Gain3.color), QSize(4,4)));
#else
    Gain3Line->setSymbol(QwtSymbol(QwtSymbol::NoSymbol,
        QColor(colordialog->line.Gain3.color), QColor(colordialog->line.Gain3.color), QSize(4,4)));
#endif
    Gain3Line->setYAxis(QwtPlot::yRight);
    Gain3Line->attach(ui->MainQwtPlot);

    AngleLine = new QwtPlotCurve("Angle");
    AngleLine->setRenderHint(QwtPlotItem::RenderAntialiased);
    AngleLine->setPen(QPen(colordialog->line.Angle.color, colordialog->line.Angle.width));
    AngleLine->setYAxis(QwtPlot::yLeft);
    AngleLine->attach(ui->MainQwtPlot);

    QPen marker1pen(colordialog->line.Marker1.color,0,Qt::DashLine);

    Marker1 = new QwtPlotMarker();
    Marker1->setLineStyle(QwtPlotMarker::VLine);
    Marker1->setLinePen(marker1pen);
    Marker1->setValue(marker1.frequency, 0.0);
    Marker1->setVisible(marker1.enabled);
    Marker1->attach(ui->MainQwtPlot);

    QPen marker2pen(colordialog->line.Marker2.color,0,Qt::DashLine);

    Marker2 = new QwtPlotMarker();
    Marker2->setLineStyle(QwtPlotMarker::VLine);
    Marker2->setLinePen(marker2pen);
    Marker2->setValue(marker2.frequency, 0.0);
    Marker2->setVisible(marker2.enabled);
    Marker2->attach(ui->MainQwtPlot);

    QPen markerminpen(colordialog->line.MarkerMin.color,0,Qt::DashLine);

    MarkerMin = new QwtPlotMarker();
    MarkerMin->setLineStyle(QwtPlotMarker::VLine);
    MarkerMin->setLinePen(markerminpen);
    MarkerMin->setValue(markermin.frequency, 0.0);
    MarkerMin->setVisible(markermin.enabled);
    MarkerMin->attach(ui->MainQwtPlot);

    Grid = new QwtPlotGrid;
#if QWT_VERSION >= 0x060100
    Grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
#else
    Grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
#endif
    Grid->attach(ui->MainQwtPlot);

    ui->MainQwtPlot->show();
}

void MainWindow::SetLeftScale()
{
    if (colordialog->line.SWR.enabled || !colordialog->line.Angle.enabled)
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::yLeft,1.0,10.0,1.0);
        ui->MainQwtPlot->setAxisTitle(QwtPlot::yLeft,tr("SWR"));
        meas.angle_scale = false;
    }
    else
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::yLeft,0.0,180.0, 15.0);
        ui->MainQwtPlot->setAxisTitle(QwtPlot::yLeft,tr("Angle"));
        meas.angle_scale = true;
    }
}

void MainWindow::SetRightScale()
{
    if (colordialog->line.RL.enabled || colordialog->line.Gain1.enabled || colordialog->line.Gain2.enabled || colordialog->line.Gain3.enabled ||
            !(colordialog->line.Z.enabled || colordialog->line.Rs.enabled || colordialog->line.Xs.enabled))
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::yRight,-40.0, 0.0);
        ui->MainQwtPlot->setAxisTitle(QwtPlot::yRight, RightAxisName);
        meas.r_scale = false;
    }
    else
    {
        ui->MainQwtPlot->setAxisScale(QwtPlot::yRight, 0.0, 400.0);
        ui->MainQwtPlot->setAxisTitle(QwtPlot::yRight, tr("Z, Rs, Xs"));
        meas.r_scale = true;
    }
}

void MainWindow::on_Marker1CheckBox_clicked()
{
    bool state;
    state = ui->Marker1CheckBox->isChecked();
    ui->Marker1DoubleSpinBox->setEnabled(state);
    marker1.enabled = state;
    Marker1->setVisible(state);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_Marker2CheckBox_clicked()
{
    bool state;
    state = ui->Marker2CheckBox->isChecked();
    ui->Marker2DoubleSpinBox->setEnabled(state);
    marker2.enabled = state;
    Marker2->setVisible(state);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_MinSWRCheckBox_clicked()
{
    bool state;
    state = ui->MinSWRCheckBox->isChecked();
    markermin.enabled = state;
    MarkerMin->setVisible(state);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_Marker1DoubleSpinBox_valueChanged(double arg1)
{
    marker1.frequency = arg1;
    Marker1->setValue(marker1.frequency, 0.0);
    if (!running)
    {
        getMarker1Data();
        DrawPlots(meas.points);
    }
}

void MainWindow::on_Marker2DoubleSpinBox_valueChanged(double arg1)
{
    marker2.frequency = arg1;
    Marker2->setValue(marker2.frequency, 0.0);
    if (!running)
    {
        getMarker2Data();
        DrawPlots(meas.points);
    }
}

void MainWindow::on_StartPushButton_clicked()
{
    if (settingsdialog->analyzer[meas.hardware - 1].configured)
    {
        if (!running)
            StartMeasurement();
        else
            StopMeasurement();
    }
    else
        on_actionSetup_triggered();
}

void MainWindow::StartMeasurement(void)
{
    int n;
    int result = 0;
    QString portname;
    n = meas.hardware - 1;
    switch (settingsdialog->analyzer[n].modeN)
    {
    case 0:
        vna = new VnaSerial();
        portname = settingsdialog->analyzer[n].serialport;
        break;
#ifdef HAVE_AD9851
    case 1:
        vna = new VnaAD9851();
        portname = "0x" + QString::number(settingsdialog->analyzer[n].port, 16);
        break;
#endif
#ifdef HAVE_AD9951
    case 2:
        vna = new VnaAD9951();
        portname = "0x" + QString::number(settingsdialog->analyzer[n].port, 16);
        break;
#endif
#ifdef HAVE_TEST
    case 3:
        vna = new VnaTest();
        portname = "Test";
        break;
#endif
    default:
        result = -3;
    }
    if (result ==0)
        result = vna->initVna(settingsdialog->analyzer[n].port, settingsdialog->analyzer[n].serialport);
    switch (result)
    {
    case 0:
        ui->statusBar->showMessage(tr("Connected to %1").arg(portname));
        break;
    case -1:
        QMessageBox::critical(this, tr("Error"),
                              tr("Can't configure port: %1,\n"
                                 "error code: %2")
                              .arg(portname).arg(vna->getError()));
        ui->statusBar->showMessage(tr("Configure error"));
        break;
    case -2:
        QMessageBox::critical(this, tr("Error"),
                              tr("Can't open port: %1,\n"
                                 "error code: %2")
                              .arg(portname).arg(vna->getError()));
        ui->statusBar->showMessage(tr("Open error"));
        break;
    case -3:
        QMessageBox::critical(this, tr("Error"),
                              tr("Driver not available: %1")
                              .arg(portname));
        ui->statusBar->showMessage(tr("Not available"));
        break;
    }
    if (result == 0)
    {
        running = true;
        SetHWLimits(true);
        ui->StartPushButton->setText(tr("STOP"));
        ui->actionSetup->setEnabled(false);
        ui->HWSpinBox->setEnabled(false);
        ui->FunctionButtonGroup->blockSignals(true);
        meas.display_mode = meas.measure_mode;

        if (meas.measure_mode == 10)
        {
            StartGenerator(meas.fmax);
            ui->statusBar->showMessage(tr("Generator on"));
        }
        else
        {
            SetRightScale();
            CleanGainData();
            UpdateGainCheckboxes(meas.measure_mode);
            MeasureTimer->start(speed);
            ui->statusBar->showMessage(tr("Running"));
        }
    }
}

void MainWindow::StopMeasurement(void)
{
    if (running)
    {
        if (meas.measure_mode  == 10)
        {
            StartGenerator(0.0);
            ui->statusBar->showMessage(tr("Generator off"));
        }
        else
        {
            MeasureTimer->stop();
            ui->statusBar->showMessage(tr("Stopped"));
        }
        vna->closePort();
    }
    running = false;
    ui->StartPushButton->setText(tr("RUN"));
    ui->actionSetup->setEnabled(true);
    ui->HWSpinBox->setEnabled(true);
    ui->FunctionButtonGroup->blockSignals(false);
    meas.time = QDateTime::currentDateTime();
    SetHWLimits(false);
}

void MainWindow::Measure(void)
{
    int n;
    double k[6];

    n = meas.hardware - 1;
    k[0] = settingsdialog->analyzer[n].det1_gain;
    k[1] = settingsdialog->analyzer[n].det1_offset;
    k[2] = settingsdialog->analyzer[n].det2_gain;
    k[3] = settingsdialog->analyzer[n].det2_offset;
    k[4] = settingsdialog->analyzer[n].det3_gain;
    k[5] = settingsdialog->analyzer[n].det3_offset;

    if (ui->FullRangeCheckBox->isChecked())
    {
        Measure2(settingsdialog->analyzer[n].fmin, settingsdialog->analyzer[n].fmax, meas.points, settingsdialog->analyzer[n].DDSclock,
                 settingsdialog->analyzer[n].transmitance_offset,settingsdialog->analyzer[n].reflectance_offset,settingsdialog->analyzer[n].mode40dB, meas.measure_mode, k);
    }
    else
    {
        Measure2(meas.fmin, meas.fmax, meas.points, settingsdialog->analyzer[n].DDSclock, settingsdialog->analyzer[n].transmitance_offset,
                 settingsdialog->analyzer[n].reflectance_offset,settingsdialog->analyzer[n].mode40dB, meas.measure_mode, k);
    }
}

void MainWindow::Measure2(double fmin, double fmax, int points, double clock, double gain_offset, double refl_offset, bool mode40dB, int mode, double *k)
{
    unsigned int n, step;
    double fstep, f;
    double rl_mul, offset;
    int i;
    int return_loss, angle;
    int gain1, gain2, gain3;
    fstep = (fmax - fmin) / (points - 1);
    f = fmin;
    n = (unsigned int) (f / clock * X2TO32);
    step = (unsigned int) (fstep / clock * X2TO32);
    if (mode40dB)
        rl_mul = -1.0;
    else
        rl_mul = 1.0;
    if (mode == 1)
        offset = gain_offset + refl_offset;
    else
        offset = refl_offset;
//    cout << "Mode:"  << mode << " start: " << n << " step: " << step << " points: " << points << endl;
    vna->initMeasure(n, step, points, mode);
    for (i=0;i<points;i++)
    {
        plotdata.freq[i] = f;
        n = (unsigned int) (f / clock * X2TO32);
        vna->setFrequency(n);
        vna->getData(mode, &return_loss, &angle, &gain1, &gain2, &gain3);
        dut_calc2(return_loss, angle, offset, rl_mul, &pdut);
        plotdata.return_loss[i] = pdut.return_loss;
        plotdata.angle[i] = pdut.angle;
        plotdata.angle_scaled[i] = pdut.angle * 0.05 + 1;
        plotdata.swr[i] = pdut.swr;
        plotdata.r_imp[i] = pdut.r_imp;
        plotdata.r_imp_scaled[i] = pdut.r_imp * 0.1 - 40;
        plotdata.x_imp[i] = pdut.x_imp;
        plotdata.x_imp_scaled[i] = pdut.x_imp * 0.1 - 40;
        plotdata.z_imp[i] = pdut.z_imp;
        plotdata.z_imp_scaled[i] = pdut.z_imp * 0.1 - 40;
        if (mode >= 2)
            plotdata.gain1[i] = gain1 *  + k[0] + k[1];
        else
            plotdata.gain1[i] = 0;
        if (mode >= 3)
            plotdata.gain2[i] = gain2 * k[2] + k[3];
        else
            plotdata.gain2[i] = 0;
        if (mode == 4)
            plotdata.gain3[i] = gain3 * k[4] + k[5];
        else
            plotdata.gain3[i] = 0;
        f += fstep;
    }
    plotdata.points = points;
    getMarkerData();
    ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,fmin,fmax);
    DrawPlots(points);
}

void MainWindow::StartGenerator(double freq)
{
    unsigned int n;
    double clock;
    int return_loss, angle, a, b, c;
    clock = settingsdialog->analyzer[meas.hardware - 1].DDSclock;
    n = (unsigned int) (freq / clock * X2TO32);
//    vna->initMeasure(n, 0, 0, 1);
    vna->initMeasure(n, 0, 1, 1);
    vna->getData(0, &return_loss, &angle, &a, &b, &c);
//    cout << "Generator set to " << n<< endl;
}

void MainWindow::DrawPlots(int points)
{
#if QWT_VERSION >= 0x060000
    SWRline->setSamples(plotdata.freq, plotdata.swr, points);
    RLline->setSamples(plotdata.freq, plotdata.return_loss, points);
    Gain1Line->setSamples(plotdata.freq, plotdata.gain1, points);
    Gain2Line->setSamples(plotdata.freq, plotdata.gain2, points);
    Gain3Line->setSamples(plotdata.freq, plotdata.gain3, points);
    if (meas.angle_scale)
        AngleLine->setSamples(plotdata.freq, plotdata.angle, points);
    else
        AngleLine->setSamples(plotdata.freq, plotdata.angle_scaled, points);
    if (meas.r_scale)
    {
        Zline->setSamples(plotdata.freq, plotdata.z_imp, points);
        RsLine->setSamples(plotdata.freq, plotdata.r_imp, points);
        XsLine->setSamples(plotdata.freq, plotdata.x_imp, points);
    }
    else
    {
        Zline->setSamples(plotdata.freq, plotdata.z_imp_scaled, points);
        RsLine->setSamples(plotdata.freq, plotdata.r_imp_scaled, points);
        XsLine->setSamples(plotdata.freq, plotdata.x_imp_scaled, points);
    }
#else
    SWRline->setData(plotdata.freq, plotdata.swr, points);
    RLline->setData(plotdata.freq, plotdata.return_loss, points);
    Gain1Line->setData(plotdata.freq, plotdata.gain1, points);
    Gain2Line->setData(plotdata.freq, plotdata.gain2, points);
    Gain3Line->setData(plotdata.freq, plotdata.gain3, points);
    if (meas.angle_scale)
        AngleLine->setData(plotdata.freq, plotdata.angle, points);
    else
        AngleLine->setData(plotdata.freq, plotdata.angle_scaled, points);
    if (meas.r_scale)
    {
        Zline->setData(plotdata.freq, plotdata.z_imp, points);
        RsLine->setData(plotdata.freq, plotdata.r_imp, points);
        XsLine->setData(plotdata.freq, plotdata.x_imp, points);
    }
    else
    {
        Zline->setData(plotdata.freq, plotdata.z_imp_scaled, points);
        RsLine->setData(plotdata.freq, plotdata.r_imp_scaled, points);
        XsLine->setData(plotdata.freq, plotdata.x_imp_scaled, points);
    }
#endif

    ui->MainQwtPlot->replot();

}

/*
    The following procedure was taken from gVNA project, version 0.1.2,
    file vna_dutmng.c by Davide Tosatti IW3HEV and J. C. Remis
*/
bool MainWindow::dut_calc2(int return_loss, int angle, double rl_offset, double rl_cal, PDUT *pdut)
{
/* return_loss(i) contains the ad8302 return loss from the ADC
    0.0586 is the resolution 60/1024
    angle(i) contains the ad8302 angle from the ADC
    0.175 is the resolution 180/1024
    57.324 is the conversion from radian to degree
    Mag is the reflection coefficient
*/
    double mag, f, g, rr, ss;

//     g_return_val_if_fail (pdut != NULL, FAILED);

    pdut->return_loss = (return_loss * 0.0586 - 30.0) * rl_cal + rl_offset;
    pdut->angle = angle * 0.175;
    mag = pow(10.0, pdut->return_loss / 20);
    mag = 1.0 / mag;
    f = cos(pdut->angle / R2D);
    g = sin(pdut->angle / R2D);
    rr = f * mag;
    ss = g * mag;
    /* ----- xs_calc ---- */
    pdut->x_imp = fabs(((2.0 * ss) / (((1.0 - rr)*(1.0 - rr)) + (ss*ss))) * 50.0)+0.01;
    /* ----- rs_calc ---- */
    pdut->r_imp = fabs(((1.0 - (rr*rr) - (ss*ss)) / (((1.0 - rr)*(1.0 - rr)) + (ss*ss))) * 50.0);
    /* ---- swr ---- */
    pdut->swr = fabs((1.0 + mag) / (1.001 - mag));

    /* ---- Z calc ---- */
    pdut->z_imp = sqrt((pdut->r_imp * pdut->r_imp) + (pdut->x_imp * pdut->x_imp));
//    pdut->return_loss *= (-1);
    return (true);
}

void MainWindow::getMarkerData()
{
    if (marker1.enabled)
        getMarker1Data();
    if (marker2.enabled)
        getMarker2Data();
    if (markermin.enabled)
        getMarkerMinData();
}

void MainWindow::getMarker1Data()
{
    int i;
    double k;
    double xs;
    for (i=1;i<plotdata.points;i++)
        if ((marker1.frequency >= plotdata.freq[i-1]) && (marker1.frequency <= plotdata.freq[i]))
        {
            k = (marker1.frequency - plotdata.freq[i-1]) / (plotdata.freq[i] - plotdata.freq[i-1]);
            xs = k * plotdata.x_imp[i] + (1-k) * plotdata.x_imp[i-1];
            ui->M1RLdoubleSpinBox->setValue(k * plotdata.return_loss[i] + (1-k) * plotdata.return_loss[i-1]);
            ui->M1SWRdoubleSpinBox->setValue(k * plotdata.swr[i] + (1-k) * plotdata.swr[i-1]);
            ui->M1PhaseDoubleSpinBox->setValue(k * plotdata.angle[i] + (1-k) * plotdata.angle[i-1]);
            ui->M1ZdoubleSpinBox->setValue(k * plotdata.z_imp[i] + (1-k) * plotdata.z_imp[i-1]);
            ui->M1RsDoubleSpinBox->setValue(k * plotdata.r_imp[i] + (1-k) * plotdata.r_imp[i-1]);
            ui->M1XsDoubleSpinBox->setValue(xs);
            ui->M1CsDoubleSpinBox->setValue(1e6 / (2 * M_PI * marker1.frequency * xs));
            ui->M1LsDoubleSpinBox->setValue(xs / (2 * M_PI * marker1.frequency));
            ui->M1G1DoubleSpinBox->setValue(k * plotdata.gain1[i] + (1-k) * plotdata.gain1[i-1]);
            ui->M1G2DoubleSpinBox->setValue(k * plotdata.gain2[i] + (1-k) * plotdata.gain2[i-1]);
            ui->M1G3DoubleSpinBox->setValue(k * plotdata.gain3[i] + (1-k) * plotdata.gain3[i-1]);
        }
}

void MainWindow::getMarker2Data()
{
    int i;
    double k;
    double xs;
    for (i=1;i<plotdata.points;i++)
        if ((marker2.frequency >= plotdata.freq[i-1]) && (marker2.frequency <= plotdata.freq[i]))
        {
            k = (marker2.frequency - plotdata.freq[i-1]) / (plotdata.freq[i] - plotdata.freq[i-1]);
            xs = k * plotdata.x_imp[i] + (1-k) * plotdata.x_imp[i-1];
            ui->M2RLdoubleSpinBox->setValue(k * plotdata.return_loss[i] + (1-k) * plotdata.return_loss[i-1]);
            ui->M2SWRdoubleSpinBox->setValue(k * plotdata.swr[i] + (1-k) * plotdata.swr[i-1]);
            ui->M2PhaseDoubleSpinBox->setValue(k * plotdata.angle[i] + (1-k) * plotdata.angle[i-1]);
            ui->M2ZdoubleSpinBox->setValue(k * plotdata.z_imp[i] + (1-k) * plotdata.z_imp[i-1]);
            ui->M2RsDoubleSpinBox->setValue(k * plotdata.r_imp[i] + (1-k) * plotdata.r_imp[i-1]);
            ui->M2XsDoubleSpinBox->setValue(xs);
            ui->M2CsDoubleSpinBox->setValue(1e6 / (2 * M_PI * marker2.frequency * xs));
            ui->M2LsDoubleSpinBox->setValue(xs / (2 * M_PI * marker2.frequency));
            ui->M2G1DoubleSpinBox->setValue(k * plotdata.gain1[i] + (1-k) * plotdata.gain1[i-1]);
            ui->M2G2DoubleSpinBox->setValue(k * plotdata.gain2[i] + (1-k) * plotdata.gain2[i-1]);
            ui->M2G3DoubleSpinBox->setValue(k * plotdata.gain3[i] + (1-k) * plotdata.gain3[i-1]);
        }
}

void MainWindow::getMarkerMinData()
{
    int i;
    int i_min = 0;
    double minswr = 100;
    for (i=0;i<plotdata.points;i++)
        if (plotdata.swr[i] < minswr)
        {
            minswr = plotdata.swr[i];
            markermin.frequency = plotdata.freq[i];
            i_min = i;
        }
    MarkerMin->setValue(markermin.frequency, 0.0);
    ui->MarkerMinDoubleSpinBox->setValue(plotdata.freq[i_min]);
    ui->MinRLdoubleSpinBox->setValue(plotdata.return_loss[i_min]);
    ui->MinSWRdoubleSpinBox->setValue(plotdata.swr[i_min]);
    ui->MinPhaseDoubleSpinBox->setValue(plotdata.angle[i_min]);
    ui->MinZdoubleSpinBox->setValue(plotdata.z_imp[i_min]);
    ui->MinRsDoubleSpinBox->setValue(plotdata.r_imp[i_min]);
    ui->MinXsDoubleSpinBox->setValue(plotdata.x_imp[i_min]);
    ui->MinCsDoubleSpinBox->setValue(1e6 / (2 * M_PI * markermin.frequency * plotdata.x_imp[i_min]));
    ui->MinLsDoubleSpinBox->setValue(plotdata.x_imp[i_min] / (2 * M_PI * markermin.frequency));
}

void MainWindow::on_SWRcheckBox_clicked(bool checked)
{
    colordialog->line.SWR.enabled = checked;
    SetLeftScale();
    SWRline->setVisible(checked);
if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_RLcheckBox_clicked(bool checked)
{
    colordialog->line.RL.enabled = checked;
    SetRightScale();
    RLline->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_ZcheckBox_clicked(bool checked)
{
    colordialog->line.Z.enabled = checked;
    SetRightScale();
    Zline->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_PhaseCheckBox_clicked(bool checked)
{
    colordialog->line.Angle.enabled = checked;
    SetLeftScale();
    AngleLine->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_RsCheckBox_clicked(bool checked)
{
    colordialog->line.Rs.enabled = checked;
    SetRightScale();
    RsLine->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_XsCheckBox_clicked(bool checked)
{
    colordialog->line.Xs.enabled = checked;
    SetRightScale();
    XsLine->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_Gain1CheckBox_clicked(bool checked)
{
    colordialog->line.Gain1.enabled = checked;
    SetRightScale();
    Gain1Line->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_Gain2CheckBox_clicked(bool checked)
{
    colordialog->line.Gain2.enabled = checked;
    SetRightScale();
    Gain2Line->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_Gain3CheckBox_clicked(bool checked)
{
    colordialog->line.Gain3.enabled = checked;
    SetRightScale();
    Gain3Line->setVisible(checked);
    if (!running)
        DrawPlots(meas.points);
}

void MainWindow::on_ImpedanceRadioButton_clicked()
{
    UpdateMode(0, false);
}

void MainWindow::on_TransmittanceRadioButton_clicked()
{
    UpdateMode(1, false);
}

void MainWindow::on_TR1RadioButton_clicked()
{
    UpdateMode(2, false);
}

void MainWindow::on_TR2RadioButton_clicked()
{
    UpdateMode(3, false);
}

void MainWindow::on_TR3RadioButton_clicked()
{
    UpdateMode(4, false);
}

void MainWindow::on_GeneratorRadioButton_clicked()
{
    if (meas.measure_mode < 10)
        StopMeasurement();
    meas.measure_mode = 10;
    SetGeneratorMode();
}

void MainWindow::UpdateMode(int mode, bool loading)
{
    if (meas.measure_mode == 10)
    {
        EndGeneratorMode();
        StopMeasurement();
    }
    meas.display_mode = mode;
    if (!loading)
        meas.measure_mode = mode;
    switch (mode)
    {
    case 0:
        RightAxisName = tr("Return Loss [dB]");
        break;
    case 1:
        RightAxisName = tr("Gain [dB]");
        break;
    default:
        RightAxisName = tr("Return Loss / Gain [dB]");
        break;
    }
    if (running || loading)
    {
        UpdateGainCheckboxes(mode);
        SetRightScale();
    }
}

void MainWindow::UpdateGainCheckboxes(int mode)
{
    if (mode >= 2)
        ui->Gain1CheckBox->setCheckable(true);
    else
    {
        ui->Gain1CheckBox->setChecked(false);
        ui->Gain1CheckBox->setCheckable(false);
        colordialog->line.Gain1.enabled = false;
        Gain1Line->setVisible(false);
    }
    if (mode >= 3)
        ui->Gain2CheckBox->setCheckable(true);
    else
    {
        ui->Gain2CheckBox->setChecked(false);
        ui->Gain2CheckBox->setCheckable(false);
        colordialog->line.Gain2.enabled = false;
        Gain2Line->setVisible(false);
    }
    if (mode >= 4)
        ui->Gain3CheckBox->setCheckable(true);
    else
    {
        ui->Gain3CheckBox->setChecked(false);
        ui->Gain3CheckBox->setCheckable(false);
        colordialog->line.Gain3.enabled = false;
        Gain3Line->setVisible(false);
    }
}

void MainWindow::SetGeneratorMode()
{
    ui->MinFrequencyDoubleSpinBox->setEnabled(false);
    ui->PointsSpinBox->setEnabled(false);
    ui->MaxFreqLabel->setText(tr("Frequency"));
}

void MainWindow::EndGeneratorMode()
{
    ui->MinFrequencyDoubleSpinBox->setEnabled(true);
    ui->PointsSpinBox->setEnabled(true);
    ui->MaxFreqLabel->setText(tr("Max Freq."));
}

void MainWindow::UpdateHWLimits()
{
    if (settingsdialog->analyzer[meas.hardware - 1].transmitance_enabled)
    {
        ui->TransmittanceRadioButton->setEnabled(true);
    }
    else
    {
        ui->TransmittanceRadioButton->setEnabled(false);
    }
    switch (settingsdialog->analyzer[meas.hardware -1].detectors)
    {
    case 0:
        ui->TR1RadioButton->setEnabled(false);
        ui->TR2RadioButton->setEnabled(false);
        ui->TR3RadioButton->setEnabled(false);
        break;
    case 1:
        ui->TR1RadioButton->setEnabled(true);
        ui->TR2RadioButton->setEnabled(false);
        ui->TR3RadioButton->setEnabled(false);
        break;
    case 2:
        ui->TR1RadioButton->setEnabled(true);
        ui->TR2RadioButton->setEnabled(true);
        ui->TR3RadioButton->setEnabled(false);
        break;
    case 3:
        ui->TR1RadioButton->setEnabled(true);
        ui->TR2RadioButton->setEnabled(true);
        ui->TR3RadioButton->setEnabled(true);
        break;
    }
}

void MainWindow::SetHWLimits(bool set)
{
    if (set)
    {
        ui->MaxFrequencyDoubleSpinBox->setMaximum(settingsdialog->analyzer[meas.hardware - 1].fmax);
        ui->MaxFrequencyDoubleSpinBox->setMinimum(settingsdialog->analyzer[meas.hardware - 1].fmin);
        ui->MinFrequencyDoubleSpinBox->setMaximum(settingsdialog->analyzer[meas.hardware - 1].fmax);
        ui->MinFrequencyDoubleSpinBox->setMinimum(settingsdialog->analyzer[meas.hardware - 1].fmin);
    }
    else
    {
        ui->MaxFrequencyDoubleSpinBox->setMaximum(999.999999);
        ui->MaxFrequencyDoubleSpinBox->setMinimum(0.0);
        ui->MinFrequencyDoubleSpinBox->setMaximum(999.999999);
        ui->MinFrequencyDoubleSpinBox->setMinimum(0.0);
    }
}

void MainWindow::on_HWSpinBox_valueChanged(int arg1)
{
    meas.hardware = arg1;
    UpdateHWLimits();
}

void MainWindow::on_actionOpen_triggered()
{
    if (running)
        StopMeasurement();
    ReadFile();
}

void MainWindow::ReadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Load Measurement Results"), "",
        tr("VNA Binary (*.vec);;Comma Separated (*.csv)"));

    QFileInfo fi(fileName);
    QString extension = fi.suffix();

    if (fileName.isEmpty())
        return;
    else
    {
        int i;
        double fmin, fmax;
        double offset;
        double rl_mul;
        int points;

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }

        CleanGainData();

        if (extension == "vec")
        {
            quint8 version[10];
            char date[15];
            QByteArray desc;
            union
            {
                float a;
                quint32 k;
            };
            float b;
            double f, fstep;
            quint8 c, mode;
            QString s;

            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_4_5);
            in.setByteOrder(QDataStream::LittleEndian);

            desc.clear();
            in >> k;
            if (k == VNA_magic_number)
            {
                for (i=0;i<10;i++)
                {
                    in >> version[i];
                }

                for (i=0;i<15;i++)
                {
                    in >> c;
                    date[i] = c;
                }
                s = QString(date);
                meas.time = QDateTime::fromString(s,"yyyyMMddhhmmss");

                for (i=0;i<80;i++)
                {
                    in >> c;
                    if (c > 0)
                    {
                        desc += c;
                    }
                }
                commentdialog->comment = QString::fromUtf8(desc);

                in >> mode;
                UpdateMode(mode, true);

                for (i=0;i<2;i++)
                    in >> c;

                in >> a;
                in >> b;
                in >> points;
                fmin = a;
                fmax = b;
                plotdata.points = points;
                fstep = (fmax - fmin) / (points - 1);
                f = fmin;
                for (i=0;i<points;i++)
                {
                    plotdata.freq[i] = f;
                    in >> a;
                    in >> b;
                    dut_calc2(a, b, 0.0, 1.0, &pdut);
                    plotdata.return_loss[i] = pdut.return_loss;
                    plotdata.angle[i] = pdut.angle;
                    plotdata.angle_scaled[i] = pdut.angle * 0.05 + 1;
                    plotdata.swr[i] = pdut.swr;
                    plotdata.r_imp[i] = pdut.r_imp;
                    plotdata.r_imp_scaled[i] = pdut.r_imp * 0.1 - 40;
                    plotdata.x_imp[i] = pdut.x_imp;
                    plotdata.x_imp_scaled[i] = pdut.x_imp * 0.1 - 40;
                    plotdata.z_imp[i] = pdut.z_imp;
                    plotdata.z_imp_scaled[i] = pdut.z_imp * 0.1 - 40;
                    if (mode >= 2)
                    {
                        in >> a;
                        plotdata.gain1[i] = a;
                        if (mode >= 3)
                        {
                            in >> a;
                            plotdata.gain2[i] = a;
                            if (mode >= 4)
                            {
                                in >> a;
                                plotdata.gain3[i] = a;
                            }
                        }
                    }
                    f += fstep;
                }
                if (mode < 2)
                {
                    UpdateGain1(false);
                }
                if (mode < 3)
                {
                    UpdateGain2(false);
                }
                if (mode < 4)
                {
                    UpdateGain3(false);
                }
            }
            else
            {
                points = 501;
                plotdata.points = points;
                float rl[501];
                float angle[501];

                if (settingsdialog->analyzer[meas.hardware - 1].mode40dB)
                    rl_mul = -1.0;
                else
                    rl_mul = 1.0;
                if (meas.measure_mode == 1)
                {
                    offset = settingsdialog->analyzer[meas.hardware - 1].transmitance_offset + settingsdialog->analyzer[meas.hardware - 1].reflectance_offset;
                }
                else
                {
                    offset = settingsdialog->analyzer[meas.hardware - 1].reflectance_offset;
                }

                rl[0] = a;
                for (i=1;i<points;i++)
                {
                    in >> rl[i];
                }
                for (i=0;i<points;i++)
                {
                    in >> angle[i];
                }

                for (i=0;i<points;i++)
                {
                    in >> f;
                    plotdata.freq[i] = f;
                    dut_calc2(rl[i], angle[i], offset, rl_mul, &pdut);
                    plotdata.return_loss[i] = pdut.return_loss;
                    plotdata.angle[i] = pdut.angle;
                    plotdata.angle_scaled[i] = pdut.angle * 0.05 + 1;
                    plotdata.swr[i] = pdut.swr;
                    plotdata.r_imp[i] = pdut.r_imp;
                    plotdata.r_imp_scaled[i] = pdut.r_imp * 0.1 - 40;
                    plotdata.x_imp[i] = pdut.x_imp;
                    plotdata.x_imp_scaled[i] = pdut.x_imp * 0.1 - 40;
                    plotdata.z_imp[i] = pdut.z_imp;
                    plotdata.z_imp_scaled[i] = pdut.z_imp * 0.1 - 40;
                }
                in >> a;
                in >> b;
                fmin = a;
                fmax = b;

                commentdialog->comment = "";

                // Some programs output point frequency expressed in DDS steps, correct this

                double k = f/fmax;
                if (k > 1.01)
                {
                    for (i=0;i<points;i++)
                    {
                        plotdata.freq[i] /= k;
                    }
                }
            }
        }
        else
        {
            QString line;
            QStringList list;

            QTextStream in(&file);

            line = in.readLine();
            list = line.split(";");
            if (list[0].toULong() != VNA_magic_number)
            {
                QMessageBox::information(this, tr("Unable to read file"),
                                         tr("Not a VNA file"));
                file.close();
                return;
            }
            fmin = list[3].toDouble();
            fmax = list[4].toDouble();
            points = list[5].toInt();
            cout << list[6].toUtf8().data() << endl;

            line = in.readLine();   //skip line with table headers

            for (i=0;i<points;i++)
            {
                line = in.readLine();
                list = line.split(";");
                plotdata.freq[i] = list[0].toDouble();
                dut_calc2(list[1].toFloat(), list[2].toFloat(), 0.0, 1.0, &pdut);
                plotdata.return_loss[i] = pdut.return_loss;
                plotdata.angle[i] = pdut.angle;
                plotdata.angle_scaled[i] = pdut.angle * 0.05 + 1;
                plotdata.swr[i] = pdut.swr;
                plotdata.r_imp[i] = pdut.r_imp;
                plotdata.r_imp_scaled[i] = pdut.r_imp * 0.1 - 40;
                plotdata.x_imp[i] = pdut.x_imp;
                plotdata.x_imp_scaled[i] = pdut.x_imp * 0.1 - 40;
                plotdata.z_imp[i] = pdut.z_imp;
                plotdata.z_imp_scaled[i] = pdut.z_imp * 0.1 - 40;
            }
        }

        file.close();

        getMarkerData();
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,fmin,fmax);
        DrawPlots(points);

        ui->MinFrequencyDoubleSpinBox->setValue(fmin);
        ui->MaxFrequencyDoubleSpinBox->setValue(fmax);
        ui->PointsSpinBox->setValue(points);

        ui->FullRangeCheckBox->setChecked(false);
        freqcounter = 0;
    }
}

void MainWindow::CleanGainData()
{
    int i;
    for (i=0;i<MAXPOINTS;i++)
    {
        plotdata.gain1[i] = 0.0;
        plotdata.gain2[i] = 0.0;
        plotdata.gain3[i] = 0.0;
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (running)
    {
        StopMeasurement();
        WriteFileWithComment();
        StartMeasurement();
    }
    else
        WriteFileWithComment();
}

void MainWindow::WriteFileWithComment()
{
    if (commentdialog->ask_for_comment)
    {
        commentdialog = new CommentDialog(this);
        commentdialog->exec();
    }
    WriteFile();
}

void MainWindow::WriteFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Measurement Results"), "",
                                                    tr("VNA Binary (*.vec)"));

    if (fileName.isEmpty())
        return;
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        int i;
        quint8 version[10];
        quint8 date[15];
        quint8 descr[80];
        quint8 mode;
        quint8 nul = 0;
        float a, b;

        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_4_5);
        out.setByteOrder(QDataStream::LittleEndian);

        for (i=0;i<4;i++)
            out << VNA_magic[i];

        for (i=0;i<10;i++)
        {
            if (i < VNA_file_version.length())
                version[i] = VNA_file_version.at(i).toAscii();
            else
                version[i] = 0;
        }
        for (i=0;i<10;i++)
            out << version[i];

        QString dateTimeString = meas.time.toString("yyyyMMddhhmmss");
        for (i=0;i<15;i++)
        {
            if (i < dateTimeString.length())
                date[i] = dateTimeString.at(i).toAscii();
            else
                date[i] = 0;
        }
        for (i=0;i<15;i++)
            out << date[i];

        QByteArray descstring = commentdialog->comment.toUtf8();
        for (i=0;i<80;i++)
        {
            if (i < descstring.length())
                descr[i] = descstring.at(i);
            else
                descr[i] = 0;
        }
        for (i=0;i<80;i++)
            out << descr[i];

        mode = meas.display_mode;
        out << mode;

        for (i=0;i<2;i++)
            out << nul;

        a = plotdata.freq[0];
        b = plotdata.freq[plotdata.points - 1];

        out << a;
        out << b;
        out << plotdata.points;

        for (i=0;i<plotdata.points;i++)
        {
            a = (plotdata.return_loss[i] + 30.0) / 0.0586;
            b = plotdata.angle[i] / 0.175;
            out << a << b;
            if (mode >= 2)
            {
                a = plotdata.gain1[i];
                out << a;
                if (mode >= 3)
                {
                    a = plotdata.gain2[i];
                    out << a;
                    if (mode >= 4)
                    {
                        a = plotdata.gain3[i];
                        out << a;
                    }
                }
            }
        }
        file.close();
    }
}

void MainWindow::on_actionComment_triggered()
{
    commentdialog = new CommentDialog(this);
    commentdialog->show();
}

void MainWindow::on_actionImport_triggered()
{
    if (running)
        StopMeasurement();
    ReadFile();
}


void MainWindow::on_actionExport_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export Measurement Results"), "",
                                                    tr("VNA Binary (*.vec);;Comma Separated (*.csv)"));

    QFileInfo fi(fileName);
    QString extension = fi.suffix();

    if (fileName.isEmpty())
        return;
    else
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly))
        {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        if (extension == "vec")
        {
            int i;
            float a, b;
            double f;
            double rl_mul, offset;

            if (plotdata.points != 501)
                if (QMessageBox::warning(this, tr("Problem exporting binary VEC file"),
                                         tr("Number of points is other than 501. The file probably will not load correctly by qVNAmax or another software."),
                                         QMessageBox::Cancel, QMessageBox::Save) != QMessageBox::Save)
                    return;

            if (settingsdialog->analyzer[meas.hardware - 1].mode40dB)
                rl_mul = -1.0;
            else
                rl_mul = 1.0;
            if (meas.display_mode == 1)
            {
                offset = settingsdialog->analyzer[meas.hardware - 1].transmitance_offset + settingsdialog->analyzer[meas.hardware - 1].reflectance_offset;
            }
            else
            {
                offset = settingsdialog->analyzer[meas.hardware - 1].reflectance_offset;
            }

            QDataStream out(&file);
            out.setVersion(QDataStream::Qt_4_5);
            out.setByteOrder(QDataStream::LittleEndian);

            for (i=0;i<plotdata.points;i++)
            {
                a = ((plotdata.return_loss[i] - offset) * rl_mul + 30.0) / 0.0586;
                out << a;
            }

            for (i=0;i<plotdata.points;i++)
            {
                b = plotdata.angle[i] / 0.175;
                out << b;
            }

            for (i=0;i<plotdata.points;i++)
            {
                f = plotdata.freq[i];
                f *= (X2TO32 / settingsdialog->analyzer[meas.hardware - 1].DDSclock);
                out << f;
            }

            a = plotdata.freq[0];
            b = plotdata.freq[plotdata.points - 1];

            out << a;
            out << b;
        }
        else
        {
            int i;
            int mode = meas.display_mode;

            QTextStream out(&file);

            out << VNA_magic_number << ";";
            out << '\"' << VNA_file_version.toAscii() << '\"' << ";";

            QString dateTimeString = meas.time.toString("yyyyMMddhhmmss");
            out << '\"' << dateTimeString.toAscii() << '\"' << ";";
            out << plotdata.freq[0] << ";";
            out << plotdata.freq[plotdata.points - 1] << ";";
            out << plotdata.points << ";";
            out << '\"' << commentdialog->comment.toUtf8() << '\"' << endl;

            out << '\"' << "Freq." << '\"' << ';';
            out << '\"' << "R.L." << '\"' << ';';
            out << '\"' << "angle" << '\"' << ';';
            out << '\"' << "R.L.n" << '\"' << ';';
            out << '\"' << "angle-n" << '\"' << ';';
            out << '\"' << "SWR" << '\"' << ';';
            out << '\"' << "Xs" << '\"' << ';';
            out << '\"' << "Rs" << '\"' << ';';
            out << '\"' << "Z" << '\"' << ';';
            out << '\"' << "Cs" << '\"' << ';';
            out << '\"' << "Ls" << '\"' << ';';
            out << '\"' << "Rp" << '\"' << ';';
            out << '\"' << "Xp" << '\"';
            if (mode >= 2)
            {
                out << ';' << '\"' << "Gain1" << '\"' ;
                if (mode >= 3)
                {
                    out << ';' << '\"' << "Gain2"<< '\"' ;
                    if (mode >=4)
                    {
                        out << ';' << '\"' << "Gain3" << '\"';
                    }
                }
            }
            out << endl;

            for (i=0;i<plotdata.points;i++)
            {
                out << plotdata.freq[i] << ';';
                out << ((plotdata.return_loss[i] + 30.0) / 0.0586) << ';';
                out << (plotdata.angle[i] / 0.175) << ';';
                out << plotdata.return_loss[i] << ';';
                out << plotdata.angle[i] << ';';
                out << plotdata.swr[i] << ';';
                out << plotdata.x_imp[i] << ';';
                out << plotdata.r_imp[i] << ';';
                out << plotdata.z_imp[i] << ';';
                out << (1e6 / (plotdata.x_imp[i] * 2.0 * M_PI * plotdata.freq[i])) << ';';
                out << (plotdata.x_imp[i] * 2.0 * M_PI * plotdata.freq[i]) << ';';
                out << ((plotdata.x_imp[i] * plotdata.x_imp[i] + plotdata.r_imp[i] * plotdata.r_imp[i]) / plotdata.x_imp[i]) << ';';
                out << ((plotdata.x_imp[i] * plotdata.x_imp[i] + plotdata.r_imp[i] * plotdata.r_imp[i]) / plotdata.r_imp[i]);
                if (mode >= 2)
                {
                    out << ';' << plotdata.gain1[i];
                    if (mode >= 3)
                    {
                        out << ';' << plotdata.gain2[i];
                        if (mode >=4)
                        {
                            out << ';' << plotdata.gain3[i];
                        }
                    }
                }
                out << endl;
            }
        }

        file.close();
    }
}

#if QWT_VERSION >= 0x060000
void MainWindow::OnPickerPointSelected(const QPointF & p)
#else
void MainWindow::OnPickerPointSelected(const QwtDoublePoint & p)
#endif
{
    QPointF point = p;
//    cout << QString("x: %1, y: %2").arg(point.x()).arg(point.y()).toAscii().data() << endl;
    if (marker1.enabled)
        ui->Marker1DoubleSpinBox->setValue(point.x());
//    if (marker2.enabled)
//        ui->Marker2DoubleSpinBox->setValue(point.x());
}

#if QWT_VERSION >= 0x060000
void MainWindow::OnPicker2PointSelected(const QPointF & p)
#else
void MainWindow::OnPicker2PointSelected(const QwtDoublePoint & p)
#endif
{
    QPointF point = p;
//    cout << QString("x: %1, y: %2").arg(point.x()).arg(point.y()).toAscii().data() << endl;
    if (marker2.enabled)
        ui->Marker2DoubleSpinBox->setValue(point.x());
}

void MainWindow::on_actionColors_triggered()
{
    colordialog = new ColorDialog(this);
    colordialog->show();
}

void MainWindow::UpdateSWR(bool checked)
{
    ui->SWRcheckBox->setChecked(checked);
    on_SWRcheckBox_clicked(checked);
}

void MainWindow::UpdateRL(bool checked)
{
    ui->RLcheckBox->setChecked(checked);
    on_RLcheckBox_clicked(checked);
}

void MainWindow::UpdateAngle(bool checked)
{
    ui->PhaseCheckBox->setChecked(checked);
    on_PhaseCheckBox_clicked(checked);
}

void MainWindow::UpdateZ(bool checked)
{
    ui->ZcheckBox->setChecked(checked);
    on_ZcheckBox_clicked(checked);
}

void MainWindow::UpdateRs(bool checked)
{
    ui->RsCheckBox->setChecked(checked);
    on_RsCheckBox_clicked(checked);
}

void MainWindow::UpdateXs(bool checked)
{
    ui->XsCheckBox->setChecked(checked);
    on_XsCheckBox_clicked(checked);
}

void MainWindow::UpdateGain1(bool checked)
{
    ui->Gain1CheckBox->setChecked(checked);
    on_Gain1CheckBox_clicked(checked);
}

void MainWindow::UpdateGain2(bool checked)
{
    ui->Gain2CheckBox->setChecked(checked);
    on_Gain2CheckBox_clicked(checked);
}

void MainWindow::UpdateGain3(bool checked)
{
    ui->Gain3CheckBox->setChecked(checked);
    on_Gain3CheckBox_clicked(checked);
}

void MainWindow::UpdateSWRLineWidth()
{
    SWRline->setPen(QPen(colordialog->line.SWR.color, colordialog->line.SWR.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->SWRcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.SWR.color);
    ui->SWRcheckBox->setPalette(pal);
    colordialog->line.SWR.set = true;
}

void MainWindow::UpdateRLLineWidth()
{
    RLline->setPen(QPen(colordialog->line.RL.color, colordialog->line.RL.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->RLcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.RL.color);
    ui->RLcheckBox->setPalette(pal);
    colordialog->line.RL.set = true;
}

void MainWindow::UpdateAngleLineWidth()
{
    AngleLine->setPen(QPen(colordialog->line.Angle.color, colordialog->line.Angle.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->PhaseCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Angle.color);
    ui->PhaseCheckBox->setPalette(pal);
    colordialog->line.Angle.set = true;
}

void MainWindow::UpdateZLineWidth()
{
    Zline->setPen(QPen(colordialog->line.Z.color, colordialog->line.Z.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->ZcheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Z.color);
    ui->ZcheckBox->setPalette(pal);
    colordialog->line.Z.set = true;
}

void MainWindow::UpdateRsLineWidth()
{
    RsLine->setPen(QPen(colordialog->line.Rs.color, colordialog->line.Rs.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->RsCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Rs.color);
    ui->RsCheckBox->setPalette(pal);
    colordialog->line.Rs.set = true;
}

void MainWindow::UpdateXsLineWidth()
{
    XsLine->setPen(QPen(colordialog->line.Xs.color, colordialog->line.Xs.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->XsCheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Xs.color);
    ui->XsCheckBox->setPalette(pal);
    colordialog->line.Xs.set = true;
}

void MainWindow::UpdateGain1LineWidth()
{
    Gain1Line->setPen(QPen(colordialog->line.Gain1.color, colordialog->line.Gain1.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->Gain1CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain1.color);
    ui->Gain1CheckBox->setPalette(pal);
    colordialog->line.Gain1.set = true;
}

void MainWindow::UpdateGain2LineWidth()
{
    Gain2Line->setPen(QPen(colordialog->line.Gain2.color, colordialog->line.Gain2.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->Gain2CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain2.color);
    ui->Gain2CheckBox->setPalette(pal);
    colordialog->line.Gain2.set = true;
}

void MainWindow::UpdateGain3LineWidth()
{
    Gain3Line->setPen(QPen(colordialog->line.Gain3.color, colordialog->line.Gain3.width));
    if (!running)
            DrawPlots(meas.points);
    pal = ui->Gain3CheckBox->palette();
    pal.setColor(QPalette::Text,colordialog->line.Gain3.color);
    ui->Gain3CheckBox->setPalette(pal);
    colordialog->line.Gain3.set = true;
}

void MainWindow::on_StepComboBox_currentIndexChanged(int index)
{
    index *= -1;
    ui->MaxFrequencyDoubleSpinBox->setSingleStep(pow(10, index));
    ui->MinFrequencyDoubleSpinBox->setSingleStep(pow(10, index));
//    cout << "Step=" << index << "/" <<  pow(10,index) << endl;
}

void MainWindow::on_ZoomInPushButton_clicked()
{
    if (marker2.frequency != marker1.frequency)
    {
        if (freqcounter < 4)
        {
            freqmemory[freqcounter].fmin = ui->MinFrequencyDoubleSpinBox->value();
            freqmemory[freqcounter].fmax = ui->MaxFrequencyDoubleSpinBox->value();
            freqcounter++;
        }
        if (marker1.frequency > marker2.frequency)
        {
            ui->MinFrequencyDoubleSpinBox->setValue(marker2.frequency);
            ui->MaxFrequencyDoubleSpinBox->setValue(marker1.frequency);
        }
        else
        {
            ui->MinFrequencyDoubleSpinBox->setValue(marker1.frequency);
            ui->MaxFrequencyDoubleSpinBox->setValue(marker2.frequency);
        }
        ui->FullRangeCheckBox->setChecked(false);
    }
}

void MainWindow::on_ZoomOutPushButton_clicked()
{
    if (freqcounter > 0)
    {
        freqcounter--;
        ui->MaxFrequencyDoubleSpinBox->setValue(freqmemory[freqcounter].fmax);
        ui->MinFrequencyDoubleSpinBox->setValue(freqmemory[freqcounter].fmin);
     }
}

void MainWindow::on_FullRangeCheckBox_clicked(bool checked)
{
    fullrange = checked;
    if (!running)
    {
        if(fullrange)
        {
            int n;
            n = meas.hardware - 1;
            meas.fmin = settingsdialog->analyzer[n].fmin;
            meas.fmax = settingsdialog->analyzer[n].fmax;
        }
        else
        {
            meas.fmin = ui->MinFrequencyDoubleSpinBox->value();
            meas.fmax = ui->MaxFrequencyDoubleSpinBox->value();
        }
        ui->MainQwtPlot->setAxisScale(QwtPlot::xBottom,meas.fmin,meas.fmax);
        ui->MainQwtPlot->replot();
    }
}

void MainWindow::on_LineConfigurePushButton_clicked()
{
    colordialog = new ColorDialog(this);
    colordialog->show();
}
