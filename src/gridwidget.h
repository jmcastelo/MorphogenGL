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

signals:
    void itemRowColChanged(QWidget* widget, int row, int column);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QGridLayout* gridLayout;
    QPoint dragStartPosition;
    QLayoutItem* sourceItem;
    QWidget* placeholder;
    QLayoutItem* placeholderItem = nullptr;
    int oldTargetIndex = -1;

    int itemIndex(QPoint pos);
    void moveItemsUp(int row, int col);
    void moveItemsDown(int row, int col);
    void swapItems(QLayoutItem* item1, QLayoutItem* item2);
};



#endif // GRIDWIDGET_H
