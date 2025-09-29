#include "operationbuilder.h"
#include "imageoperation.h"
#include "operationparser.h"

#include <QToolBar>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QVBoxLayout>



OperationBuilder::OperationBuilder(ImageOperation *operation, QWidget *parent) :
    QWidget {parent},
    mOperation { operation }
{
    // OpenGL context

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

    // Toolbar

    QToolBar* toolBar = new QToolBar;
    toolBar->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    toolBar->addAction(QIcon(QPixmap(":/icons/document-open.png")), "Load operation", this, &OperationBuilder::loadOperation);

    toolBar->addAction(QIcon(QPixmap(":/icons/document-save.png")), "Save operation", this, &OperationBuilder::saveOperation);

    toolBar->addSeparator();

    toolBar->addAction(QIcon(QPixmap(":/icons/letter-v.png")), "Load vertex shader", this, &OperationBuilder::loadVertexShader);

    toolBar->addAction(QIcon(QPixmap(":/icons/letter-f.png")), "Load fragment shader", this, &OperationBuilder::loadFragmentShader);

    toolBar->addAction(QIcon(QPixmap(":/icons/run-build.png")), "Parse shaders", this, &OperationBuilder::parseShaders);

    toolBar->addSeparator();

    setupOpAction = new QAction(QIcon(QPixmap(":/icons/dialog-ok.png")), "Setup operation", this);
    setupOpAction->setEnabled(false);
    connect(setupOpAction, &QAction::triggered, this, &OperationBuilder::setupOperation);

    toolBar->addAction(setupOpAction);

    // Tabs

    vertexEditor = new QPlainTextEdit;
    vertexEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(vertexEditor, &QPlainTextEdit::textChanged, this, [=, this](){
        setupOpAction->setEnabled(false);
    });

    fragmentEditor = new QPlainTextEdit;
    fragmentEditor->setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(fragmentEditor, &QPlainTextEdit::textChanged, this, [=, this](){
        setupOpAction->setEnabled(false);
    });

    /*attrComboBox = new QComboBox;
    attrComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    attrComboBox->setEditable(false);

    QFormLayout* attrFormLayout = new QFormLayout;
    attrFormLayout->addRow("Select vertex position input attribute:", attrComboBox);

    setupOperationButton = new QPushButton("Setup operation");
    setupOperationButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setupOperationButton->setEnabled(false);*/

    /*QVBoxLayout* overviewLayout = new QVBoxLayout;
    overviewLayout->addLayout(attrFormLayout);
    overviewLayout->addWidget(setupOperationButton);

    QWidget* overviewWidget = new QWidget;
    overviewWidget->setLayout(overviewLayout);*/

    shadersTabWidget = new QTabWidget;
    shadersTabWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    shadersTabWidget->addTab(vertexEditor, "Vertex shader");
    shadersTabWidget->addTab(fragmentEditor, "Fragment shader");
    //shadersTabWidget->addTab(overviewWidget, "Overview");

    /*connect(setupOperationButton, &QPushButton::clicked, this, [=, this](){
        if (mOperation->setShadersFromSourceCode(vertexEditor->toPlainText(), fragmentEditor->toPlainText()))
        {
            emit operationSetUp();

            //mOperation->setInAttributes();
            mOperation->enableUpdate(true);
        }
        else
        {
             QMessageBox::information(this, "GLSL Shaders error", "Could not set up operation due to GLSL shaders error.");
        }
    });*/

    // Layouts

    QVBoxLayout* shadersLayout = new QVBoxLayout;
    shadersLayout->addWidget(toolBar);
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



void OperationBuilder::addMat4UniformParameter(UniformMat4Parameter* parameter)
{
    mat4ParamMap.insert(parameter->uniformName(), parameter);
    mOperation->addMat4UniformParameter(parameter);
}



void OperationBuilder::removeMat4UniformParameter(UniformMat4Parameter* parameter)
{
    if (mat4ParamMap.contains(parameter->uniformName()))
    {
        mat4ParamMap.remove(parameter->uniformName());
        mOperation->removeMat4UniformParameter(parameter);
    }
}



void OperationBuilder::addUniformFloatParameter(UniformParameter<float>* parameter)
{
    fParamMap.insert(parameter->uniformName(), parameter);
    mOperation->addUniformParameter<float>(parameter);
}



void OperationBuilder::removeUniformFloatParameter(UniformParameter<float>* parameter)
{
    if (fParamMap.contains(parameter->uniformName()))
    {
        fParamMap.remove(parameter->uniformName());
        mOperation->removeUniformParameter<float>(parameter);
    }
}



void OperationBuilder::loadOperation()
{
    QString path = QFileDialog::getOpenFileName(this, "Load operation", QDir::currentPath() + "/operations", "Operations (*.op)");

    if (!path.isEmpty())
    {
        // Clear data

        vertexEditor->clear();
        fragmentEditor->clear();

        fParamMap.clear();
        iParamMap.clear();
        uiParamMap.clear();
        mat4ParamMap.clear();
        paramList.clear();

        //inAttribList.clear();

        //attrComboBox->clear();

        setupOpAction->setEnabled(false);

        // Parse operation

        OperationParser opParser;
        if (!opParser.read(mOperation, path, false))
        {
            emit operationSetUp();
            return;
        }

        mOperation->linkShaders();
        mOperation->enableUpdate(true);
        mOperation->setAllParameters();

        // Shaders

        vertexShader = mOperation->vertexShader();
        fragmentShader = mOperation->fragmentShader();

        vertexEditor->setPlainText(vertexShader);
        fragmentEditor->setPlainText(fragmentShader);

        // Uniforms

        foreach (auto parameter, mOperation->uniformParameters<float>())
        {
            fParamMap.insert(parameter->uniformName(), parameter);
            paramList.append(parameter->uniformName());
        }

        foreach (auto parameter, mOperation->uniformParameters<int>())
        {
            iParamMap.insert(parameter->uniformName(), parameter);
            paramList.append(parameter->uniformName());
        }

        foreach (auto parameter, mOperation->uniformParameters<unsigned int>())
        {
            uiParamMap.insert(parameter->uniformName(), parameter);
            paramList.append(parameter->uniformName());
        }

        foreach (auto parameter, mOperation->mat4UniformParameters())
        {
            mat4ParamMap.insert(parameter->uniformName(), parameter);
            paramList.append(parameter->uniformName());
        }

        // Input attributes

        // inAttribList.append(mOperation->posInAttribName());
        // inAttribList.append(mOperation->texInAttribName());

        // setAttribComboBox();

        setupOpAction->setEnabled(true);

        emit operationSetUp();
    }
}



void OperationBuilder::saveOperation()
{
    QString path = QFileDialog::getSaveFileName(this, "Save operation", QDir::currentPath() + "/operations", "Operations (*.op)");

    if (!path.isEmpty())
    {
        OperationParser opParser;
        opParser.write(mOperation, path, false);
    }
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

        shadersTabWidget->setCurrentIndex(0);
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

        shadersTabWidget->setCurrentIndex(1);
    }
}



