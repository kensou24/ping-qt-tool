#pragma once

//
// ping.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <QCoreApplication>
#include <QDateTime>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <ostream>

#include "icmp_header.hpp"
#include "ipv4_header.hpp"

using namespace boost;

using asio::steady_timer;
using asio::ip::icmp;
namespace chrono = asio::chrono;

const QEvent::Type LogEvent =
    (QEvent::Type)QEvent::registerEventType(QEvent::User + 1);

const QEvent::Type LogFailEvent =
    (QEvent::Type)QEvent::registerEventType(QEvent::User + 2);

class PinglogEvent : public QEvent {
 public:
  PinglogEvent(const QString& log) : QEvent(LogEvent), mlog(log) {}

 public:
  QString mlog;
};

class PingFailEvent : public QEvent {
 public:
  PingFailEvent(const QString& log) : QEvent(LogFailEvent), mlog(log) {}

 public:
  QString mlog;
};

class pinger {
 public:
  pinger(asio::io_context& io_context, const char* destination, QObject* object)
      : resolver_(io_context),
        socket_(io_context, icmp::v4()),
        timer_(io_context),
        sequence_number_(0),
        num_replies_(0),
        m_object(object),
        ip(destination) {
    destination_ = *resolver_.resolve(icmp::v4(), destination, "").begin();

    start_send();
    start_receive();
  }

 private:
  void start_send() {
    std::string body("\"Hello!\" from Asio ping.");

    // Create an ICMP header for an echo request.
    icmp_header echo_request;
    echo_request.type(icmp_header::echo_request);
    echo_request.code(0);
    echo_request.identifier(get_identifier());
    echo_request.sequence_number(++sequence_number_);
    compute_checksum(echo_request, body.begin(), body.end());

    // Encode the request packet.
    asio::streambuf request_buffer;
    std::ostream os(&request_buffer);
    os << echo_request << body;

    // Send the request.
    time_sent_ = steady_timer::clock_type::now();
    socket_.send_to(request_buffer.data(), destination_);

    // Wait up to five seconds for a reply.
    num_replies_ = 0;
    timer_.expires_at(time_sent_ + chrono::seconds(3));
    timer_.async_wait(boost::bind(&pinger::handle_timeout, this));
  }

  void handle_timeout() {
    if (num_replies_ == 0) {
      std::cout << "Request timed out" << std::endl;

      QString info =
          QString("%1  Request timed out of %2")
              .arg(QDateTime::currentDateTime().toString("yyyy-MMdd- hh:mm:ss"),
                   ip);
      QCoreApplication::postEvent(m_object, new PingFailEvent(info));
    }

    // Requests must be sent no less than one second apart.
    timer_.expires_at(time_sent_ + chrono::seconds(1));
    timer_.async_wait(boost::bind(&pinger::start_send, this));
  }

  void start_receive() {
    // Discard any data already in the buffer.
    reply_buffer_.consume(reply_buffer_.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    socket_.async_receive(reply_buffer_.prepare(65536),
                          boost::bind(&pinger::handle_receive, this, _2));
  }

  void handle_receive(std::size_t length) {
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    reply_buffer_.commit(length);

    // Decode the reply packet.
    std::istream is(&reply_buffer_);
    ipv4_header ipv4_hdr;
    icmp_header icmp_hdr;
    is >> ipv4_hdr >> icmp_hdr;

    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == icmp_header::echo_reply &&
        icmp_hdr.identifier() == get_identifier() &&
        icmp_hdr.sequence_number() == sequence_number_) {
      // If this is the first reply, interrupt the five second timeout.
      if (num_replies_++ == 0) timer_.cancel();

      // Print out some information about the reply packet.
      chrono::steady_clock::time_point now = chrono::steady_clock::now();
      chrono::steady_clock::duration elapsed = now - time_sent_;
      std::cout << length - ipv4_hdr.header_length() << " bytes from "
                << ipv4_hdr.source_address()
                << ": icmp_seq=" << icmp_hdr.sequence_number()
                << ", ttl=" << ipv4_hdr.time_to_live() << ", time="
                << chrono::duration_cast<chrono::milliseconds>(elapsed).count()
                << std::endl;

      QString log =
          QString("%1  bytes from %2: icmp_seq=%3, ttl=%4, time=%5 at %6")
              .arg(
                  QString::number(length - ipv4_hdr.header_length()),
                  QString::fromStdString(ipv4_hdr.source_address().to_string()),
                  QString::number(icmp_hdr.sequence_number()),
                  QString::number(ipv4_hdr.time_to_live()),
                  QString::number(
                      chrono::duration_cast<chrono::milliseconds>(elapsed)
                          .count()),
                  QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
      QCoreApplication::postEvent(m_object, new PinglogEvent(log));
    }

    start_receive();
  }

  static unsigned short get_identifier() {
#if defined(ASIO_WINDOWS)
    return static_cast<unsigned short>(::GetCurrentProcessId());
#else
    return static_cast<unsigned short>(::getpid());
#endif
  }

  icmp::resolver resolver_;
  icmp::endpoint destination_;
  icmp::socket socket_;
  steady_timer timer_;
  unsigned short sequence_number_;
  chrono::steady_clock::time_point time_sent_;
  asio::streambuf reply_buffer_;
  std::size_t num_replies_;
  QObject* m_object;
  QString ip;
};
