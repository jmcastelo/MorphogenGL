#include "operationbuilder.h"

#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>



OperationBuilder::OperationBuilder(QWidget *parent)
    : QWidget{parent}
{
    mProgram = new QOpenGLShaderProgram();

    QPushButton* setVertexShaderButton = new QPushButton("Set vertex shader");
    setVertexShaderButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(setVertexShaderButton, &QPushButton::clicked, this, &OperationBuilder::setVertexShaderPath);

    QPushButton* setFragmentShaderButton = new QPushButton("Set fragment shader");
    setFragmentShaderButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    connect(setFragmentShaderButton, &QPushButton::clicked, this, &OperationBuilder::setFragmentShaderPath);

    parseButton = new QPushButton("Parse shaders");
    parseButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    parseButton->setEnabled(false);

    connect(parseButton, &QPushButton::clicked, this, &OperationBuilder::parseShaders);

    vertexLineEdit = new QLineEdit;
    vertexLineEdit->setReadOnly(true);

    fragmentLineEdit = new QLineEdit;
    fragmentLineEdit->setReadOnly(true);

    QHBoxLayout* setShadersLayout = new QHBoxLayout;
    setShadersLayout->addWidget(setVertexShaderButton);
    setShadersLayout->addWidget(setFragmentShaderButton);

    QFormLayout* pathsLayout = new QFormLayout;
    pathsLayout->addRow("Vertex shader:", vertexLineEdit);
    pathsLayout->addRow("Fragment shader:", fragmentLineEdit);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(setShadersLayout);
    layout->addLayout(pathsLayout);
    layout->addWidget(parseButton);

    setLayout(layout);

    setWindowTitle("Operation Builder");
    setVisible(false);
}



OperationBuilder::~OperationBuilder()
{
    delete mProgram;
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
}



void OperationBuilder::setVertexShaderPath()
{
    QString path = QFileDialog::getOpenFileName(this, "Set vertex shader", QDir::currentPath() + "/shaders", "Vertex shaders (*.vert)");

    if (!path.isEmpty())
    {
        mVertexShader = path;
        vertexLineEdit->setText(path);
    }

    enableParseButton();
}



void OperationBuilder::setFragmentShaderPath()
{
    QString path = QFileDialog::getOpenFileName(this, "Set fragment shader", QDir::currentPath() + "/shaders", "Fragment shaders (*.frag)");

    if (!path.isEmpty())
    {
        mFragmentShader = path;
        fragmentLineEdit->setText(path);
    }

    enableParseButton();
}



void OperationBuilder::enableParseButton()
{
    parseButton->setEnabled(!mVertexShader.isEmpty() && !mFragmentShader.isEmpty());
}



void OperationBuilder::parseShaders()
{
    if (linkProgram())
    {
        mProgram->bind();

        GLint numUniforms;
        glGetProgramInterfaceiv(mProgram->programId(), GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

        for (GLint index = 0; index < numUniforms; index++)
        {
            QList<GLenum> properties = { GL_NAME_LENGTH, GL_TYPE, GL_ARRAY_SIZE };
            QList<GLint> values(properties.size());

            glGetProgramResourceiv(mProgram->programId(), GL_UNIFORM, index, properties.size(), properties.data(), values.size(), NULL, values.data());

            QList<GLchar> name(values.at(0));
            glGetProgramResourceName(mProgram->programId(), GL_UNIFORM, index, name.size(), NULL, name.data());

            QString uniformName(name.constData());
            qDebug() << uniformName;
            qDebug() << values;
        }

        mProgram->release();
    }
}



bool OperationBuilder::linkProgram()
{
    mProgram->removeAllShaders();

    if (!mProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, mVertexShader))
    {
        qDebug() << "Vertex shader error:\n" << mProgram->log();
        return false;
    }
    if (!mProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, mFragmentShader))
    {
        qDebug() << "Fragment shader error:\n" << mProgram->log();
        return false;
    }
    if (!mProgram->link())
    {
        qDebug() << "Shader link error:\n" << mProgram->log();
        return false;
    }
    return true;
}
