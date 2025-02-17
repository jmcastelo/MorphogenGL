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



struct UniformData
{
    QString name;
    int type;
    int size;
};



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
    QMap<QString, Parameter*> mParameterMap;
    ImageOperation* mOperation;
    OperationsWidget* mOpWidget = nullptr;

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
