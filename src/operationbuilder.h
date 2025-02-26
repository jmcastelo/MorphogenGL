#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include "imageoperation.h"
#include "parameter.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QListWidget>



class OperationWidget;



class OperationBuilder : public QWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit OperationBuilder(ImageOperation* operation, OperationWidget* opWidget, QWidget *parent = nullptr);
    ~OperationBuilder();

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;
    QOpenGLShaderProgram* mProgram;

    QPlainTextEdit* vertexEditor;
    QPlainTextEdit* fragmentEditor;

    QString vertexShader;
    QString fragmentShader;

    QPushButton* parseButton;

    QMap<QString, Parameter*> newParamMap;
    QMap<QString, Parameter*> paramMap;
    ImageOperation* mOperation;
    OperationWidget* mOpWidget;

    QMap<QString, UniformParameter<float>*> fParamMap;
    QMap<QString, UniformParameter<int>*> iParamMap;
    QMap<QString, UniformParameter<unsigned int>*> uiParamMap;

    bool linkProgram();
    void parseUniforms();
    void parseAttributes();

    void addUniformParameter(QString uniformName, int uniformType, int numItems);

    int maxRow();

private slots:
    void loadVertexShader();
    void loadFragmentShader();
    void parseShaders();
};



#endif // OPERATIONBUILDER_H
