#include "operationbuilder.h"
#include "imageoperation.h"

#include <QTabWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>



OperationBuilder::OperationBuilder(ImageOperation *operation, QWidget *parent) :
    QWidget {parent},
    QOpenGLExtraFunctions(operation->context()),
    mOperation { operation }
{
    mContext = new QOpenGLContext();
    mContext->setFormat(mOperation->context()->format());
    mContext->setShareContext(mOperation->context());
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(mOperation->context()->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);
    initializeOpenGLFunctions();
    mContext->doneCurrent();

    mProgram = new QOpenGLShaderProgram();

    QPushButton* loadVertexButton = new QPushButton("Load vertex shader");
    loadVertexButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(loadVertexButton, &QPushButton::clicked, this, &OperationBuilder::loadVertexShader);

    QPushButton* loadFragmentButton = new QPushButton("Load fragment shader");
    loadFragmentButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(loadFragmentButton, &QPushButton::clicked, this, &OperationBuilder::loadFragmentShader);

    parseButton = new QPushButton("Parse shaders");
    parseButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(parseButton, &QPushButton::clicked, this, &OperationBuilder::parseShaders);

    vertexEditor = new QPlainTextEdit;
    vertexEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    fragmentEditor = new QPlainTextEdit;
    fragmentEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    QTabWidget* shadersTabWidget = new QTabWidget;
    shadersTabWidget->addTab(vertexEditor, "Vertex shader");
    shadersTabWidget->addTab(fragmentEditor, "Fragment shader");

    connect(loadVertexButton, &QPushButton::clicked, this, [=](){ shadersTabWidget->setCurrentIndex(0); });
    connect(loadFragmentButton, &QPushButton::clicked, this, [=](){ shadersTabWidget->setCurrentIndex(1); });

    // Layouts

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->setAlignment(Qt::AlignLeft);
    buttonsLayout->addWidget(loadVertexButton);
    buttonsLayout->addWidget(loadFragmentButton);
    buttonsLayout->addWidget(parseButton);

    QVBoxLayout* shadersLayout = new QVBoxLayout;
    shadersLayout->addLayout(buttonsLayout);
    shadersLayout->addWidget(shadersTabWidget);

    setLayout(shadersLayout);

    setWindowTitle("Operation Builder");
}



OperationBuilder::~OperationBuilder()
{
    delete mProgram;
    delete mContext;
    delete mSurface;
}



void OperationBuilder::loadVertexShader()
{
    QString path = QFileDialog::getOpenFileName(this, "Load vertex shader", QDir::currentPath() + "/shaders", "Vertex shaders (*.vert)");

    if (!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly))
            QMessageBox::information(this, "Error opening file", file.errorString());

        QTextStream in(&file);
        vertexShader = in.readAll();

        vertexEditor->setPlainText(vertexShader);

        file.close();
    }
}



void OperationBuilder::loadFragmentShader()
{
    QString path = QFileDialog::getOpenFileName(this, "Load fragment shader", QDir::currentPath() + "/shaders", "Fragment shaders (*.frag)");

    if (!path.isEmpty())
    {
        QFile file(path);
        if(!file.open(QIODevice::ReadOnly))
            QMessageBox::information(this, "Error opening file", file.errorString());

        QTextStream in(&file);
        fragmentShader = in.readAll();

        fragmentEditor->setPlainText(fragmentShader);

        file.close();
    }
}



void OperationBuilder::parseShaders()
{
    if (linkProgram())
    {
        parseAttributes();
        parseUniforms();

        emit shadersParsed();
    }
}



