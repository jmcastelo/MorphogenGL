


#include "seedwidget.h"

#include <QToolBar>
#include <QAction>
#include <QActionGroup>
#include <QHBoxLayout>
#include <QFileDialog>



SeedWidget::SeedWidget(Seed *seed, QWidget *parent) :
    QFrame(parent),
    mSeed { seed }
{
    // Header widget

    headerWidget = new QWidget;
    headerWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    headerWidget->setStyleSheet("QWidget { background-color: rgb(128, 128, 164); }");

    QToolBar* headerToolBar = new QToolBar;
    headerToolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    // Draw seed action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/applications-graphics.png")), "Draw", this, &SeedWidget::drawSeed);

    // Set as output action

    outputAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/eye.png")), "Set as output", this, &SeedWidget::outputChanged);
    outputAction->setCheckable(true);

    // Set fixed action

    fixedAction = headerToolBar->addAction(mSeed->isFixed() ? QIcon(QPixmap(":/icons/document-encrypt.png")) : QIcon(QPixmap(":/icons/document-decrypt.png")), mSeed->isFixed() ? "Fixed" : "Not fixed", this, &SeedWidget::setFixedSeed);
    fixedAction->setCheckable(true);
    fixedAction->setChecked(mSeed->isFixed());

    headerToolBar->addSeparator();

    // Seed type actions

    QAction* colorAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/color-chooser.png")), "Random: color", this, &SeedWidget::setSeedType);
    colorAction->setCheckable(true);
    colorAction->setData(QVariant(0));

    QAction* grayScaleAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/gray-chooser.png")), "Random: grayscale", this, &SeedWidget::setSeedType);
    grayScaleAction->setCheckable(true);
    grayScaleAction->setData(QVariant(1));

    QAction* imageAction = headerToolBar->addAction(QIcon(QPixmap(":/icons/image-x-generic.png")), "Image", this, &SeedWidget::setSeedType);
    imageAction->setCheckable(true);
    imageAction->setData(QVariant(2));

    QActionGroup* type = new QActionGroup(this);
    type->addAction(colorAction);
    type->addAction(grayScaleAction);
    type->addAction(imageAction);

    colorAction->setChecked(mSeed->type() == 0);
    grayScaleAction->setChecked(mSeed->type() == 1);
    imageAction->setChecked(mSeed->type() == 2);

    headerToolBar->addSeparator();

    // Load seed image action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/folder-image.png")), "Load image", this, &SeedWidget::loadSeedImage);

    headerToolBar->addSeparator();

    // Connect action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/network-connect.png")), "Connect", this, &SeedWidget::connectTo);

    headerToolBar->addSeparator();

    // Remove action

    headerToolBar->addAction(QIcon(QPixmap(":/icons/dialog-close.png")), "Delete", this, &SeedWidget::remove);

    QHBoxLayout* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(headerToolBar);

    headerWidget->setLayout(headerLayout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(headerWidget);

    setLayout(mainLayout);

    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setMidLineWidth(3);
    setLineWidth(3);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}



void SeedWidget::drawSeed()
{
    mSeed->draw();
    emit seedDrawn();
}



void SeedWidget::setFixedSeed(bool fixed)
{
    mSeed->setFixed(fixed);

    fixedAction->setIcon(fixed ? QIcon(QPixmap(":/icons/document-encrypt.png")) : QIcon(QPixmap(":/icons/document-decrypt.png")));
    fixedAction->setText(fixed ? "Fixed" : "Not fixed");
}



void SeedWidget::setSeedType()
{
    QAction* action = qobject_cast<QAction*>(sender());
    mSeed->setType(action->data().toInt());
    emit typeChanged();
}



void SeedWidget::loadSeedImage()
{
    QString filename = QFileDialog::getOpenFileName(this, "Load image", QDir::homePath(), "Images (*.bmp *.jpeg *.jpg *.png *.tif *.tiff)");

    if (!filename.isEmpty())
        mSeed->loadImage(filename);
}



void SeedWidget::toggleOutputAction(QWidget *widget)
{
    if (widget != this)
    {
        outputAction->setChecked(false);
        headerWidget->setStyleSheet("QWidget { background-color: rgb(128, 128, 164); }");
    }
    else
        headerWidget->setStyleSheet("QWidget { background-color: rgb(164, 128, 128); }");
}
