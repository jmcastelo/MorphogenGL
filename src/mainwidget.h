#pragma once

#include "morphowidget.h"
#include "controlwidget.h"
#include <QWidget>
#include <QImage>

class Heart;
class GeneratorGL;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(Heart* heart, QWidget* parent = nullptr);
    ~MainWidget();

    GeneratorGL* generator(){ return controlWidget->generator; }

    void computePlots();
    QImage grabMorphoWidgetFramebuffer();
    QOpenGLContext* morphoWidgetContext(){ return morphoWidget->context(); }
    int morphoWidgetWidth(){ return morphoWidget->width(); }
    int morphoWidgetHeight(){ return morphoWidget->height(); }

signals:
    void morphoWidgetInitialized();
    void outputTextureChanged(GLuint id);
    void closing();

public slots:
    void updateMorphoWidget();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    MorphoWidget* morphoWidget;
    ControlWidget* controlWidget;

    int oldWidth;
    int oldHeight;

private slots:
    void detachControlWidget();
};
