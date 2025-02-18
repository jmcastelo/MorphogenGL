#include "gridwidget.h"

#include <QDrag>
#include <QPixmap>
#include <QMimeData>
#include <QApplication>



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



void GridWidget::clear()
{
    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr)
    {
        QWidget* widget = child->widget();
        gridLayout->removeWidget(widget);
        delete child;
    }
}



int GridWidget::itemIndex(QPoint pos)
{
    for (int index = 0; index < gridLayout->count(); index++)
        if (gridLayout->itemAt(index)->geometry().contains(pos) && index != sourceIndex)
            return index;

    return -1;
}



void GridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        sourceIndex = itemIndex(event->position().toPoint());
        dragStartPosition = event->position().toPoint();
    }
    else
        sourceIndex = -1;
}



void GridWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
    sourceIndex = -1;
}



void GridWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton && sourceIndex >= 0 && (event->position().toPoint() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    {
        QWidget* widget = gridLayout->itemAt(sourceIndex)->widget();

        QPixmap pixmap = widget->grab();

        QMimeData mimeData;
        mimeData.setImageData(pixmap);

        QDrag drag(this);
        drag.setMimeData(&mimeData);
        drag.setPixmap(pixmap);
        drag.setHotSpot(QPoint(0, 0));
        drag.exec(Qt::IgnoreAction);
    }
}



void GridWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasImage())
        event->acceptProposedAction();
}



void GridWidget::dropEvent(QDropEvent* event)
{
    int targetIndex = itemIndex(event->position().toPoint());

    if (targetIndex >= 0 && targetIndex != sourceIndex)
    {
        int i = qMax(targetIndex, sourceIndex);
        int j = qMin(targetIndex, sourceIndex);

        int rowI, colI, rowSpanI, colSpanI;
        gridLayout->getItemPosition(i, &rowI, &colI, &rowSpanI, &colSpanI);

        int rowJ, colJ, rowSpanJ, colSpanJ;
        gridLayout->getItemPosition(j, &rowJ, &colJ, &rowSpanJ, &colSpanJ);

        gridLayout->addItem(gridLayout->takeAt(i), rowJ, colJ, rowSpanJ, colSpanJ);
        gridLayout->addItem(gridLayout->takeAt(j), rowI, colI, rowSpanI, colSpanI);

        emit itemRowColChanged(gridLayout->itemAt(i)->widget(), rowJ, colJ);
        emit itemRowColChanged(gridLayout->itemAt(j)->widget(), rowI, colI);

        sourceIndex = -1;

        event->acceptProposedAction();
    }
}
