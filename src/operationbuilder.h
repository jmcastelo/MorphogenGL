#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include "parameters/uniformparameter.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>



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
    void operationSetUp();

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;
    QOpenGLShaderProgram* mProgram;

    QPlainTextEdit* vertexEditor;
    QPlainTextEdit* fragmentEditor;

    QString vertexShader;
    QString fragmentShader;

    QList<QString> inAttribList;

    QComboBox* attrComboBox;

    QList<QString> newParamList;
    QList<QString> paramList;

    QPushButton* setupOperationButton;

    QTabWidget* shadersTabWidget;

    ImageOperation* mOperation;

    QMap<QString, UniformParameter<float>*> fParamMap;
    QMap<QString, UniformParameter<int>*> iParamMap;
    QMap<QString, UniformParameter<unsigned int>*> uiParamMap;
    QMap<QString, UniformMat4Parameter*> mat4ParamMap;

    bool linkProgram();
    void parseInputAttributes();
    void parseUniforms();

    void addUniformParameter(QString uniformName, int uniformType, int numItems);

    bool checkInputAttributes();

    void setAttribComboBox();

private slots:
    void loadOperation();
    void saveOperation();
    void loadVertexShader();
    void loadFragmentShader();
    void parseShaders();
};



#endif // OPERATIONBUILDER_H
