#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QColor>
#include <QFileDialog>
#include <qwt_global.h>
#include <QPointF>
#include <QDateTime>
#if QWT_VERSION < 0x060000
#include <qwt_double_rect.h>
#endif

#define N_HW 4
#define MAXPOINTS 501

struct MarkerStruct
{
    bool enabled;
    double frequency;
};

struct MeasureStruct
{
    double fmin;
    double fmax;
    double step;
    int points;
    bool fullscale;
    int hardware;
    bool new_analyzer;
    bool angle_scale;
    bool r_scale;
    int measure_mode;
    int display_mode;
    QString description;
    QDateTime time;
};

struct AnalyzerStruct
{
    bool configured;
    QString serialport;
    int serialportN;
    QString mode;
    int modeN;
    int port;
    int portN;
    bool transmitance_enabled;
    bool mode40dB;
    double DDSclock;
    double fmax;
    double fmin;
    double reflectance_offset;
    double transmitance_offset;
    int detectors;
    double det1_gain;
    double det2_gain;
    double det3_gain;
    double det1_offset;
    double det2_offset;
    double det3_offset;

};

struct PlotData
{
    int points;
    double freq[MAXPOINTS];
    double return_loss[MAXPOINTS];
    double angle[MAXPOINTS];
    double angle_scaled[MAXPOINTS];
    double swr[MAXPOINTS];
    double x_imp[MAXPOINTS];
    double x_imp_scaled[MAXPOINTS];
    double r_imp[MAXPOINTS];
    double r_imp_scaled[MAXPOINTS];
    double z_imp[MAXPOINTS];
    double z_imp_scaled[MAXPOINTS];
    double gain1[MAXPOINTS];
    double gain2[MAXPOINTS];
    double gain3[MAXPOINTS];
};

struct PDUT
{ /* device under test data */
    double  freq;
    double  return_loss;
    double  angle;
    double  swr;
    double  x_imp;
    double  r_imp;
    double  z_imp;
};

struct LineStruct
{
    bool enabled;
    QColor color;
    int width;
    bool set;
};

struct AllLinesStruct
{
    LineStruct SWR;
    LineStruct RL;
    LineStruct Z;
    LineStruct Rs;
    LineStruct Xs;
    LineStruct Angle;
    LineStruct Gain1;
    LineStruct Gain2;
    LineStruct Gain3;
    LineStruct Marker1;
    LineStruct Marker2;
    LineStruct MarkerMin;
};

struct FreqMemoryStruct
{
    double fmin;
    double fmax;
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void UpdateHWLimits();

    void UpdateSWR(bool checked);

    void UpdateRL(bool checked);

    void UpdateAngle(bool checked);

    void UpdateZ(bool checked);

    void UpdateRs(bool checked);

    void UpdateXs(bool checked);

    void UpdateGain1(bool checked);

    void UpdateGain2(bool checked);

    void UpdateGain3(bool checked);

    void UpdateSWRLineWidth();

    void UpdateRLLineWidth();

    void UpdateAngleLineWidth();

    void UpdateZLineWidth();

    void UpdateRsLineWidth();

    void UpdateXsLineWidth();

    void UpdateGain1LineWidth();

    void UpdateGain2LineWidth();

    void UpdateGain3LineWidth();

private slots:
    void on_actionPrint_triggered();

    void on_actionExit_triggered();

    void on_actionAbout_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionSetup_triggered();

    void on_MinFrequencyDoubleSpinBox_valueChanged(double arg1);

    void on_MaxFrequencyDoubleSpinBox_valueChanged(double arg1);

    void on_PointsSpinBox_valueChanged(int arg1);

    void on_QuitPushButton_clicked();

    void on_Marker1CheckBox_clicked();

    void on_Marker2CheckBox_clicked();

    void on_MinSWRCheckBox_clicked();

    void on_Marker1DoubleSpinBox_valueChanged(double arg1);

    void on_Marker2DoubleSpinBox_valueChanged(double arg1);

    void on_StartPushButton_clicked();

    void on_SWRcheckBox_clicked(bool checked);

    void on_RLcheckBox_clicked(bool checked);

    void on_ZcheckBox_clicked(bool checked);

    void on_PhaseCheckBox_clicked(bool checked);

    void on_RsCheckBox_clicked(bool checked);

    void on_XsCheckBox_clicked(bool checked);

    void on_Gain1CheckBox_clicked(bool checked);

    void on_Gain2CheckBox_clicked(bool checked);

    void on_Gain3CheckBox_clicked(bool checked);

    void on_ImpedanceRadioButton_clicked();

    void on_TransmittanceRadioButton_clicked();

    void on_TR1RadioButton_clicked();

    void on_TR2RadioButton_clicked();

    void on_TR3RadioButton_clicked();

    void on_GeneratorRadioButton_clicked();

    void on_HWSpinBox_valueChanged(int arg1);

    void Measure();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

    void on_actionComment_triggered();

    void on_actionImport_triggered();

    void on_actionExport_triggered();

#if QWT_VERSION >= 0x060000
    void OnPickerPointSelected(const QPointF &p);

    void OnPicker2PointSelected(const QPointF &p);
#else
    void OnPickerPointSelected(const QwtDoublePoint &p);

    void OnPicker2PointSelected(const QwtDoublePoint &p);
#endif

    void on_MinFrequencyDoubleSpinBox_editingFinished();

    void on_MaxFrequencyDoubleSpinBox_editingFinished();

    void on_actionColors_triggered();

    void on_StepComboBox_currentIndexChanged(int index);

    void on_ZoomInPushButton_clicked();

    void on_ZoomOutPushButton_clicked();

    void on_FullRangeCheckBox_clicked(bool checked);

    void on_LineConfigurePushButton_clicked();

private:
    Ui::MainWindow *ui;

    void CalculateStep();

    void ReadSettings();

    void WriteSettings();

    void InitPlot();

    void SetLeftScale();

    void SetRightScale();

    void StartMeasurement();

    void StopMeasurement();

    void Measure2(double fmin, double fmax, int n, double clock, double gain_offset, double refl_offset, bool mode40dB, int mode, double *k);

    void DrawPlots(int points);

    bool dut_calc2(int return_loss, int angle, double rl_offset, double rl_cal, PDUT *pdut);

    void getMarkerData();

    void getMarker1Data();

    void getMarker2Data();

    void getMarkerMinData();

    void ReadFile();

    void CleanGainData();

    void WriteFileWithComment();

    void WriteFile();

    void UpdateMode(int mode, bool loading);

    void UpdateGainCheckboxes(int mode);

    void SetGeneratorMode();

    void EndGeneratorMode();

    void StartGenerator(double freq);

    void SetHWLimits(bool set);
};

#endif // MAINWINDOW_H