void OperationBuilder::parseUniforms()
{
    newParamList.clear();

    mProgram->bind();

    GLint numUniforms;
    glGetProgramInterfaceiv(mProgram->programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

    for (GLint index = 0; index < numUniforms; index++)
    {
        QList<GLenum> properties = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE };
        QList<GLint> values(properties.size());

        glGetProgramResourceiv(mProgram->programId(), GL_UNIFORM, index, properties.size(), properties.data(), values.size(), NULL, values.data());

        QList<GLchar> name(values.at(0));
        glGetProgramResourceName(mProgram->programId(), GL_UNIFORM, index, name.size(), nullptr, name.data());

        QString uniformName(name.constData());
        int uniformType = values.at(1);
        int numItems = values.at(2);

        if (!paramList.contains(uniformName))
        {
            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (fParamMap.contains(uniformName) && (fParamMap[uniformName]->uniformType() != uniformType || fParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<float>(fParamMap[uniformName]);
            fParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (iParamMap.contains(uniformName) && (iParamMap[uniformName]->uniformType() != uniformType || iParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<int>(iParamMap[uniformName]);
            iParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (uiParamMap.contains(uniformName) && (uiParamMap[uniformName]->uniformType() != uniformType || uiParamMap[uniformName]->numItems() != numItems))
        {
            mOperation->removeUniformParameter<unsigned int>(uiParamMap[uniformName]);
            uiParamMap.remove(uniformName);
            paramList.removeOne(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else
        {
            newParamList.append(uniformName);
        }
    }

    mProgram->release();

    foreach (QString name, paramList)
    {
        if (!newParamList.contains(name))
        {
            if (fParamMap.contains(name))
            {
                mOperation->removeUniformParameter<float>(fParamMap[name]);
                fParamMap.remove(name);
            }
            else if (iParamMap.contains(name))
            {
                mOperation->removeUniformParameter<int>(iParamMap[name]);
                iParamMap.remove(name);
            }
            else if (uiParamMap.contains(name))
            {
                mOperation->removeUniformParameter<unsigned int>(uiParamMap[name]);
                uiParamMap.remove(name);
            }
        }
    }

    paramList = newParamList;
}



void OperationBuilder::parseAttributes()
{
    mProgram->bind();

    GLint numAttributes;
    glGetProgramInterfaceiv(mProgram->programId(), GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numAttributes);

    for (GLint index = 0; index < numAttributes; index++)
    {
        QList<GLenum> properties = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE };
        QList<GLint> values(properties.size());

        glGetProgramResourceiv(mProgram->programId(), GL_PROGRAM_INPUT, index, properties.size(), properties.data(), values.size(), NULL, values.data());

        QList<GLchar> name(values.at(0));
        glGetProgramResourceName(mProgram->programId(), GL_PROGRAM_INPUT, index, name.size(), nullptr, name.data());

        QString attributeName(name.constData());
    }

    mProgram->release();
}



bool OperationBuilder::linkProgram()
{
    mProgram->removeAllShaders();

    if (!mProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexEditor->toPlainText()))
    {
        QMessageBox::information(this, "Vertex shader error", mProgram->log());
        return false;
    }
    if (!mProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentEditor->toPlainText()))
    {
        QMessageBox::information(this, "Fragment shader error", mProgram->log());
        return false;
    }
    if (!mProgram->link())
    {
        QMessageBox::information(this, "Shader link error", mProgram->log());
        return false;
    }

    return true;
}



void OperationBuilder::addUniformParameter(QString uniformName, int uniformType, int numItems)
{
    Parameter* parameter = nullptr;

    if (uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC2 || uniformType == GL_FLOAT_VEC3 || uniformType == GL_FLOAT_VEC4 || uniformType == GL_FLOAT_MAT2 || uniformType == GL_FLOAT_MAT3 || uniformType == GL_FLOAT_MAT4)
    {
        UniformParameter<float>* fParameter = new UniformParameter<float>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        fParamMap.insert(uniformName, fParameter);
        mOperation->addUniformParameter<float>(fParameter);
        parameter = fParameter;
    }
    else if (uniformType == GL_INT || uniformType == GL_INT_VEC2 || uniformType == GL_INT_VEC3 || uniformType == GL_INT_VEC4)
    {
        UniformParameter<int>* iParameter = new UniformParameter<int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        iParamMap.insert(uniformName, iParameter);
        mOperation->addUniformParameter<int>(iParameter);
        parameter = iParameter;
    }
    else if (uniformType == GL_UNSIGNED_INT || uniformType == GL_UNSIGNED_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC3 || uniformType == GL_UNSIGNED_INT_VEC4)
    {
        UniformParameter<unsigned int>* uiParameter = new UniformParameter<unsigned int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, mOperation);
        uiParamMap.insert(uniformName, uiParameter);
        mOperation->addUniformParameter<unsigned int>(uiParameter);
        parameter = uiParameter;
    }

    if (parameter)
    {
        parameter->setUpdateOperation(false);

        parameter->setRow(-1);
        parameter->setCol(0);

        newParamList.append(uniformName);
    }
}
