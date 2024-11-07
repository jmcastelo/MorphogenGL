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

    void setStartTime();
    void startTimer();
    void stopTimer();

    void setTimerInterval(std::chrono::nanoseconds interval);

    void startRecording(QString recordFilename, int framesPerSecond, QMediaFormat format);
    void stopRecording();
    int getFrameCount();

signals:
    void morphoWidgetInitialized();
    void outputTextureChanged(GLuint id);
    void closing();

    void iterationPerformed();
    void iterationTimeMeasured(std::chrono::microseconds iterationTime, int numIterations);

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

    unsigned int numIterations = 100;
    double fps = 60.0;

    QWidget* stackedWidget;
    QStackedLayout* stackedLayout;

    QGraphicsOpacityEffect* opacityEffect;
    qreal controlWidgetOpacity = 0.75;

private slots:
    void beat();
    void iterate();

    void setIterationState(bool state);

    void takeScreenshot(QString filename);

    void setSize(int with, int height);
};


#endif // MAINWINDOW_H
