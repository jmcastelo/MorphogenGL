#include "operationbuilder.h"

#include <QFileDialog>
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

    QHBoxLayout* setShadersLayout = new QHBoxLayout;
    setShadersLayout->addWidget(setVertexShaderButton);
    setShadersLayout->addWidget(setFragmentShaderButton);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout(setShadersLayout);
    layout->addWidget(parseButton);

    setLayout(layout);

    setWindowTitle("Operation Builder");
    setVisible(false);
}



OperationBuilder::~OperationBuilder()
{
    delete mProgram;
}


void OperationBuilder::setVertexShaderPath()
{
    QString path = QFileDialog::getOpenFileName(this, "Set vertex shader", QDir::currentPath() + "/shaders", "Vertex shaders (*.vert)");

    if (!path.isEmpty())
        mVertexShader = path;

    enableParseButton();
}



void OperationBuilder::setFragmentShaderPath()
{
    QString path = QFileDialog::getOpenFileName(this, "Set fragment shader", QDir::currentPath() + "/shaders", "Fragment shaders (*.frag)");

    if (!path.isEmpty())
        mFragmentShader = path;

    enableParseButton();
}



void OperationBuilder::enableParseButton()
{
    parseButton->setEnabled(!mVertexShader.isEmpty() && !mFragmentShader.isEmpty());
}



void OperationBuilder::parseShaders()
{
    linkProgram();
}



void OperationBuilder::linkProgram()
{
    mProgram->removeAllShaders();

    if (!mProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, mVertexShader))
        qDebug() << "Vertex shader error:\n" << mProgram->log();
    if (!mProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, mFragmentShader))
        qDebug() << "Fragment shader error:\n" << mProgram->log();
    if (!mProgram->link())
        qDebug() << "Shader link error:\n" << mProgram->log();
}
