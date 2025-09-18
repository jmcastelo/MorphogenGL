


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

    outFile.close();
}



bool OperationParser::read(ImageOperation* operation, QString filename, bool readIds)
{
    QFile inFile(filename);
    inFile.open(QIODevice::ReadOnly);

    QXmlStreamReader mStream;
    mStream.setDevice(&inFile);

    bool success = false;

    if (mStream.readNextStartElement() && mStream.name() == "fosforo")
        success = readOperation(operation, mStream, readIds);

    if (mStream.tokenType() == QXmlStreamReader::Invalid)
        mStream.readNext();

    if (mStream.hasError())
        mStream.raiseError();

    inFile.close();

    return success;
}



void OperationParser::writeOperation(ImageOperation *operation, QXmlStreamWriter& stream, bool writeIds)
{
    // Operation

    stream.writeStartElement("operation");

    stream.writeAttribute("name", operation->name());
    stream.writeAttribute("enabled", QString::number(operation->enabled()));

    // Shaders: encoded in base64

    stream.writeStartElement("vertex_shader");
    stream.writeCharacters(QString::fromUtf8(operation->vertexShader().toUtf8().toBase64()));
    stream.writeEndElement();

    stream.writeStartElement("fragment_shader");
    stream.writeCharacters(QString::fromUtf8(operation->fragmentShader().toUtf8().toBase64()));
    stream.writeEndElement();

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

    stream.writeEndElement();
}



bool OperationParser::readOperation(ImageOperation* operation, QXmlStreamReader& stream, bool readIds)
{
    // Operation

    if (stream.readNextStartElement() && stream.name() == "operation")
    {
        operation->clearParameters();

        QString name = stream.attributes().value("name").toString();
        bool enabled = stream.attributes().value("enabled").toInt();

        operation->setName(name);
        operation->enable(enabled);

        QString vertexShader;
        QString fragmentShader;

        while (stream.readNextStartElement())
        {
            if (stream.name() == "vertex_shader")
            {
                vertexShader = QString::fromUtf8(QByteArray::fromBase64(stream.readElementText().toUtf8()));

                if (!vertexShader.isEmpty() && !fragmentShader.isEmpty())
                    if (!operation->setShadersFromSourceCode(vertexShader, fragmentShader))
                    {
                        stream.raiseError("GLSL Shaders error");
                        return false;
                    }
            }
            else if (stream.name() == "fragment_shader")
            {
                fragmentShader = QString::fromUtf8(QByteArray::fromBase64(stream.readElementText().toUtf8()));

                if (!vertexShader.isEmpty() && !fragmentShader.isEmpty())
                    if (!operation->setShadersFromSourceCode(vertexShader, fragmentShader))
                    {
                        stream.raiseError("GLSL Shaders error");
                        return false;
                    }
            }
            else if (stream.name() == "input_attributes")
            {
                while (stream.readNextStartElement())
                {
                    if (stream.name() == "position")
                    {
                        QString posInAttribName = stream.readElementText();
                        operation->setPosInAttribName(posInAttribName);
                    }
                    else if (stream.name() == "texture")
                    {
                        QString texInAttribName = stream.readElementText();
                        operation->setTexInAttribName(texInAttribName);
                    }
                    else
                        stream.skipCurrentElement();
                }
            }
            else if (stream.name() == "parameter")
            {
                QString paramType = stream.attributes().value("type").toString();

                if (paramType == "float_uniform")
                    readParameters<float>(operation, stream, readIds);
                else if (paramType == "int_uniform")
                    readParameters<int>(operation, stream, readIds);
                else if (paramType == "uint_uniform")
                    readParameters<unsigned int>(operation, stream, readIds);
                else if (paramType == "mat4_uniform")
                    readMat4Parameters(operation, stream, readIds);
                else if (paramType == "options")
                    readOptionsParameters(operation, stream);
            }
            else
                stream.skipCurrentElement();
        }

        return true;
    }

    return false;
}



