#include "gridwidget.h"

#include <QDrag>
#include <QPixmap>
#include <QMimeData>
#include <QApplication>



GridWidget::GridWidget(QWidget *parent) :
    QWidget{parent}
{
    gridLayout = new QGridLayout;

    placeholder = new QWidget;

    setAcceptDrops(true);
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
        if (gridLayout->itemAt(index)->geometry().contains(pos))
            return index;

    return -1;
}



void GridWidget::moveItemsUp(int row, int col)
{
    for (int r = row + 1; r < gridLayout->rowCount(); r++)
    {
        QLayoutItem* item = gridLayout->itemAtPosition(r, col);
        if (item)
        {
            int index = gridLayout->indexOf(item);
            gridLayout->addItem(gridLayout->takeAt(index), r - 1, col);
            emit itemRowColChanged(item->widget(), r - 1, col);
        }
    }
}



void GridWidget::moveItemsDown(int row, int col)
{
    for (int r = row; r < gridLayout->rowCount(); r++)
    {
        QLayoutItem* item = gridLayout->itemAtPosition(r, col);
        if (item)
        {
            int index = gridLayout->indexOf(item);
            gridLayout->addItem(gridLayout->takeAt(index), r + 1, col);
            emit itemRowColChanged(item->widget(), r + 1, col);
        }
    }
}



void GridWidget::swapItems(QLayoutItem* item1, QLayoutItem* item2)
{
    int index1 = gridLayout->indexOf(item1);
    int index2 = gridLayout->indexOf(item2);

    if (index1 >= 0 && index2 >= 0 && index1 != index2)
    {
        int row1, col1, rowSpan1, colSpan1;
        gridLayout->getItemPosition(index1, &row1, &col1, &rowSpan1, &colSpan1);

        int row2, col2, rowSpan2, colSpan2;
        gridLayout->getItemPosition(index2, &row2, &col2, &rowSpan2, &colSpan2);

        QLayoutItem* takenItem1 = gridLayout->takeAt(gridLayout->indexOf(item1));
        QLayoutItem* takenItem2 = gridLayout->takeAt(gridLayout->indexOf(item2));

        gridLayout->addItem(takenItem1, row2, col2, rowSpan2, colSpan2);
        gridLayout->addItem(takenItem2, row1, col1, rowSpan1, colSpan1);

        emit itemRowColChanged(takenItem1->widget(), row2, col2);
        emit itemRowColChanged(takenItem2->widget(), row1, col1);
    }
}



void GridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        int sourceIndex = itemIndex(event->position().toPoint());

        if (sourceIndex >= 0)
        {
            QWidget* widget = gridLayout->itemAt(sourceIndex)->widget();

            QPixmap pixmap = widget->grab();

            QByteArray itemData;
            QDataStream dataStream(&itemData, QIODevice::WriteOnly);
            dataStream << sourceIndex;

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-dnditemdata", itemData);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmap);
            drag->setHotSpot(event->position().toPoint() - widget->pos());
            drag->exec(Qt::MoveAction);
        }
    }
}



void GridWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        int sourceIndex;
        dataStream >> sourceIndex;

        int row, col, rowSpan, colSpan;
        gridLayout->getItemPosition(sourceIndex, &row, &col, &rowSpan, &colSpan);

        sourceItem = gridLayout->takeAt(sourceIndex);

        //moveItemsUp(row, col);

        placeholder->setFixedSize(sourceItem->widget()->size());

        gridLayout->addWidget(placeholder, row, col, Qt::AlignCenter);
        placeholderItem = gridLayout->itemAtPosition(row, col);

        event->accept();
    }
    else
    {
        event->ignore();
    }
}



void GridWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        int targetIndex = itemIndex(event->position().toPoint());

        if (targetIndex >= 0 && targetIndex != oldTargetIndex)
        {
            QLayoutItem* targetItem = gridLayout->itemAt(targetIndex);

            if (targetItem != placeholderItem)
            {
                int row, col, rowSpan, colSpan;
                gridLayout->getItemPosition(targetIndex, &row, &col, &rowSpan, &colSpan);

                int placeholderIndex = gridLayout->indexOf(placeholderItem);

                int phRow, phCol, phRowSpan, phColSpan;
                gridLayout->getItemPosition(placeholderIndex, &phRow, &phCol, &phRowSpan, &phColSpan);

                if (col == phCol)
                {
                    swapItems(placeholderItem, targetItem);
                }
                else
                {
                    moveItemsDown(row, col);

                    gridLayout->addItem(gridLayout->takeAt(placeholderIndex), row, col, phRowSpan, phColSpan, Qt::AlignCenter);
                    placeholderItem = gridLayout->itemAtPosition(row, col);

                    moveItemsUp(phRow, phCol);
                }
            }
        }

        oldTargetIndex = targetIndex;

        event->accept();
        return;
    }

    event->ignore();
}



void GridWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        int targetIndex = itemIndex(event->position().toPoint());
        int placeholderIndex = gridLayout->indexOf(placeholderItem);

        if (targetIndex >= 0 && placeholderIndex >= 0 && targetIndex == placeholderIndex)
        {
            int phRow, phCol, phRowSpan, phColSpan;
            gridLayout->getItemPosition(placeholderIndex, &phRow, &phCol, &phRowSpan, &phColSpan);

            gridLayout->takeAt(placeholderIndex);

            gridLayout->addItem(sourceItem, phRow, phCol);

            emit itemRowColChanged(sourceItem->widget(), phRow, phCol);
        }

        event->accept();
        return;
    }

    event->ignore();
}
