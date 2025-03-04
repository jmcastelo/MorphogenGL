#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H



#include <QWidget>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>



class GridWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GridWidget(QWidget *parent = nullptr);

    void addWidget(QWidget* widget, int row, int column);
    void clear();
    void optimizeLayout();

signals:
    void itemRowColChanged(QWidget* widget, int row, int column);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QGridLayout* gridLayout;
    int margin = 20;
    int spacing = 10;
    int minWidth, minHeight;
    QMap<QLayoutItem*, QList<int>> itemCoords;
    QLayoutItem* sourceItem = nullptr;
    QPoint sourceOffset;
    QSize sourceSize;
    QPoint dragPoint;

    void setRowColSizes();

    int getMaxRow();
    int getMaxCol();

    int countItemsInRow(int row, int colMax);
    int countItemsInCol(int col, int rowCount);

    int itemIndex(QPoint pos);

    void moveItem(QLayoutItem* item, int deltaRow, int deltaCol);
};



#endif // GRIDWIDGET_H
