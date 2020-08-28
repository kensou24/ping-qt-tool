#include "mainwindow.h"

#include "pingthread.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextStream>
extern int test(std::string ip);

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->m_lineEdits.append(ui->lineEdit);
}

MainWindow::~MainWindow()
{
    delete ui;
    qDeleteAll(m_pingThreadList);
    m_pingThreadList.clear();
}

void MainWindow::on_pushButton_clicked()
{
    on_pushButton_2_clicked();

    ui->textEdit_log->append(tr("prepare to start new"));

    for (int i = 0; i < m_lineEdits.size(); i++) {
        QString ip = m_lineEdits[i]->text().simplified();
        if (ip.isEmpty()) {
            continue;
        }
        PingThread* pingThread = new PingThread(ip);

        ui->textEdit_log->append(tr("prepare to start new thread ") + ip);

        m_pingThreadList.append(pingThread);
        connect(pingThread, &PingThread::addLog, this,
            [&](const QString& log) {
                if (ui->textEdit_log->toPlainText().size() > 500000) {
                    ui->textEdit_log->clear();
                }

                ui->textEdit_log->append(log);
            });
        connect(pingThread, &PingThread::addFailLog, this,
            [&](const QString& log) {
                if (ui->textEdit_failLog->toPlainText().size() > 500000) {
                    ui->textEdit_failLog->clear();
                }
                ui->textEdit_failLog->append(log);
            });
    }
}

void MainWindow::on_pushButton_addIP_clicked()
{
    QLineEdit* edit = new QLineEdit(this);
    ui->gridLayout_extern->addWidget(edit);
    this->m_lineEdits.append(edit);
}

void MainWindow::on_pushButton_2_clicked()
{
    if (!m_pingThreadList.isEmpty()) {
        ui->textEdit_log->append(tr("prepare to stop"));
        qDeleteAll(m_pingThreadList);
        m_pingThreadList.clear();
    }
}

void MainWindow::on_pushButton_exportLog_clicked()
{
    QFile data("log.txt");
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        QString logStr = ui->textEdit_log->toPlainText();
        out << logStr;
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("导出失败"));
        msgBox.exec();
        return;
    }

    data.close();

    QFileInfo fileInfo(data);
    QMessageBox msgBox;
    msgBox.setText(QString(tr("导出成功,日志文件地址-%1")).arg(fileInfo.absoluteFilePath()));
    msgBox.exec();
}

void MainWindow::on_pushButton_clearlog_clicked()
{
    ui->textEdit_log->clear();
}
