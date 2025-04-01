#ifndef OPERATIONPARSER_H
#define OPERATIONPARSER_H



#include "imageoperation.h"

#include <QXmlStreamWriter>


class OperationParser
{
public:
    OperationParser();

    void write(ImageOperation* operation, QString filename, bool writeIds);

    void writeOperation(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds);

    void writeMat4Parameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds);

    void writeOptionsParameters(ImageOperation* operation, QXmlStreamWriter& stream);

    template<typename T>
    void writeParameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds)
    {
        foreach (auto parameter, operation->uniformParameters<T>())
        {
            stream.writeStartElement("parameter");

            stream.writeAttribute("name", parameter->name());
            stream.writeAttribute("editable", QString::number(parameter->editable()));
            stream.writeAttribute("row", QString::number(parameter->row()));
            stream.writeAttribute("column", QString::number(parameter->col()));

            stream.writeStartElement("uniform");

            stream.writeAttribute("name", parameter->uniformName());
            stream.writeAttribute("type", QString::number(parameter->uniformType()));
            stream.writeAttribute("numitems", QString::number(parameter->numItems()));

            if (!parameter->empty())
            {
                foreach (auto number, parameter->numbers())
                {
                    stream.writeStartElement("number");

                    if (writeIds)
                        stream.writeAttribute("id", number->id().toString());
                    stream.writeAttribute("inf", QString::number(number->inf()));
                    stream.writeAttribute("sup", QString::number(number->sup()));
                    stream.writeAttribute("min", QString::number(number->min()));
                    stream.writeAttribute("max", QString::number(number->max()));

                    stream.writeCharacters(QString::number(number->value()));

                    stream.writeEndElement();
                }
            }

            stream.writeEndElement();

            QMap<QString, QList<T>> presets = parameter->presets();

            if (!presets.empty())
            {
                stream.writeStartElement("presets");

                for (auto [name, values]: presets.asKeyValueRange())
                {
                    stream.writeStartElement(name);

                    foreach (auto value, values)
                    {
                        stream.writeStartElement("value");
                        stream.writeCharacters(QString::number(value));
                        stream.writeEndElement();
                    }

                    stream.writeEndElement();
                }

                stream.writeEndElement();
            }

            stream.writeEndElement();
        }
    }
};



#endif // OPERATIONPARSER_H
