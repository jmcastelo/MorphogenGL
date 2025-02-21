#include "operationbuilder.h"

#include <QTabWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>



OperationBuilder::OperationBuilder(QOpenGLContext *mainContext, QWidget *parent) :
    QWidget {parent},
    QOpenGLExtraFunctions(mainContext)
{
    initializeOpenGLFunctions();

    mOperation = new ImageOperation("New Operation", false, mainContext);

    mOpWidget = new OperationsWidget(mOperation, false);
    mOpWidget->setVisible(false);

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

    QVBoxLayout* opLayout = new QVBoxLayout;
    opLayout->setAlignment(Qt::AlignTop);
    opLayout->addWidget(mOpWidget);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->setAlignment(Qt::AlignTop);
    layout->addLayout(shadersLayout);
    layout->addLayout(opLayout);

    setLayout(layout);

    setWindowTitle("Operation Builder");
    setVisible(false);
}



OperationBuilder::~OperationBuilder()
{
    delete mOperation;
    delete mProgram;
    delete mContext;
    delete mSurface;
}



void OperationBuilder::init(QOpenGLContext* mainContext)
{
    mContext = new QOpenGLContext();
    mContext->setFormat(mainContext->format());
    mContext->setShareContext(mainContext);
    mContext->create();

    mSurface = new QOffscreenSurface();
    mSurface->setFormat(mainContext->format());
    mSurface->create();

    mContext->makeCurrent(mSurface);
    initializeOpenGLFunctions();
    mContext->doneCurrent();

    mMainContext = mainContext;

    mOperation = new ImageOperation("New Operation", false, mainContext);
}



void OperationBuilder::loadVertexShader()
{
    QString path = QFileDialog::getOpenFileName(this, "Load vertex shader", QDir::currentPath(), "Vertex shaders (*.vert)");

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
    QString path = QFileDialog::getOpenFileName(this, "Load fragment shader", QDir::currentPath(), "Fragment shaders (*.frag)");

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

        mOpWidget->setVisible(true);
    }
}



void OperationBuilder::parseUniforms()
{
    newParamMap.clear();

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

        if (!paramMap.contains(uniformName))
        {
            addUniformParameter(uniformName, uniformType, numItems);
        }
        else if (paramMap[uniformName]->uniformType() != uniformType || paramMap[uniformName]->numItems() != numItems)
        {
            if (fParamMap.contains(uniformName))
                fParamMap.remove(uniformName);
            else if (iParamMap.contains(uniformName))
                iParamMap.remove(uniformName);
            else if (uiParamMap.contains(uniformName))
                uiParamMap.remove(uniformName);

            paramMap.remove(uniformName);

            addUniformParameter(uniformName, uniformType, numItems);
        }
        else
        {
            newParamMap.insert(uniformName, paramMap[uniformName]);
        }
    }

    mProgram->release();

    for (auto [uniformName, parameter] : paramMap.asKeyValueRange())
    {
        if (!newParamMap.contains(uniformName))
        {
            if (fParamMap.contains(uniformName))
                fParamMap.remove(uniformName);
            else if (iParamMap.contains(uniformName))
                iParamMap.remove(uniformName);
            else if (uiParamMap.contains(uniformName))
                uiParamMap.remove(uniformName);
        }
    }

    paramMap = newParamMap;

    mOperation->clearParameters();

    for (auto [uniformName, parameter] : fParamMap.asKeyValueRange())
        mOperation->addUniformParameter<float>(parameter);
    for (auto [uniformName, parameter] : iParamMap.asKeyValueRange())
        mOperation->addUniformParameter<int>(parameter);
    for (auto [uniformName, parameter] : uiParamMap.asKeyValueRange())
        mOperation->addUniformParameter<unsigned int>(parameter);

    mOpWidget->recreate(mOperation, false);
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
    int numValuesPerItem = 0;

    if (uniformType == GL_FLOAT || uniformType == GL_INT || uniformType == GL_UNSIGNED_INT)
        numValuesPerItem = 1;
    else if (uniformType == GL_FLOAT_VEC2 || uniformType == GL_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC2)
        numValuesPerItem = 2;
    else if (uniformType == GL_FLOAT_VEC3 || uniformType == GL_INT_VEC3 || uniformType == GL_UNSIGNED_INT_VEC3)
        numValuesPerItem = 3;
    else if (uniformType == GL_FLOAT_VEC4 || uniformType == GL_INT_VEC4 || uniformType == GL_UNSIGNED_INT_VEC4 || uniformType == GL_FLOAT_MAT2)
        numValuesPerItem = 4;
    else if (uniformType == GL_FLOAT_MAT3)
        numValuesPerItem = 9;
    else if (uniformType == GL_FLOAT_MAT4)
        numValuesPerItem = 16;

    Parameter* parameter = nullptr;

    if (uniformType == GL_FLOAT || uniformType == GL_FLOAT_VEC2 || uniformType == GL_FLOAT_VEC3 || uniformType == GL_FLOAT_VEC4 || uniformType == GL_FLOAT_MAT2 || uniformType == GL_FLOAT_MAT3 || uniformType == GL_FLOAT_MAT4)
    {
        UniformParameter<float>* fParameter = new UniformParameter<float>(uniformName.toUpper(), uniformName, uniformType, numItems, true, QList<float>(numItems * numValuesPerItem, 0.0f), -1.0f, 1.0f, -1.0f, 1.0f, mOperation);
        fParamMap.insert(uniformName, fParameter);
        parameter = fParameter;
    }
    else if (uniformType == GL_INT || uniformType == GL_INT_VEC2 || uniformType == GL_INT_VEC3 || uniformType == GL_INT_VEC4)
    {
        UniformParameter<int>* iParameter = new UniformParameter<int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, QList<int>(numItems * numValuesPerItem, 0), 0, 1, 0, 1, mOperation);
        iParamMap.insert(uniformName, iParameter);
        parameter = iParameter;
    }
    else if (uniformType == GL_UNSIGNED_INT || uniformType == GL_UNSIGNED_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC3 || uniformType == GL_UNSIGNED_INT_VEC4)
    {
        UniformParameter<unsigned int>* uiParameter = new UniformParameter<unsigned int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, QList<unsigned int>(numItems * numValuesPerItem, 0), 0, 1, 0, 1, mOperation);
        uiParamMap.insert(uniformName, uiParameter);
        parameter = uiParameter;
    }

    if (parameter)
    {
        parameter->setUpdateOperation(false);

        parameter->setRow(maxRow() + 1);
        parameter->setCol(0);

        newParamMap.insert(uniformName, parameter);
    }
}



int OperationBuilder::maxRow()
{
    int row = -1;

    for (auto [uniformName, parameter] : newParamMap.asKeyValueRange())
        if (parameter->row() > row)
            row = parameter->row();

    return row;
}
