#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include <QWidget>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLShaderProgram>
#include <QLineEdit>
#include <QPushButton>



class OperationBuilder : public QWidget, public QOpenGLExtraFunctions
{
    Q_OBJECT

public:
    explicit OperationBuilder(QWidget *parent = nullptr);
    ~OperationBuilder();

    void init(QOpenGLContext* mainContext);

private:
    QOpenGLContext* mContext;
    QOffscreenSurface* mSurface;
    QOpenGLShaderProgram* mProgram;

    QString mVertexShader;
    QString mFragmentShader;

    QLineEdit* vertexLineEdit;
    QLineEdit* fragmentLineEdit;

    QPushButton* parseButton;

    void enableParseButton();
    bool linkProgram();

private slots:
    void setVertexShaderPath();
    void setFragmentShaderPath();
    void parseShaders();
};



#endif // OPERATIONBUILDER_H
