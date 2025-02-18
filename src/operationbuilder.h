#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include "imageoperations.h"
#include "parameter.h"
#include "operationswidget.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QListWidget>



class OperationBuilder : public QWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit OperationBuilder(QOpenGLContext* mainContext, QWidget *parent = nullptr);
    ~OperationBuilder();

    void init(QOpenGLContext* mainContext);

private:
    QOpenGLContext* mMainContext;
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;
    QOpenGLShaderProgram* mProgram;

    QPlainTextEdit* vertexEditor;
    QPlainTextEdit* fragmentEditor;

    QString vertexShader;
    QString fragmentShader;

    QPushButton* parseButton;

    QListWidget* uniformListWidget;
    QMap<QString, Parameter*> newParamMap;
    QMap<QString, Parameter*> paramMap;
    ImageOperation* mOperation;
    OperationsWidget* mOpWidget = nullptr;

    QMap<QString, UniformParameter<float>*> fParamMap;
    QMap<QString, UniformParameter<int>*> iParamMap;
    QMap<QString, UniformParameter<unsigned int>*> uiParamMap;

    bool linkProgram();
    void parseUniforms();
    void parseAttributes();

    void addUniformParameter(QString uniformName, int uniformType, int numItems);

private slots:
    void loadVertexShader();
    void loadFragmentShader();
    void parseShaders();
    void setParametersIndices();
};



#endif // OPERATIONBUILDER_H
