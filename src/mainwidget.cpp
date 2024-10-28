#include "heart.h"
#include "mainwidget.h"
#include <QDebug>

MainWidget::MainWidget(Heart* heart, QWidget* parent) : QWidget(parent)
{
    // MorphoWidget

    morphoWidget = new MorphoWidget(this);
    //morphoWidget->setUpdatesEnabled(false);

    // ControlWidget

    controlWidget = new ControlWidget(heart, this);
    controlWidget->setGeometry(682, 0, 342, 1024);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(morphoWidget);

    setLayout(mainLayout);

    controlWidget->raise();

    setWindowIcon(QIcon(":/icons/morphogengl.png"));

    connect(morphoWidget, &MorphoWidget::openGLInitialized, this, [=]()
    {
        controlWidget->generator->init(morphoWidget->context());
        controlWidget->initPlotsWidget(morphoWidget->context());
        emit morphoWidgetInitialized();
    });
    connect(morphoWidget, &MorphoWidget::screenSizeChanged, controlWidget, &ControlWidget::updateWindowSizeLineEdits);
    connect(morphoWidget, &MorphoWidget::screenSizeChanged, controlWidget->generator, &GeneratorGL::resize);
    connect(morphoWidget, &MorphoWidget::selectedPointChanged, controlWidget, &ControlWidget::selectedPointChanged);

    connect(controlWidget, &ControlWidget::updateStateChanged, morphoWidget, &MorphoWidget::setUpdate);
    connect(controlWidget, &ControlWidget::detach, this, &MainWidget::detachControlWidget);

    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, morphoWidget, &MorphoWidget::updateOutputTextureID);
    connect(controlWidget->generator, &GeneratorGL::outputTextureChanged, this, &MainWidget::outputTextureChanged);
    connect(controlWidget->generator, &GeneratorGL::imageSizeChanged, morphoWidget, &MorphoWidget::resetZoom);
}

MainWidget::~MainWidget()
{
    delete morphoWidget;
    delete controlWidget;
}

void MainWidget::updateMorphoWidget()
{
    morphoWidget->update();
}

QImage MainWidget::grabMorphoWidgetFramebuffer()
{
    return morphoWidget->grabFramebuffer();
}

void MainWidget::computePlots()
{
    controlWidget->computePlots();
}

void MainWidget::closeEvent(QCloseEvent* event)
{
    controlWidget->close();
    emit closing();
    event->accept();
}

void MainWidget::resizeEvent(QResizeEvent* event)
{
    qreal factor = event->oldSize().width() > 0 ? static_cast<qreal>(event->size().width()) / static_cast<qreal>(event->oldSize().width()) : 1.0;
    int newWidth = static_cast<int>(factor * controlWidget->width());

    controlWidget->setFixedHeight(event->size().height());
    controlWidget->setMaximumWidth(event->size().width());

    controlWidget->setGeometry(event->size().width() - newWidth, 0, newWidth, event->size().height());

    controlWidget->grip->setVisible(true);

    oldWidth = event->size().width();
    oldHeight = event->size().height();

    event->accept();
}

void MainWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::ControlModifier)
        controlWidget->setVisible(!controlWidget->isVisible());
    event->accept();
}

void MainWidget::detachControlWidget()
{
    if (controlWidget->parentWidget() == this)
    {
        oldControlWidgetWidth = controlWidget->width();

        controlWidget->setParent(nullptr);

        controlWidget->setWindowFlag(Qt::Window, true);
        controlWidget->setWindowFlag(Qt::WindowCloseButtonHint, false);
        controlWidget->setWindowTitle("Control panel");

        controlWidget->setMinimumHeight(0);
        controlWidget->setMaximumHeight(QWIDGETSIZE_MAX);

        controlWidget->setMaximumWidth(QWIDGETSIZE_MAX);

        //controlWidget->grip->setVisible(false);
    }
    else
    {
        controlWidget->setParent(this);

        controlWidget->setFixedHeight(height());
        controlWidget->setMaximumWidth(width());

        //QResizeEvent event(QSize(width(), height()), QSize(width(), height()));
        //resizeEvent(&event);

        //controlWidget->grip->setVisible(true);
    }

    controlWidget->show();
}
