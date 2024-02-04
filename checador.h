#ifndef CHECADOR_H
#define CHECADOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class Checador; }
QT_END_NAMESPACE

class Checador : public QMainWindow
{
    Q_OBJECT

public:
    Checador(QWidget *parent = nullptr);

    void set_to_not_found();

    ~Checador();

private slots:
    void on_input_returnPressed();

    void on_pushButton_clicked();

private:
    Ui::Checador *ui;
};
#endif // CHECADOR_H
