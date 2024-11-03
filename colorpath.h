#ifndef COLORPATH_H
#define COLORPATH_H



#include <QList>
#include <QPoint>



class ColorPath
{
public:
    ColorPath(QPoint source, int maxPoints);

    void setSource(QPoint source) { source_ = source; }
    QPoint source() { return source_; }

    void setMaxNumPoints(int maxPoints) { maxNumPoints = maxPoints; }

    void addPoint(float red, float green, float blue);

    void clear();

    QList<float> lines() { return lines_; }
    int linesSize() { return lines_.size(); }
    bool linesEmpty() { return lines_.isEmpty(); }

private:
    QPoint source_;
    QList<float> redPoints_, greenPoints_, bluePoints_;
    QList<float> lines_;
    int numPoints = 0;
    int maxNumPoints;
};

#endif // COLORPATH_H
