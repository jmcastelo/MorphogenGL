#ifndef MAINWINDOW_H
#define MAINWINDOW_H



#include "factory.h"
#include "rendermanager.h"
#include "nodemanager.h"
#include "morphowidget.h"
#include "controlwidget.h"
#include "plotswidget.h"
#include "recorder.h"
#include "timerthread.h"
#include "midicontrol.h"
#include "midilistwidget.h"
#include "midilinkmanager.h"
#include "overlay.h"

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

    void iterationPerformed();
    void iterationTimeMeasured(double uspf, double currentFPS);
    void updateTimeMeasured(double uspf, double currentFPS);

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Factory* factory;
    RenderManager* renderManager;
    NodeManager* nodeManager;
    MorphoWidget* morphoWidget;
    ControlWidget* controlWidget;
    PlotsWidget* plotsWidget;
    Recorder* recorder = nullptr;
    MidiControl midiControl;
    MidiListWidget* midiListWidget;
    MidiLinkManager midiLinkManager;
    Overlay* overlay;

    TimerThread* updateTimer;
    std::chrono::time_point<std::chrono::steady_clock> updateStart;
    std::chrono::time_point<std::chrono::steady_clock> updateEnd;

    TimerThread* iterationTimer;
    std::chrono::time_point<std::chrono::steady_clock> iterationStart;
    std::chrono::time_point<std::chrono::steady_clock> iterationEnd;

    int numUpdates = 0;
    double updateFPS = 60.0;
    std::chrono::microseconds updateTime;

    int numIterations = 0;
    double iterationFPS = 60.0;
    std::chrono::microseconds iterationTime;

    QWidget* stackedWidget;
    QStackedLayout* stackedLayout;

    QGraphicsOpacityEffect* controlWidgetOpacityEffect;
    QGraphicsOpacityEffect* plotsWidgetOpacityEffect;
    qreal opacity = 0.9;

private slots:
    void beat();
    void iterate();

    void computeUpdateFPS();
    void computeIterationFPS();

    void setUpdateTimerInterval(double newFPS);
    void setIterationTimerInterval(double newFPS);

    void setIterationState(bool state);

    void takeScreenshot(QString filename);

    void setSize(int with, int height);

    void showMidiWidget();
};


#endif // MAINWINDOW_H
