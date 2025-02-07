#ifndef UNIFORMWIDGET_H
#define UNIFORMWIDGET_H



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
class UniformParameterWidget : public ParameterWidget
{
    Q_OBJECT

public:
    UniformParameterWidget(UniformParameter<T>* theUniformParameter, QWidget* parent = nullptr) :
        ParameterWidget(parent),
        mUniformParameter { theUniformParameter }
    {
        mGroupBox(mUniformParameter->name());

        mGridLayout = new QGridLayout;

        int row = 0;
        int col = 0;

        for (Number<T>* number : mUniformParameter->numbers())
        {
            FocusLineEdit* lineEdit = new FocusLineEdit;
            lineEdit->setFixedWidth(75);

            QValidator* validator;
            if (std::is_same<T, float>::value)
                validator = new QDoubleValidator(number->inf(), number->sup(), 5, lineEdit);
            else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value)
                validator = new QIntValidator(number->inf(), number->sup(), lineEdit);

            lineEdit->setValidator(validator);
            lineEdit->setText(QString::number(number->value()));

            mGridLayout->addWidget(lineEdit, row, col, Qt::AlignCenter);

            col++;
            if (col == 3)
            {
                row++;
                col = 0;
            }

            mLineEdits.push_back(lineEdit);
        }

        for (size_t i = 0; i < mUniformParameter->numbers().size(); i++)
        {
            connect(mLineEdits[i], &FocusLineEdit::editingFinished, this, [=, this](){
                if (std::is_same<T, float>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toFloat());
                else if (std::is_same<T, int>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toInt());
                else if (std::is_same<T, unsigned int>::value)
                    mUniformParameter->setValue(i, mLineEdits[i]->text().toUInt());
            });

            connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ focusedWidget = mLineEdits[i]; });
            connect(mLineEdits[i], &FocusLineEdit::focusIn, this, [=, this](){ emit focusIn(mUniformParameter->numbers(i)); });
            connect(mLineEdits[i], &FocusLineEdit::focusIn, this, QOverload<>::of(&ParameterWidget::focusIn));
            connect(mLineEdits[i], &FocusLineEdit::focusOut, this, &ParameterWidget::focusOut);

            connect(mUniformParameter, &Parameter::valueChanged, this, [=, this](int i, QVariant newValue){
                if (std::is_same<T, float>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toFloat()));
                else if (std::is_same<T, int>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toInt()));
                else if (std::is_same<T, unsigned int>::value)
                    mLineEdits[i]->setText(QString::number(newValue.toUInt()));

                emit focusIn();
            });
        }

        focusedWidget = mLineEdits[0];
    }

    QString name() { return mUniformParameter->name(); }

    QGroupBox* groupBox(){ return mGroupBox; }

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
                itemGridLayout = new QGridLayout;

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
                mItemGridLayouts.append(itemGridLayout);
            }
        }
    }

    void setLayoutFormat(LayoutFormat format)
    {
        mLayoutFormat = format;

        if (format == LayoutFormat::Column)
        {
            QGridLayout* gridLayout = new QGridLayout;

            int row = 0;
            foreach (QGridLayout* itemLayout, mItemGridLayouts)
                gridLayout->addLayout(itemLayout, row++, 0, Qt::AlignCenter);

            setLayout(gridLayout);
        }
        else if (format == LayoutFormat::Row)
        {
            QGridLayout* gridLayout = new QGridLayout;

            int col = 0;
            foreach (QGridLayout* itemLayout, mItemGridLayouts)
                gridLayout->addLayout(itemLayout, 0, col, Qt::AlignCenter);

            setLayout(gridLayout);
        }
        else if (format == LayoutFormat::Grid)
        {
            QGridLayout* gridLayout = new QGridLayout;

            int dim = static_cast<int>(ceil(sqrt(mUniformParameter->numItems())));

            int row = 0;
            int col = 0;

            foreach (QGridLayout* itemLayout, mItemGridLayouts)
            {
                gridLayout->addLayout(itemLayout, row, col, Qt::AlignCenter);

                col++;
                if (col == dim)
                {
                    row++;
                    col = 0;
                }
            }
        }
        else if (format == LayoutFormat::Stacked)
        {
            QStackedLayout* stackedLayout;

            foreach (QGridLayout* itemLayout, mItemGridLayouts)
            {
                QWidget* widget = new QWidget;
                widget->setLayout(itemLayout);
                stackedLayout->addWidget(widget);
            }

            setLayout(stackedLayout);
        }
    }

private:
    UniformParameter<T>* mUniformParameter;
    QList<FocusLineEdit*> mLineEdits;
    QGroupBox* mGroupBox;
    QGridLayout* mGridLayout;
    QList<QGridLayout*> mItemGridLayouts;
    LayoutFormat mLayoutFormat;
};



#endif // UNIFORMWIDGET_H
