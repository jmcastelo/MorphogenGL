#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H



#include <QFrame>
#include <QWidget>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>



class GridWidget : public QFrame
{
    Q_OBJECT

public:
    explicit GridWidget(QWidget *parent = nullptr);

    void addWidget(QWidget* widget, int row, int column);
    void removeWidget(QWidget* widget);
    void clear();
    void optimizeLayout();
    void setEditMode(bool editMode);

signals:
    void itemRowColChanged(QWidget* widget, int row, int column);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QGridLayout* gridLayout;

    int margin = 10;
    int spacing = 10;

    int minWidgetWidth = -1;
    int minWidgetHeight = -1;

    QMap<QLayoutItem*, QList<int>> itemCoords;

    QLayoutItem* sourceItem = nullptr;
    QPoint sourceOffset;
    QPoint dragPoint;

    void computeMinWidgetSize();

    int getRowSpan(QWidget* widget);
    int getColSpan(QWidget* widget);

    void setRowColSizes();

    int getMaxRow();
    int getMaxCol();

    void lowestFreeCell(int& lowestRow, int& lowestCol, int row, int col);

    int countItemsInRow(int row, int colMax);
    int countItemsInCol(int col, int rowCount);

    int itemIndex(QPoint pos);

    void moveItem(QLayoutItem* item, int deltaRow, int deltaCol);
};



#endif // GRIDWIDGET_H
