#ifndef GRIDWIDGET_H
#define GRIDWIDGET_H



#include <QWidget>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>



class GridWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GridWidget(QWidget *parent = nullptr);

    void addWidget(QWidget* widget, int row, int column);

signals:
    void itemIndicesChanged(QWidget* widget, int row, int column);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    QGridLayout* gridLayout;
    int targetIndex = -1;

    int itemIndex(QPoint pos);
};



#endif // GRIDWIDGET_H
