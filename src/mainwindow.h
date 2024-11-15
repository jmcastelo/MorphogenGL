#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "generator.h"
#include "morphowidget.h"
#include "controlwidget.h"
#include "plotswidget.h"
#include "recorder.h"

#include <QMainWindow>
#include <QStackedLayout>
#include <QGraphicsOpacityEffect>
#include <QChronoTimer>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();
    int getFrameCount();

signals:
    void outputTextureChanged(GLuint id);
    void closing();

    void iterationPerformed();
    void iterationTimeMeasured(double uspf, double currentFPS);

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    GeneratorGL* generator;
    MorphoWidget* morphoWidget;
    ControlWidget* controlWidget;
    PlotsWidget* plotsWidget;
    Recorder* recorder = nullptr;

    QChronoTimer* timer;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

    int numBeats = 0;
    int numBeatsTrigger = 60;
    double fps = 60.0;
    std::chrono::microseconds beatTime;

    QWidget* stackedWidget;
    QStackedLayout* stackedLayout;

    QGraphicsOpacityEffect* opacityEffect;
    qreal controlWidgetOpacity = 0.7;

private slots:
    void beat();
    void iterate();

    void computeFPS();
    void setTimerInterval(double newFPS);

    void setIterationState(bool state);

    void takeScreenshot(QString filename);

    void setSize(int with, int height);
};


#endif // MAINWINDOW_H
