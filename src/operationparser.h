#ifndef OPERATIONPARSER_H
#define OPERATIONPARSER_H



#include "imageoperation.h"

#include <QXmlStreamWriter>
#include <QXmlStreamReader>



class OperationParser
{
public:
    OperationParser();

    void write(ImageOperation* operation, QString filename, bool writeIds);
    void read(ImageOperation* operation, QString filename, bool readIds);

    void writeOperation(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds);
    void readOperation(ImageOperation* operation, QXmlStreamReader& stream, bool readIds);

    void writeMat4Parameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds);
    void readMat4Parameters(ImageOperation* operation, QXmlStreamReader& stream, bool readIds);

    void writeOptionsParameters(ImageOperation* operation, QXmlStreamWriter& stream);
    void readOptionsParameters(ImageOperation* operation, QXmlStreamReader& stream);

    template<typename T>
    void writeParameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds);

    template<typename T>
    void readParameters(ImageOperation* operation, QXmlStreamReader& stream, bool readIds);
};



#endif // OPERATIONPARSER_H
