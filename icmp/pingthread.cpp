#include "pingthread.h"

#include <QThread>
#include <QTimerEvent>

PingThread::PingThread(const QString &ip, QObject *parent)
    : QObject(parent), m_workThread(new QThread()), m_ip(ip) {
  m_workThread->setObjectName("mq-process-thread");
  moveToThread(m_workThread.data());
  connect(m_workThread.data(), &QThread::started, this, &PingThread::init,
          Qt::AutoConnection);

  connect(m_workThread.data(), &QThread::finished, this, &PingThread::stop,
          Qt::AutoConnection);
  m_workThread->start();
}

PingThread::~PingThread() {
  m_workThread->quit();
  m_workThread->wait();
}

void PingThread::init() {
  io_context.reset(new asio::io_context());
  try {
    m_pinger.reset(
        new pinger(*io_context.data(), m_ip.toStdString().c_str(), this));
  } catch (exception &e) {
    addLog("exception create pinger " + m_ip);
    m_pinger.reset();
  }

  m_timerId = startTimer(1);
}

bool PingThread::event(QEvent *event) {
  if (event->type() == LogEvent) {
    PinglogEvent *myEvent = static_cast<PinglogEvent *>(event);
    if (myEvent) {
      emit addLog(myEvent->mlog);
    }
    return true;
  } else if (event->type() == LogFailEvent) {
    PingFailEvent *myEvent = static_cast<PingFailEvent *>(event);
    if (myEvent) {
      emit addFailLog(myEvent->mlog);
    }
    return true;
  }
  return QObject::event(event);
}

void PingThread::stop() {
  if (io_context) {
    io_context->stop();

    if (m_pinger) {
      m_pinger.reset();
    }
    io_context.reset();
  }

  if (-1 != m_timerId) {
    killTimer(m_timerId);
    m_timerId = -1;
  }
}

void PingThread::timerEvent(QTimerEvent *event) {
  if (event->timerId() == m_timerId) {
    if (io_context) {
      io_context->poll();
    }
  }
}
