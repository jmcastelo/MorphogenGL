#include "gridwidget.h"

#include <QDrag>
#include <QPixmap>
#include <QMimeData>
#include <QApplication>



GridWidget::GridWidget(QWidget *parent) :
    QWidget{parent}
{
    gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(margin, margin, margin, margin);
    gridLayout->setSpacing(spacing);

    setAcceptDrops(true);
    setLayout(gridLayout);
}



void GridWidget::addWidget(QWidget* widget, int row, int column)
{
    gridLayout->addWidget(widget, row, column, Qt::AlignCenter);
    widget->show();
}



void GridWidget::clear()
{
    QLayoutItem* child;
    while ((child = gridLayout->takeAt(0)) != nullptr)
    {
        QWidget* widget = child->widget();
        gridLayout->removeWidget(widget);
        delete widget;
        delete child;
    }

    itemCoords.clear();
}



void GridWidget::optimizeLayout()
{
    // Obtain widget's minimum width and height

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QRect widgetRect = gridLayout->itemAt(index)->widget()->geometry();

        if (index == 0)
        {
            minWidth = widgetRect.width();
            minHeight = widgetRect.height();
        }

        if (widgetRect.width() < minWidth)
            minWidth = widgetRect.width();
        if (widgetRect.height() < minHeight)
            minHeight = widgetRect.height();
    }

    // Set the row and column spans of items in proportion to the minimum size

    itemCoords.clear();

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QLayoutItem* item = gridLayout->itemAt(index);
        QWidget* widget = item->widget();
        QRect widgetRect = widget->geometry();

        qreal heightRatio = static_cast<qreal>(widgetRect.height()) / minHeight;
        int rowSpan = qFloor(heightRatio + 0.5);

        qreal widthRatio = static_cast<qreal>(widgetRect.width()) / minWidth;
        int colSpan = qFloor(widthRatio + 0.5);

        int row, col, rSpan, cSpan;
        gridLayout->getItemPosition(index, &row, &col, &rSpan, &cSpan);

        itemCoords.insert(item, QList<int>{ row, col, rowSpan, colSpan });
    }

    // Compute new positions (row, col) considering the spans

    for (int col = 0; col < gridLayout->columnCount(); col++)
    {
        int nextRow = 0;

        for (int row = 0; row < gridLayout->rowCount(); row++)
        {
            QLayoutItem* item = gridLayout->itemAtPosition(row, col);
            if (item)
            {
                itemCoords[item][0] = (row <= nextRow) ? nextRow : row;
                nextRow = itemCoords[item][0] + itemCoords[item][2];
            }
        }
    }

    for (int row = 0; row < gridLayout->rowCount(); row++)
    {
        int nextCol = 0;

        for (int col = 0; col < gridLayout->rowCount(); col++)
        {
            QLayoutItem* item = gridLayout->itemAtPosition(row, col);
            if (item)
            {
                itemCoords[item][1] = (col <= nextCol) ? nextCol : col;
                nextCol = itemCoords[item][1] + itemCoords[item][3];
            }
        }
    }

    // Relocate items

    for (auto [item, coords] : itemCoords.asKeyValueRange())
    {
        gridLayout->removeItem(item);
        gridLayout->addItem(item, coords[0], coords[1], coords[2], coords[3], Qt::AlignCenter);
    }

    setRowColSizes();
}



void GridWidget::setRowColSizes()
{
    int maxRow = getMaxRow();
    int maxCol = getMaxCol();

    for (int row = 0; row <= maxRow; row++)
    {
        if (countItemsInRow(row, maxCol) == 0)
            gridLayout->setRowMinimumHeight(row, minHeight);
        else
            gridLayout->setRowMinimumHeight(row, 0);
    }

    int maxGridRow = gridLayout->rowCount() - 1;
    for (int row = maxRow + 1; row <= maxGridRow; row++)
        gridLayout->setRowMinimumHeight(row, 0);

    for (int col = 0; col <= maxCol; col++)
    {
        if (countItemsInCol(col, maxRow) == 0)
            gridLayout->setColumnMinimumWidth(col, minWidth);
        else
            gridLayout->setColumnMinimumWidth(col, 0);
    }

    int maxGridCol = gridLayout->columnCount() - 1;
    for (int col = maxCol + 1; col <= maxGridCol; col++)
        gridLayout->setColumnMinimumWidth(col, 0);
}



int GridWidget::itemIndex(QPoint pos)
{
    for (int index = 0; index < gridLayout->count(); index++)
        if (gridLayout->itemAt(index)->geometry().contains(pos))
            return index;

    return -1;
}



int GridWidget::getMaxRow()
{
    int maxRow = 0;

    for (int index = 0; index < gridLayout->count(); index++)
    {
        int row, col, rowSpan, colSpan;
        gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

        if (row + rowSpan - 1 > maxRow)
            maxRow = row + rowSpan - 1;
    }

    return maxRow;
}