template<typename T>
void OperationParser::writeParameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds)
{
    foreach (auto parameter, operation->uniformParameters<T>())
    {
        stream.writeStartElement("parameter");

        stream.writeAttribute("name", parameter->name());

        if (std::is_same<T, float>::value)
            stream.writeAttribute("type", "float_uniform");
        else if (std::is_same<T, int>::value)
            stream.writeAttribute("type", "int_uniform");
        else if (std::is_same<T, unsigned int>::value)
            stream.writeAttribute("type", "uint_uniform");
        else
            stream.writeAttribute("type", "unsupported");

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



template<typename T>
void OperationParser::readParameters(ImageOperation* operation, QXmlStreamReader& stream, bool readIds)
{
    QString paramName = stream.attributes().value("name").toString();
    bool editable = stream.attributes().value("editable").toInt();
    int row = stream.attributes().value("row").toInt();
    int col = stream.attributes().value("column").toInt();

    QString uniformName;
    int uniformType;
    int numItems;

    QList<QUuid> ids;
    QList<T> infs;
    QList<T> sups;
    QList<T> mins;
    QList<T> maxs;
    QList<T> values;

    QMap<QString, QList<T>> presets;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "uniform")
        {
            uniformName = stream.attributes().value("name").toString();
            uniformType = stream.attributes().value("type").toInt();
            numItems = stream.attributes().value("numitems").toInt();

            while (stream.readNextStartElement())
            {
                if (stream.name() == "number")
                {
                    if (readIds)
                        ids.append(QUuid(stream.attributes().value("id").toString()));

                    if (std::is_same<T, float>::value)
                    {
                        infs.append(stream.attributes().value("inf").toFloat());
                        sups.append(stream.attributes().value("sup").toFloat());
                        mins.append(stream.attributes().value("min").toFloat());
                        maxs.append(stream.attributes().value("max").toFloat());
                        values.append(stream.readElementText().toFloat());
                    }
                    else if (std::is_same<T, int>::value)
                    {
                        infs.append(stream.attributes().value("inf").toInt());
                        sups.append(stream.attributes().value("sup").toInt());
                        mins.append(stream.attributes().value("min").toInt());
                        maxs.append(stream.attributes().value("max").toInt());
                        values.append(stream.readElementText().toInt());
                    }
                    else if (std::is_same<T, unsigned int>::value)
                    {
                        infs.append(stream.attributes().value("inf").toUInt());
                        sups.append(stream.attributes().value("sup").toUInt());
                        mins.append(stream.attributes().value("min").toUInt());
                        maxs.append(stream.attributes().value("max").toUInt());
                        values.append(stream.readElementText().toUInt());
                    }
                }
                else
                    stream.skipCurrentElement();
            }
        }
        else if (stream.name() == "presets")
        {
            while (stream.readNextStartElement())
            {
                QString presetName = stream.name().toString();
                QList<T> presetValues;

                while (stream.readNextStartElement())
                {
                    if (stream.name() == "value")
                    {
                        if (std::is_same<T, float>::value)
                            presetValues.append(stream.readElementText().toFloat());
                        else if (std::is_same<T, int>::value)
                            presetValues.append(stream.readElementText().toInt());
                        else if (std::is_same<T, unsigned int>::value)
                            presetValues.append(stream.readElementText().toUInt());
                    }
                    else
                        stream.skipCurrentElement();
                }

                presets[presetName] = presetValues;
            }
        }
        else
            stream.skipCurrentElement();
    }

    UniformParameter<T>* parameter = nullptr;

    if (readIds)
        parameter = new UniformParameter<T>(paramName, uniformName, uniformType, numItems, editable, ids, values, mins, maxs, infs, sups, operation);
    else
        parameter = new UniformParameter<T>(paramName, uniformName, uniformType, numItems, editable, values, mins, maxs, infs, sups, operation);

    parameter->setPresets(presets);

    parameter->setRow(row);
    parameter->setCol(col);

    operation->addUniformParameter<T>(parameter);

    parameter->setUniform();
}



