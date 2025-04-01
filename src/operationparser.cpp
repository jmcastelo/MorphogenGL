


#include "operationparser.h"

#include <QFile>



OperationParser::OperationParser(){}



void OperationParser::write(ImageOperation *operation, QString filename, bool writeIds)
{
    QFile outFile(filename);
    outFile.open(QIODevice::WriteOnly);

    QXmlStreamWriter mStream;
    mStream.setDevice(&outFile);
    mStream.setAutoFormatting(true);

    mStream.writeStartDocument();

    mStream.writeStartElement("fosforo");

    writeOperation(operation, mStream, writeIds);

    mStream.writeEndElement();

    mStream.writeEndDocument();
}



void OperationParser::writeOperation(ImageOperation *operation, QXmlStreamWriter& stream, bool writeIds)
{
    // Operation

    stream.writeStartElement("operation");

    stream.writeAttribute("name", operation->name());
    stream.writeAttribute("enabled", QString::number(operation->isEnabled()));

    // In attributes

    stream.writeStartElement("input_attributes");

    stream.writeStartElement("position");
    stream.writeCharacters(operation->posInAttribName());
    stream.writeEndElement();

    stream.writeStartElement("texture");
    stream.writeCharacters(operation->texInAttribName());
    stream.writeEndElement();

    stream.writeEndElement();

    // Parameters

    writeParameters<float>(operation, stream, writeIds);
    writeParameters<int>(operation, stream, writeIds);
    writeParameters<unsigned int>(operation, stream, writeIds);

    writeMat4Parameters(operation, stream, writeIds);

    writeOptionsParameters(operation, stream);

    // Shaders

    stream.writeStartElement("vertex_shader");
    stream.writeCharacters(operation->vertexShader());
    stream.writeEndElement();

    stream.writeStartElement("fragment_shader");
    stream.writeCharacters(operation->fragmentShader());
    stream.writeEndElement();

    stream.writeEndElement();
}



void OperationParser::writeMat4Parameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds)
{
    foreach (auto parameter, operation->mat4UniformParameters())
    {
        stream.writeStartElement("parameter");

        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("editable", QString::number(parameter->editable()));
        stream.writeAttribute("row", QString::number(parameter->row()));
        stream.writeAttribute("column", QString::number(parameter->col()));

        stream.writeStartElement("uniform");

        stream.writeAttribute("name", parameter->uniformName());
        stream.writeAttribute("type", QString::number(parameter->uniformType()));
        stream.writeAttribute("mat4_type", QString::number(parameter->typeIndex()));

        if (!parameter->empty())
        {
            QList<QString> names = parameter->numberNames();

            int i = 0;

            foreach (auto number, parameter->numbers())
            {
                stream.writeStartElement("number");

                if (writeIds)
                    stream.writeAttribute("id", number->id().toString());
                stream.writeAttribute("name", names[i++]);
                stream.writeAttribute("inf", QString::number(number->inf()));
                stream.writeAttribute("sup", QString::number(number->sup()));
                stream.writeAttribute("min", QString::number(number->min()));
                stream.writeAttribute("max", QString::number(number->max()));

                stream.writeCharacters(QString::number(number->value()));

                stream.writeEndElement();
            }
        }

        stream.writeEndElement();

        QMap<QString, QList<float>> presets = parameter->presets();

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



void OperationParser::writeOptionsParameters(ImageOperation* operation, QXmlStreamWriter& stream)
{
    foreach (auto parameter, operation->optionsParameters<GLenum>())
    {
        stream.writeStartElement("parameter");

        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("editable", QString::number(parameter->editable()));
        stream.writeAttribute("row", QString::number(parameter->row()));
        stream.writeAttribute("column", QString::number(parameter->col()));

        QList<QString> names = parameter->valueNames();

        int i = 0;

        foreach (auto value, parameter->values())
        {
            stream.writeStartElement("value");
            stream.writeAttribute("name", names[i++]);
            stream.writeCharacters(QString::number(value));
            stream.writeEndElement();
        }

        stream.writeEndElement();
    }
}
