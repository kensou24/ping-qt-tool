#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineEdit>
#include <QMainWindow>

#include "pingthread.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_addIP_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_exportLog_clicked();

    void on_pushButton_clearlog_clicked();

private:
    Ui::MainWindow* ui;
    QList<QLineEdit*> m_lineEdits;
    QList<PingThread*> m_pingThreadList;
};
#endif // MAINWINDOW_H
