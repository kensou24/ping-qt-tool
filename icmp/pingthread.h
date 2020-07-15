#ifndef PINGTHREAD_H
#define PINGTHREAD_H

#include <QObject>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "ping.h"
using namespace boost;

class PingThread : public QObject {
  Q_OBJECT
 public:
  explicit PingThread(const QString &ip, QObject *parent = nullptr);
  virtual ~PingThread();
 signals:
  void addLog(const QString &log);
  void addFailLog(const QString &log);
 public slots:
  void init();
  void stop();

 protected:
  void timerEvent(QTimerEvent *event) override;

  bool event(QEvent *event) override;

 private:
  QScopedPointer<QThread> m_workThread;  // 工作线程
  QString m_ip;
  QScopedPointer<asio::io_context> io_context;
  QScopedPointer<pinger> m_pinger;
  int m_timerId{-1};
};

#endif  // PINGTHREAD_H
