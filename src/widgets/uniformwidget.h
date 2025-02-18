#ifndef UNIFORMWIDGET_H
#define UNIFORMWIDGET_H



#include "../parameter.h"
#include "parameterwidget.h"
#include "focuswidgets.h"

#include <cmath>

#include <QGridLayout>
#include <QStackedLayout>
#include <QGroupBox>



enum class LayoutFormat {
    Column = 0,
    Row = 1,
    Grid = 2,
    Stacked = 3
};



template <typename T>
class UniformParameterWidget : public ParameterWidget<T>
{
public:
    UniformParameterWidget(UniformParameter<T>* theUniformParameter, QObject* parent = nullptr) :
        ParameterWidget<T>(parent),
        mUniformParameter { theUniformParameter }
    {
        ParameterWidget<T>::mGroupBox = new QGroupBox(mUniformParameter->name());
        ParameterWidget<T>::mGroupBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        // Set up line edits

        foreach (Number<T>* number, mUniformParameter->numbers())
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

            QValidator* validator = nullptr;

            if (std::is_same<T, float>::value)
                validator = new QDoubleValidator(number->inf(), number->sup(), 5, lineEdit);
            else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
                validator = new QIntValidator(number->inf(), number->sup(), lineEdit);

            if (validator)
                lineEdit->setValidator(validator);

            lineEdit->setText(QString::number(number->value()));

            mLineEdits.append(lineEdit);
        }

        ParameterWidget<T>::mLastFocusedWidget = mLineEdits[0];
        mLastIndex = 0;

        // Set up layouts

        mStackedLayout = new QStackedLayout;

        mColWidget = new QWidget;
        mColWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mRowWidget = new QWidget;
        mRowWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        mGridWidget = new QWidget;
        mGridWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        setItemsLayouts();
        setDefaultLayoutFormat();
        ParameterWidget<T>::mGroupBox->setLayout(mStackedLayout);

        // Connections

        for (int i = 0; i < mUniformParameter->numbers().size(); i++)
        {
            ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::editingFinished, this, [=, this](){
                if (std::is_same<T, float>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toFloat());
                else if (std::is_same<T, int>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toInt());
                else if (std::is_same<T, unsigned int>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toUInt());
            });

            ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){
                ParameterWidget<T>::mSelectedNumber = mUniformParameter->number(i);
                ParameterWidget<T>::mLastFocusedWidget = mLineEdits[i];
                mLastIndex = i;
            });
            ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusIn, this, &UniformParameterWidget::focusIn);
            ParameterWidget<T>::connect(mLineEdits[i], &FocusLineEdit::focusOut, this, &UniformParameterWidget::focusOut);

            ParameterWidget<T>::connect(mUniformParameter, QOverload<int, QVariant>::of(&Parameter::valueChanged), this, [=, this](int i, QVariant newValue){
                if (std::is_same<T, float>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toFloat()));
                else if (std::is_same<T, int>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toInt()));
                else if (std::is_same<T, unsigned int>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toUInt()));
                mLineEdits[i]->setCursorPosition(0);
            });
        }
    }

    ~UniformParameterWidget()
    {
        //clearLayouts();

        //qDeleteAll(mItemWidgets);

        /*if (mColWidget)
            delete mColWidget;
        if (mRowWidget)
            delete mRowWidget;
        if (mGridWidget)
            delete mGridWidget;*/
    }

    QString name() { return mUniformParameter->name(); }

    void setRow(int i) { mUniformParameter->setRow(i); }
    void setCol(int i) { mUniformParameter->setCol(i); }

    //Number<T>* selectedNumber() { return mSelectedNumber; }

    //FocusLineEdit* lastFocusedWidget() { return mLastFocusedWidget; }

    void setValueFromIndex(int index)
    {
        mUniformParameter->setValueFromIndex(mLastIndex, index);
    }

    void setCurrentStack(int index)
    {
        if (mLayoutFormat == LayoutFormat::Stacked && index < mStackedLayout->count())
            mStackedLayout->setCurrentIndex(index);
    }

    void setLayoutFormat(LayoutFormat format)
    {
        clearLayouts();

        mLayoutFormat = format;

        if (format == LayoutFormat::Column)
        {
            QGridLayout* gridLayout = new QGridLayout;
            gridLayout->setContentsMargins(1, 1, 1, 1);
            gridLayout->setSpacing(0);
            gridLayout->setAlignment(Qt::AlignCenter);

            int row = 0;
            foreach (QWidget* widget, mItemWidgets)
                gridLayout->addWidget(widget, row++, 0, Qt::AlignCenter);

            mColWidget->setLayout(gridLayout);
            mStackedLayout->addWidget(mColWidget);
        }
        else if (format == LayoutFormat::Row)
        {
            QGridLayout* gridLayout = new QGridLayout;
            gridLayout->setContentsMargins(1, 1, 1, 1);
            gridLayout->setSpacing(0);
            gridLayout->setAlignment(Qt::AlignCenter);

            int col = 0;
            foreach (QWidget* widget, mItemWidgets)
                gridLayout->addWidget(widget, 0, col++, Qt::AlignCenter);

            mRowWidget->setLayout(gridLayout);
            mStackedLayout->addWidget(mRowWidget);
        }
        else if (format == LayoutFormat::Grid)
        {
            QGridLayout* gridLayout = new QGridLayout;
            gridLayout->setContentsMargins(1, 1, 1, 1);
            gridLayout->setSpacing(0);
            gridLayout->setAlignment(Qt::AlignCenter);

            int dim = static_cast<int>(ceil(sqrt(mUniformParameter->numItems())));

            int row = 0;
            int col = 0;

            foreach (QWidget* widget, mItemWidgets)
            {
                gridLayout->addWidget(widget, row, col, Qt::AlignCenter);

                col++;
                if (col == dim)
                {
                    row++;
                    col = 0;
                }
            }

            mGridWidget->setLayout(gridLayout);
            mStackedLayout->addWidget(mGridWidget);
        }
        else if (format == LayoutFormat::Stacked)
        {
            foreach (QWidget* widget, mItemWidgets)
                mStackedLayout->addWidget(widget);
        }

        mStackedLayout->setCurrentIndex(0);
    }

