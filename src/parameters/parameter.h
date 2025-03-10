/*
*  Copyright 2021 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/



#ifndef PARAMETER_H
#define PARAMETER_H



#include <QObject>
#include <QVariant>



class ImageOperation;



class ParameterSignals : public QObject
{
    Q_OBJECT

public:
    explicit ParameterSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void valueChanged(QVariant value);
    void valueChanged(int i, QVariant value);
    void indexChanged(int index);
};




class Parameter : public ParameterSignals
{
    Q_OBJECT

public:
    Parameter(QString theName, bool isEditable, ImageOperation* theOperation) :
        ParameterSignals(),
        mName { theName },
        mEditable { isEditable },
        mOperation { theOperation }
    {}

    Parameter(const Parameter& parameter) :
        ParameterSignals()
    {
        mName = parameter.mName;
        mEditable = parameter.mEditable;
    }

    QString name() { return mName; }
    void setName(QString theName) { mName = theName; }

    bool editable() { return mEditable; }

    int row() { return mRow; }
    void setRow(int i) { mRow = i; }

    int col() { return mCol; }
    void setCol(int i) { mCol = i; }

    void setUpdateOperation(bool set){ mUpdate = set; }

    void setOperation(ImageOperation* operation) { mOperation = operation; }

public slots:
    void setEditable(bool set) { mEditable = set; }

protected:
    QString mName;
    bool mEditable;
    int mRow, mCol;
    bool mUpdate = true;
    ImageOperation* mOperation;
};



#endif // PARAMETER_H