void OperationParser::writeMat4Parameters(ImageOperation* operation, QXmlStreamWriter& stream, bool writeIds)
{
    foreach (auto parameter, operation->mat4UniformParameters())
    {
        stream.writeStartElement("parameter");

        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("type", "mat4_uniform");
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



void OperationParser::readMat4Parameters(ImageOperation* operation, QXmlStreamReader& stream, bool readIds)
{
    QString paramName = stream.attributes().value("name").toString();
    bool editable = stream.attributes().value("editable").toInt();
    int row = stream.attributes().value("row").toInt();
    int col = stream.attributes().value("column").toInt();

    QString uniformName;
    UniformMat4Type mat4Type;

    QList<QUuid> ids;
    QList<float> infs;
    QList<float> sups;
    QList<float> mins;
    QList<float> maxs;
    QList<float> values;

    QMap<QString, QList<float>> presets;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "uniform")
        {
            uniformName = stream.attributes().value("name").toString();
            mat4Type = static_cast<UniformMat4Type>(stream.attributes().value("mat4_type").toInt());

            while (stream.readNextStartElement())
            {
                if (stream.name() == "number")
                {
                    if (readIds)
                        ids.append(QUuid(stream.attributes().value("id").toString()));

                    infs.append(stream.attributes().value("inf").toFloat());
                    sups.append(stream.attributes().value("sup").toFloat());
                    mins.append(stream.attributes().value("min").toFloat());
                    maxs.append(stream.attributes().value("max").toFloat());
                    values.append(stream.readElementText().toFloat());
                }
                else
                    stream.skipCurrentElement();
            }
        }
        else if (stream.name() == "presets")
        {
            while (stream.readNextStartElement())
            {
                QString presetName = stream.name().toString();
                QList<float> presetValues;

                while (stream.readNextStartElement())
                {
                    if (stream.name() == "value")
                        presetValues.append(stream.readElementText().toFloat());
                    else
                        stream.skipCurrentElement();
                }

                presets[presetName] = presetValues;
            }
        }
        else
            stream.skipCurrentElement();
    }

    UniformMat4Parameter* parameter = nullptr;

    if (readIds)
        parameter = new UniformMat4Parameter(paramName, uniformName, editable, mat4Type, ids, values, mins, maxs, infs, sups, operation);
    else
        parameter = new UniformMat4Parameter(paramName, uniformName, editable, mat4Type, values, mins, maxs, infs, sups, operation);

    parameter->setPresets(presets);

    parameter->setRow(row);
    parameter->setCol(col);

    operation->addMat4UniformParameter(parameter);

    parameter->setUniform();
}



void OperationParser::writeOptionsParameters(ImageOperation* operation, QXmlStreamWriter& stream)
{
    foreach (auto parameter, operation->optionsParameters<GLenum>())
    {
        stream.writeStartElement("parameter");

        stream.writeAttribute("name", parameter->name());
        stream.writeAttribute("type", "options");
        stream.writeAttribute("editable", QString::number(parameter->editable()));
        stream.writeAttribute("row", QString::number(parameter->row()));
        stream.writeAttribute("column", QString::number(parameter->col()));
        stream.writeAttribute("value", QString::number(parameter->value()));

        QList<QString> names = parameter->valueNames();

        int i = 0;

        foreach (auto value, parameter->values())
        {
            stream.writeStartElement("option");
            stream.writeAttribute("name", names[i++]);
            stream.writeCharacters(QString::number(value));
            stream.writeEndElement();
        }

        stream.writeEndElement();
    }
}



void OperationParser::readOptionsParameters(ImageOperation* operation, QXmlStreamReader& stream)
{
    QString paramName = stream.attributes().value("name").toString();
    bool editable = stream.attributes().value("editable").toInt();
    int row = stream.attributes().value("row").toInt();
    int col = stream.attributes().value("column").toInt();
    GLenum value = stream.attributes().value("value").toUInt();

    QList<QString> names;
    QList<GLenum> values;

    while (stream.readNextStartElement())
    {
        if (stream.name() == "option")
        {
            names.append(stream.attributes().value("name").toString());
            values.append(stream.readElementText().toUInt());
        }
    }

    OptionsParameter<GLenum>* parameter = new OptionsParameter<GLenum>(paramName, editable, names, values, value, operation);

    parameter->setRow(row);
    parameter->setCol(col);

    operation->addOptionsParameter<GLenum>(parameter);

    parameter->setValue();
}