private:
    UniformParameter<T>* mUniformParameter;
    LayoutFormat mLayoutFormat;
    QList<FocusLineEdit*> mLineEdits;
    QStackedLayout* mStackedLayout;
    QList<QWidget*> mItemWidgets;
    QWidget* mColWidget;
    QWidget* mRowWidget;
    QWidget* mGridWidget;
    //FocusLineEdit* mLastFocusedWidget;
    int mLastIndex;

    void setItemsLayouts()
    {
        // Set each item on its grid layout

        QGridLayout* itemGridLayout = nullptr;

        QPair<int, int> colsRowsPerItem = mUniformParameter->colsRowsPerItem();

        int row = 0;
        int col = 0;

        foreach (FocusLineEdit* lineEdit, mLineEdits)
        {
            if (row == 0 && col == 0)
            {
                itemGridLayout = new QGridLayout;
                itemGridLayout->setContentsMargins(1, 1, 1, 1);
                itemGridLayout->setSpacing(0);
                itemGridLayout->setAlignment(Qt::AlignCenter);
            }

            itemGridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

            col++;
            if (col == colsRowsPerItem.first)
            {
                row++;
                col = 0;
            }

            if (row == colsRowsPerItem.second)
            {
                row = 0;
                col = 0;

                QWidget* widget = new QWidget;
                widget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
                widget->setLayout(itemGridLayout);
                mItemWidgets.append(widget);
            }
        }
    }

    void clearLayouts()
    {
        while(mStackedLayout->count() > 0)
        {
            int i = mStackedLayout->count() - 1;
            QWidget* widget = mStackedLayout->widget(i);
            if (widget)
                mStackedLayout->removeWidget(widget);
        }

        removeLayout(mColWidget->layout());
        removeLayout(mRowWidget->layout());
        removeLayout(mGridWidget->layout());
    }

    void removeLayout(QLayout* layout)
    {
        if (layout)
        {
            QLayoutItem* child;
            while ((child = layout->takeAt(0)) != nullptr)
            {
                QWidget* widget = child->widget();
                layout->removeWidget(widget);
                delete child;
            }
        }
    }

    void setDefaultLayoutFormat()
    {
        QPair<int, int> colsRowsPerItem = mUniformParameter->colsRowsPerItem();

        if (mUniformParameter->numItems() <= 1)
        {
            // Any single item
            mLayoutFormat = LayoutFormat::Column;
        }
        else if (colsRowsPerItem.second <= 1)
        {
            // Multiple items with one row
            if (colsRowsPerItem.first <= 1)
                // Multiple items with one row and one column
                mLayoutFormat = LayoutFormat::Grid;
            else
                // Multiple items with one row and more than one column
                mLayoutFormat = LayoutFormat::Column;
        }
        else
        {
            // Multiple items with more than one row
            mLayoutFormat = LayoutFormat::Stacked;
        }

        setLayoutFormat(mLayoutFormat);
    }
};



#endif // UNIFORMWIDGET_H
