#include "gridwidget.h"

#include <QDrag>
#include <QPixmap>
#include <QMimeData>
#include <QApplication>



GridWidget::GridWidget(QWidget *parent) :
    QWidget { parent }
{
    gridLayout = new QGridLayout;
    gridLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    gridLayout->setContentsMargins(margin, margin, margin, margin);
    gridLayout->setSpacing(spacing);

    setAcceptDrops(false);
    setLayout(gridLayout);
}



void GridWidget::addWidget(QWidget* widget, int row, int column)
{
    // Widget added to cell with row and column coordinates

    gridLayout->addWidget(widget, (row < 0) ? getMaxRow() + 1 : row, (column < 0) ? getMaxCol() + 1 : column, Qt::AlignCenter);

    // Widget must be shown to be able to compute its geometry

    widget->show();
}



void GridWidget::removeWidget(QWidget* widget)
{
    gridLayout->removeWidget(widget);
    widget->hide();
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
}



void GridWidget::computeMinWidgetSize()
{
    minWidgetWidth = -1;
    minWidgetHeight = -1;

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QRect itemRect = gridLayout->itemAt(index)->widget()->geometry();

        if (minWidgetWidth < 0 || itemRect.width() < minWidgetWidth) {
            minWidgetWidth = itemRect.width();
        }

        if (minWidgetHeight < 0 || itemRect.height() < minWidgetHeight) {
            minWidgetHeight = itemRect.height();
        }
    }
}



int GridWidget::getRowSpan(QWidget* widget)
{
    QRect widgetRect = widget->geometry();
    int rowSpan = widgetRect.height() / minWidgetHeight;
    return rowSpan;
}



int GridWidget::getColSpan(QWidget* widget)
{
    QRect widgetRect = widget->geometry();
    int colSpan = widgetRect.width() / minWidgetWidth;
    return colSpan;
}



void GridWidget::optimizeLayout()
{
    computeMinWidgetSize();

    // Set row and column spans in proportion to minimum widget size

    itemCoords.clear();

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QLayoutItem* item = gridLayout->itemAt(index);

        int row, col, rSpan, cSpan;
        gridLayout->getItemPosition(index, &row, &col, &rSpan, &cSpan);

        itemCoords.insert(item, QList<int>{ row, col, getRowSpan(item->widget()), getColSpan(item->widget()) });
    }

    // Compute new positions (row, col) considering spans

    QList<QLayoutItem*> computedItems;

    for (int col = 0; col <= getMaxCol(); col++)
    {
        int nextRow = 0;

        for (int row = 0; row <= getMaxRow(); row++)
        {
            QLayoutItem* item = gridLayout->itemAtPosition(row, col);
            if (item && !computedItems.contains(item))
            {
                itemCoords[item][0] = (row <= nextRow) ? nextRow : row;
                nextRow = itemCoords[item][0] + itemCoords[item][2];

                computedItems.append(item);
            }
        }
    }

    computedItems.clear();

    for (int row = 0; row <= getMaxRow(); row++)
    {
        int nextCol = 0;

        for (int col = 0; col <= getMaxCol(); col++)
        {
            QLayoutItem* item = gridLayout->itemAtPosition(row, col);
            if (item && !computedItems.contains(item))
            {
                itemCoords[item][1] = (col <= nextCol) ? nextCol : col;
                nextCol = itemCoords[item][1] + itemCoords[item][3];

                computedItems.append(item);
            }
        }
    }

    // Relocate items

    hide();

    for (auto [item, coords] : itemCoords.asKeyValueRange())
    {
        gridLayout->removeItem(item);
        gridLayout->addItem(item, coords[0], coords[1], coords[2], coords[3], Qt::AlignCenter);

        emit itemRowColChanged(item->widget(), coords[0], coords[1]);
    }

    show();

    setRowColSizes();

    adjustSize();
}



void GridWidget::setRowColSizes()
{
    int maxRow = getMaxRow();
    int maxCol = getMaxCol();

    for (int row = 0; row <= maxRow; row++)
        gridLayout->setRowMinimumHeight(row, minWidgetHeight);

    int maxGridRow = gridLayout->rowCount() - 1;
    for (int row = maxRow + 1; row <= maxGridRow; row++)
        gridLayout->setRowMinimumHeight(row, 0);

    for (int col = 0; col <= maxCol; col++)
        gridLayout->setColumnMinimumWidth(col, minWidgetWidth);

    int maxGridCol = gridLayout->columnCount() - 1;
    for (int col = maxCol + 1; col <= maxGridCol; col++)
        gridLayout->setColumnMinimumWidth(col, 0);
}



int GridWidget::getMaxRow()
{
    int maxRow = -1;

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QLayoutItem* item = gridLayout->itemAt(index);
        if (!item->isEmpty() && item->widget()->isVisible())
        {
            int row, col, rowSpan, colSpan;
            gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

            if (row + rowSpan - 1 > maxRow)
                maxRow = row + rowSpan - 1;
        }
    }

    return maxRow;
}



int GridWidget::getMaxCol()
{
    int maxCol = -1;

    for (int index = 0; index < gridLayout->count(); index++)
    {
        QLayoutItem* item = gridLayout->itemAt(index);
        if (!item->isEmpty() && item->widget()->isVisible())
        {
            int row, col, rowSpan, colSpan;
            gridLayout->getItemPosition(index, &row, &col, &rowSpan, &colSpan);

            if (col + colSpan - 1 > maxCol) {
                maxCol = col + colSpan - 1;
            }
        }
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
        if (newRow < 0) {
            newRow = 0;
        }

        int newCol = col + deltaCol;
        if (newCol < 0) {
            newCol = 0;
        }

        gridLayout->removeItem(item);
        gridLayout->addItem(item, newRow, newCol, rowSpan, colSpan, Qt::AlignCenter);

        item->widget()->show();

        setRowColSizes();

        adjustSize();

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



int GridWidget::itemIndex(QPoint pos)
{
    for (int index = 0; index < gridLayout->count(); index++)
        if (gridLayout->itemAt(index)->widget()->geometry().contains(pos))
            return index;

    return -1;
}



void GridWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->buttons() == Qt::LeftButton && acceptDrops())
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
            dataStream << sourceIndex << offset;

            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-dnditemdata", itemData);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->setPixmap(pixmap);
            drag->setHotSpot(offset);
            drag->exec(Qt::MoveAction);
        }

        event->accept();
        return;
    }

    event->ignore();
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

        int deltaRow = deltaDrag.y() / minWidgetHeight;
        int deltaCol = deltaDrag.x() / minWidgetWidth;

        // Check if target cell is empty

        bool targetEmpty = true;
        for (int rowSpan = 0; rowSpan < sourceRowSpan; rowSpan++)
        {
            for (int colSpan = 0; colSpan < sourceColSpan; colSpan++)
            {
                QLayoutItem* item = gridLayout->itemAtPosition(sourceRow + rowSpan + deltaRow, sourceCol + colSpan + deltaCol);
                if (item && item != sourceItem) {
                    targetEmpty = false;
                }
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
