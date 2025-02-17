#include "operationbuilder.h"

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

    uniformListWidget = new QListWidget;
    uniformListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    uniformListWidget->setDragEnabled(true);
    uniformListWidget->setDropIndicatorShown(true);
    uniformListWidget->setDragDropMode(QAbstractItemView::InternalMove);

    connect(uniformListWidget->model(), &QAbstractItemModel::rowsMoved, this, &OperationBuilder::setParametersIndices);

    // Layouts

    QVBoxLayout* vertexLayout = new QVBoxLayout;
    vertexLayout->addWidget(loadVertexButton);
    vertexLayout->addWidget(vertexEditor);

    QVBoxLayout* fragmentLayout = new QVBoxLayout;
    fragmentLayout->addWidget(loadFragmentButton);
    fragmentLayout->addWidget(fragmentEditor);

    QHBoxLayout* shadersLayout = new QHBoxLayout;
    shadersLayout->addLayout(vertexLayout);
    shadersLayout->addLayout(fragmentLayout);

    QVBoxLayout* opLayout = new QVBoxLayout;
    opLayout->addWidget(parseButton);
    opLayout->addWidget(uniformListWidget);
    opLayout->addWidget(mOpWidget);

    QHBoxLayout* layout = new QHBoxLayout;
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

        setParametersIndices();
    }
}



void OperationBuilder::parseUniforms()
{
    mOperation->clearParameters();
    mParameterMap.clear();
    uniformListWidget->clear();

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

        addUniformParameter(uniformName, uniformType, numItems);
    }

    mProgram->release();
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
    else if (uniformType == GL_FLOAT_VEC2 || uniformType == GL_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC2 || uniformType == GL_FLOAT_MAT2)
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
        mOperation->addUniformParameter<float>(fParameter);
        parameter = fParameter;
    }
    else if (uniformType == GL_INT || uniformType == GL_INT_VEC2 || uniformType == GL_INT_VEC3 || uniformType == GL_INT_VEC4)
    {
        UniformParameter<int>* iParameter = new UniformParameter<int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, QList<int>(numItems * numValuesPerItem, 0), 0, 1, 0, 1, mOperation);
        mOperation->addUniformParameter<int>(iParameter);
        parameter = iParameter;
    }
    else if (uniformType == GL_UNSIGNED_INT || uniformType == GL_UNSIGNED_INT_VEC2 || uniformType == GL_UNSIGNED_INT_VEC3 || uniformType == GL_UNSIGNED_INT_VEC4)
    {
        UniformParameter<unsigned int>* uiParameter = new UniformParameter<unsigned int>(uniformName.toUpper(), uniformName, uniformType, numItems, true, QList<unsigned int>(numItems * numValuesPerItem, 0), 0, 1, 0, 1, mOperation);
        mOperation->addUniformParameter<unsigned int>(uiParameter);
        parameter = uiParameter;
    }

    if (parameter)
    {
        parameter->setUpdateOperation(false);
        mParameterMap.insert(uniformName, parameter);
        uniformListWidget->addItem(uniformName);
    }
}



void OperationBuilder::setParametersIndices()
{
    for (int index = 0; index < uniformListWidget->count(); index++)
    {
        QString name = uniformListWidget->item(index)->text();
        mParameterMap[name]->setIndex(index);
    }

    mOpWidget->recreate(mOperation, false);
}
