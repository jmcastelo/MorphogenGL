#include "gridwidget.h"

#include <QDrag>
#include <QPixmap>
#include <QMimeData>



GridWidget::GridWidget(QWidget *parent) :
    QWidget{parent}
{
    setAcceptDrops(true);

    gridLayout = new QGridLayout;

    setLayout(gridLayout);
}



void GridWidget::addWidget(QWidget* widget, int row, int column)
{
    gridLayout->addWidget(widget, row, column, Qt::AlignCenter);
}



int GridWidget::itemIndex(QPoint pos)
{
    for (int index = 0; index < gridLayout->count(); index++)
    {
        if (gridLayout->itemAt(index)->geometry().contains(pos) && index != targetIndex)
            return index;
    }

    return -1;
}



void GridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
        targetIndex = itemIndex(event->position().toPoint());
    else
        targetIndex = -1;
}



void GridWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    targetIndex = -1;
}



void GridWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton && targetIndex >= 0)
    {
        QWidget* widget = gridLayout->itemAt(targetIndex)->widget();
        QDrag drag(widget);
        QPixmap pixmap = widget->grab();
        QMimeData mimeData;
    }
}



void GridWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasImage())
        event->accept();
    else
        event->ignore();
}



void GridWidget::dropEvent(QDropEvent* event)
{
}
