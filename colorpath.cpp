#include "colorpath.h"



ColorPath::ColorPath(QPoint source, int maxPoints) :
    source_ { source }
{
    numPoints = 0;

    if (maxPoints < 2)
        maxNumPoints = 2;
    else
        maxNumPoints = maxPoints;
}



void ColorPath::addPoint(float red, float green, float blue)
{
    // Line segment

    if (numPoints > 0)
    {
        lines_.append(redPoints_.last());
        lines_.append(greenPoints_.last());
        lines_.append(bluePoints_.last());

        lines_.append(red);
        lines_.append(green);
        lines_.append(blue);
    }

    // RGB Points

    redPoints_.append(red);
    greenPoints_.append(green);
    bluePoints_.append(blue);

    numPoints++;

    // Keep number of points within range

    if (numPoints > maxNumPoints)
    {
        redPoints_.remove(0, numPoints - maxNumPoints);
        greenPoints_.remove(0, numPoints - maxNumPoints);
        bluePoints_.remove(0, numPoints - maxNumPoints);

        lines_.remove(0, 6 * (numPoints - maxNumPoints));

        numPoints = maxNumPoints;
    }
}



void ColorPath::clear()
{
    redPoints_.clear();
    greenPoints_.clear();
    bluePoints_.clear();

    lines_.clear();

    numPoints = 0;
}
