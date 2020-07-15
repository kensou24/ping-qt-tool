#include "mainwindow.h"

#include "pingthread.h"
#include "ui_mainwindow.h"
extern int test(std::string ip);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_pushButton_clicked() {
  m_pingThread.reset(new PingThread(ui->lineEdit->text()));
  connect(m_pingThread.data(), &PingThread::addLog, this,
          [&](const QString &log) {
            if (ui->textEdit_log->toPlainText().size() > 50000) {
              ui->textEdit_log->clear();
            }

            ui->textEdit_log->append(log);
          });
  connect(m_pingThread.data(), &PingThread::addFailLog, this,
          [&](const QString &log) {
            if (ui->textEdit_failLog->toPlainText().size() > 50000) {
              ui->textEdit_failLog->clear();
            }
            ui->textEdit_failLog->append(log);
          });
}
