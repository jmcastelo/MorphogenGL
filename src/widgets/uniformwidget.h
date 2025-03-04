#ifndef UNIFORMWIDGET_H
#define UNIFORMWIDGET_H



#include "parameterwidget.h"
#include "../parameters/uniformparameter.h"
#include "layoutformat.h"

#include <cmath>

#include <QGridLayout>
#include <QStackedLayout>
#include <QGroupBox>



template <typename T>
class UniformParameterWidget : public ParameterWidget<T>
{
public:
    UniformParameterWidget(UniformParameter<T>* theUniformParameter, QObject* parent = nullptr);

    QString name();
    void setName(QString theName);

    void setMin(T theMin);
    void setMax(T theMax);
    void setInf(T theInf);
    void setSup(T theSup);

    void setRow(int i);
    void setCol(int i);

    void setValueFromIndex(int index);

    void setCurrentStack(int index);

    LayoutFormat layoutFormat();
    void setLayoutFormat(LayoutFormat format);

private:
    UniformParameter<T>* mUniformParameter;
    LayoutFormat mLayoutFormat;
    QList<FocusLineEdit*> mLineEdits;
    QStackedLayout* mStackedLayout;
    QList<QWidget*> mItemWidgets;
    QWidget* mColWidget;
    QWidget* mRowWidget;
    QWidget* mGridWidget;
    int mLastIndex;

    void setItemsLayouts();
    void clearLayouts();
    void removeLayout(QLayout* layout);
    void setDefaultLayoutFormat();
};



#endif // UNIFORMWIDGET_H
