#ifndef OPERATIONBUILDER_H
#define OPERATIONBUILDER_H



#include <QWidget>
#include <QOpenGLShaderProgram>
#include <QPushButton>



class OperationBuilder : public QWidget
{
    Q_OBJECT

public:
    explicit OperationBuilder(QWidget *parent = nullptr);
    ~OperationBuilder();

private:
    QOpenGLShaderProgram* mProgram;
    QString mVertexShader;
    QString mFragmentShader;

    QPushButton* parseButton;

    void enableParseButton();
    void linkProgram();

private slots:
    void setVertexShaderPath();
    void setFragmentShaderPath();
    void parseShaders();
};



#endif // OPERATIONBUILDER_H
