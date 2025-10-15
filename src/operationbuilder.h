#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include "parameters/uniformparameter.h"

#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QAction>
#include <QStatusBar>
#include <QLabel>



class ImageOperation;



class OperationBuilder : public QWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit OperationBuilder(ImageOperation* operation, QWidget *parent = nullptr);
    ~OperationBuilder();

    void setOperation(ImageOperation* operation);

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

    QList<QString> newParamList;
    QList<QString> paramList;

    QAction* setupOpAction;

    QTabWidget* shadersTabWidget;

    QStatusBar* statusBar;

    QLabel* cursorPosLabel;

    ImageOperation* mOperation;

    QMap<QString, UniformParameter<float>*> fParamMap;
    QMap<QString, UniformParameter<int>*> iParamMap;
    QMap<QString, UniformParameter<unsigned int>*> uiParamMap;
    QMap<QString, UniformMat4Parameter*> mat4ParamMap;

    void populateParamContainers();

    bool linkProgram();
    bool parseInputAttributes();
    bool parseSamplers();
    void parseUniforms();

    void addUniformParameter(QString uniformName, int uniformType, int numItems);

private slots:
    void loadOperation();
    void saveOperation();
    void loadVertexShader();
    void loadFragmentShader();
    void parseShaders();
    void setupOperation();
    void updateCursorPosLabel();
};



#endif // OPERATIONBUILDER_H