int GridWidget::getMaxCol()
{
    int maxCol = 0;

    for (int index = 0; index < gridLayout->count(); index++)
    {
        int row, col, rowSpan, colSpan;
        gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

        if (col + colSpan - 1 > maxCol)
            maxCol = col + colSpan - 1;
    }

    return maxCol;
}



void GridWidget::moveItem(QLayoutItem* item, int deltaRow, int deltaCol)
{
    if (item)
    {
        int index = gridLayout->indexOf(item);

        int row, col, rowSpan, colSpan;
        gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

        int newRow = row + deltaRow;
        if (newRow < 0)
            newRow = 0;

        int newCol = col + deltaCol;
        if (newCol < 0)
            newCol = 0;

        gridLayout->removeItem(item);
        gridLayout->addItem(item, newRow, newCol, rowSpan, colSpan, Qt::AlignCenter);

        setRowColSizes();

        emit itemRowColChanged(item->widget(), newRow, newCol);
    }

}



int GridWidget::countItemsInRow(int row, int colMax)
{
    int numItems = 0;

    QLayoutItem* prevItem = nullptr;

    for (int col = 0; col <= colMax; col++)
    {
        QLayoutItem* item = gridLayout->itemAtPosition(row, col);
        if (item && item != prevItem)
        {
            prevItem = item;
            numItems++;
        }
    }

    return numItems;
}



int GridWidget::countItemsInCol(int col, int rowMax)
{
    int numItems = 0;

    QLayoutItem* prevItem = nullptr;

    for (int row = 0; row <= rowMax; row++)
    {
        QLayoutItem* item = gridLayout->itemAtPosition(row, col);
        if (item && item != prevItem)
        {
            prevItem = item;
            numItems++;
        }
    }

    return numItems;
}



void GridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton)
    {
        int sourceIndex = itemIndex(event->position().toPoint());

        if (sourceIndex >= 0)
        {
            QWidget* widget = gridLayout->itemAt(sourceIndex)->widget();

            QPoint offset = event->position().toPoint() - widget->pos();

            dragPoint = widget->pos();

            QPixmap pixmap = widget->grab();

            QByteArray itemData;
            QDataStream dataStream(&itemData, QIODevice::WriteOnly);
            dataStream << sourceIndex << offset << widget->size();

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-dnditemdata", itemData);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmap);
            drag->setHotSpot(offset);
            drag->exec(Qt::MoveAction);
        }
    }
}



void GridWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        if (!sourceItem)
        {
            QByteArray itemData = event->mimeData()->data("application/x-dnditemdata");
            QDataStream dataStream(&itemData, QIODevice::ReadOnly);

            int sourceIndex;
            dataStream >> sourceIndex;
            dataStream >> sourceOffset;
            dataStream >> sourceSize;

            sourceItem = gridLayout->itemAt(sourceIndex);
        }

        event->accept();
        return;
    }

    event->ignore();
}



void GridWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        QPoint eventPos = event->position().toPoint();
        QPoint newDragPoint = eventPos - sourceOffset;
        QPoint deltaDrag = newDragPoint - dragPoint;

        int sourceIndex = gridLayout->indexOf(sourceItem);

        int sourceRow, sourceCol, sourceRowSpan, sourceColSpan;
        gridLayout->getItemPosition(sourceIndex, &sourceRow, &sourceCol, &sourceRowSpan, &sourceColSpan);

        int deltaRow = deltaDrag.y() / minHeight;
        int deltaCol = deltaDrag.x() / minWidth;

        // Check if target cell is empty

        bool targetEmpty = true;
        for (int rowSpan = 0; rowSpan < sourceRowSpan; rowSpan++)
        {
            for (int colSpan = 0; colSpan < sourceColSpan; colSpan++)
            {
                QLayoutItem* item = gridLayout->itemAtPosition(sourceRow + rowSpan + deltaRow, sourceCol + colSpan + deltaCol);
                if (item && item != sourceItem)
                    targetEmpty = false;
            }
        }

        if (targetEmpty)
        {
            if (qAbs(deltaCol) > 0)
            {
                moveItem(sourceItem, 0, deltaCol);
                dragPoint.setX(newDragPoint.x());
            }

            if (qAbs(deltaRow) > 0)
            {
                moveItem(sourceItem, deltaRow, 0);
                dragPoint.setY(newDragPoint.y());
            }

            event->accept();
            return;
        }
        else
        {
            event->ignore();
            return;
        }
    }

    event->ignore();
}



void GridWidget::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-dnditemdata") && event->source() == this)
    {
        sourceItem = nullptr;

        event->accept();
        return;
    }

    event->ignore();
}