void OperationBuilder::parseShaders()
{
    mOperation->enableUpdate(false);

    if (linkProgram())
    {
        parseUniforms();
        parseInputAttributes();
    }
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



void OperationBuilder::parseInputAttributes()
{
    QList<QString> newInAttribList;

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
        int attributeType = values.at(1);
        int numItems = values.at(2);

        if (attributeType == GL_FLOAT_VEC2 && numItems == 1)
            newInAttribList.append(attributeName);
    }

    mProgram->release();

    if (newInAttribList != inAttribList)
    {
        inAttribList = newInAttribList;

        if (checkInputAttributes())
        {
            //setAttribComboBox();
            setupOpAction->setEnabled(true);
            //shadersTabWidget->setCurrentIndex(2);
        }
        else
        {
            inAttribList.clear();
            setupOpAction->setEnabled(false);
        }
    }
    else
    {
        //shadersTabWidget->setCurrentIndex(2);
        setupOpAction->setEnabled(true);
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
            else if (mat4ParamMap.contains(name))
            {
                mOperation->removeMat4UniformParameter(mat4ParamMap[name]);
                mat4ParamMap.remove(name);
            }
        }
    }

    paramList = newParamList;
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
        newParamList.append(uniformName);
}



bool OperationBuilder::checkInputAttributes()
{
    if (inAttribList.size() != 2)
    {
        QString message = "Exactly two active vec2 input attributes must be specified in the vertex shader, corresponding to the 2D vertex position (location 0) and texture coordinates (location 1).";
        QMessageBox::information(this, "Input attributes error", message);
        return false;
    }

    return true;
}



/*void OperationBuilder::setAttribComboBox()
{
    attrComboBox->disconnect();
    attrComboBox->clear();

    attrComboBox->addItems(inAttribList);

    attrComboBox->setCurrentIndex(0);

    mOperation->setPosInAttribName(inAttribList.at(0));
    mOperation->setTexInAttribName(inAttribList.at(1));

    connect(attrComboBox, &QComboBox::activated, this, [=, this](int index){
        if (index >= 0 && index < 2)
        {
            mOperation->setPosInAttribName(inAttribList.at(index));
            mOperation->setTexInAttribName(inAttribList.at((index + 1) % 2));
        }
    });
}*/



void OperationBuilder::setupOperation()
{
    mOperation->setVertexShader(vertexEditor->toPlainText());
    mOperation->setFragmentShader(fragmentEditor->toPlainText());

    if (mOperation->linkShaders())
    {
        emit operationSetUp();

        //mOperation->setInAttributes();
        mOperation->enableUpdate(true);
    }
    else
    {
        QMessageBox::information(this, "GLSL Shaders error", "Could not set up operation due to GLSL shaders error.");
    }
}
