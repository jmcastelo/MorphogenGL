#ifndef NUMBER_H
#define NUMBER_H



#include <QObject>
#include <QUuid>
#include <QVariant>



class NumberSignals : public QObject
{
    Q_OBJECT

public:
    explicit NumberSignals(QObject *parent = nullptr) : QObject(parent) {}

signals:
    void valueChanged(QVariant value);
    void indexChanged(int index);
    void indexMaxChanged(int indexMax);
    void minChanged();
    void maxChanged();
    void deleting();
    void linked(bool set);
};



template <class T>
class Number : public NumberSignals
{
public:
    Number(T theValue, T theMin, T theMax, T theInf, T theSup) :
        mValue { theValue },
        mMin { theMin },
        mMax { theMax },
        mInf { theInf },
        mSup { theSup }
    {
        mId = QUuid::createUuid();
    }

    Number(QUuid theId, T theValue, T theMin, T theMax, T theInf, T theSup) :
        mId { theId },
        mValue { theValue },
        mMin { theMin },
        mMax { theMax },
        mInf { theInf },
        mSup { theSup }
    {}

    Number(const Number<T>& number) :
        NumberSignals()
    {
        mId = number.mId;
        mValue = number.mValue;
        mMin = number.mMin;
        mMax = number.mMax;
        mInf = number.mInf;
        mSup = number.mSup;
        mIndexMax = number.mIndexMax;
    }

    ~Number()
    {
        emit deleting();
    }

    QUuid id() const
    {
        return mId;
    }

    void setLimits()
    {
        if (mInf > mSup)
            mInf = mSup;

        if (mMin < mInf)
        {
            mMin = mInf;
            emit minChanged();
        }

        if (mMax > mSup)
        {
            mMax = mSup;
            emit maxChanged();
        }

        if (mMin > mMax)
        {
            mMin = mMax;
            emit minChanged();
        }

        if (mValue < mInf)
        {
            mValue = mInf;
            emit valueChanged(QVariant(mValue));
        }

        if (mValue > mSup)
        {
            mValue = mSup;
            emit valueChanged(QVariant(mValue));
        }

        if (mMin > mValue)
        {
            mMin = mValue;
            emit minChanged();
        }

        if (mMax < mValue)
        {
            mMax = mValue;
            emit maxChanged();
        }
    }

    void setMin(T theMin)
    {
        mMin = theMin;
        setLimits();
        setIndex();
    }

    T min() const
    {
        return mMin;
    }

    void setMax(T theMax)
    {
        mMax = theMax;
        setLimits();
        setIndex();
    }

    T max() const
    {
        return mMax;
    }

    void setInf(T theInf)
    {
        mInf = theInf;
        setLimits();
    }

    T inf() const
    {
        return mInf;
    }

    void setSup(T theSup)
    {
        mSup = theSup;
        setLimits();
    }

    T sup() const
    {
        return mSup;
    }

    void setValue(T theValue)
    {
        mValue = theValue;
        setLimits();
        setIndex();
        emit valueChanged(QVariant(mValue));
    }

    void setValueFromIndex(int theIndex)
    {
        T value = static_cast<T>(mMin + (mMax - mMin) * static_cast<float>(theIndex) / static_cast<float>(mIndexMax));
        setValue(value);
    }

    T value() const
    {
        return mValue;
    }

    void setIndex()
    {
        int index = static_cast<int>(mIndexMax * static_cast<float>(mValue - mMin) / static_cast<float>(mMax - mMin));
        emit indexChanged(index);
    }

    int index() const
    {
        return static_cast<int>(mIndexMax * static_cast<float>(mValue - mMin) / static_cast<float>(mMax - mMin));
    }

    int indexMax() const
    {
        return mIndexMax;
    }

    void setIndexMax(int theIndexMax)
    {
        mIndexMax = theIndexMax;
        emit indexMaxChanged(mIndexMax);
    }

    bool midiLinked() const
    {
        return mMidiLinked;
    }

    void setMidiLinked(bool set)
    {
        mMidiLinked = set;
        emit linked(mMidiLinked);
    }

private:
    QUuid mId;
    T mValue, mMin, mMax, mInf, mSup;
    int mIndexMax = 100'000;
    bool mMidiLinked = false;
};



#endif // NUMBER_H
