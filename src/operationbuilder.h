#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include "parameters/uniformparameter.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QListWidget>



class ImageOperation;



class OperationBuilder : public QWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit OperationBuilder(ImageOperation* operation, QWidget *parent = nullptr);
    ~OperationBuilder();

    void addMat4UniformParameter(UniformMat4Parameter* parameter);
    void removeMat4UniformParameter(UniformMat4Parameter* parameter);

    void addUniformFloatParameter(UniformParameter<float>* parameter);
    void removeUniformFloatParameter(UniformParameter<float>* parameter);

signals:
    void shadersParsed();

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;
    QOpenGLShaderProgram* mProgram;

    QPlainTextEdit* vertexEditor;
    QPlainTextEdit* fragmentEditor;

    QString vertexShader;
    QString fragmentShader;

    QPushButton* parseButton;

    QList<QString> newParamList;
    QList<QString> paramList;

    ImageOperation* mOperation;

    QMap<QString, UniformParameter<float>*> fParamMap;
    QMap<QString, UniformParameter<int>*> iParamMap;
    QMap<QString, UniformParameter<unsigned int>*> uiParamMap;
    QMap<QString, UniformMat4Parameter*> mat4ParamMap;

    bool linkProgram();
    void parseUniforms();
    void parseAttributes();

    void addUniformParameter(QString uniformName, int uniformType, int numItems);

private slots:
    void loadVertexShader();
    void loadFragmentShader();
    void parseShaders();
};



#endif // OPERATIONBUILDER_H
