/***
 *  $Id$
 **
 *  File: logger.hpp
 *  Created: May 11, 2018
 *
 *  Author: Jaroslaw Zola <jaroslaw.zola@hush.com>
 *  Copyright (c) 2018 Jaroslaw Zola
 *  Distributed under the Boost Software License, Version 1.0.
 *  See accompanying file LICENSE_BOOST.txt.
 *
 *  This file is part of mpix2.
 */

#ifndef MPIX2_LOGGER_HPP
#define MPIX2_LOGGER_HPP

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>


namespace mpix {

  class log_failed : public virtual std::exception {
      virtual const char* what() const throw() { return "mpix::log_failed"; }
  }; // class bad_sample

  struct null_ostream : public std::ostream {
      null_ostream() : std::ios(0), std::ostream(0) { }
  }; // struct null_ostream


  namespace log {

    inline std::string byte_to_size(unsigned long long int sz) {
        std::ostringstream ss;

        if (sz < 1024) ss << sz << "B";
        else if (sz < (1024 * 1024)) {
            ss << std::setprecision(4) << (static_cast<float>(sz) / 1024) << "KB";
        } else if (sz < (1024 * 1024 * 1024)) {
            ss << std::setprecision(4) << (static_cast<float>(sz) / (1024 * 1024)) << "MB";
        } else {
            ss << std::setprecision(4) << (static_cast<float>(sz) / (1024 * 1024 * 1024)) << "GB";
        }

        return ss.str();
    } // byte_to_size

    inline std::string sec_to_time(double t) {
        std::ostringstream ss;

        unsigned int tt = static_cast<unsigned int>(t);
        unsigned int ht = tt / 3600;
        unsigned int mt = (tt % 3600) / 60;
        unsigned int st = (tt % 3600) % 60;

        ss << t << "s (" << ht << "h" << mt << "m" << st << "s)";

        return ss.str();
    } // sec_to_time

    template <typename CharT>
    struct Sep : public std::numpunct<CharT> {
        virtual std::string do_grouping() const { return "\003"; }
    }; // struct Sep

    template <typename Int> inline std::string large_number(Int t) {
        std::ostringstream ss;
        ss.imbue(std::locale(ss.getloc(), new Sep<char>()));

        ss << t;

        return ss.str();
    } // large_number

  } // namespace log


  class Logger {
  public:
      enum Level { DEBUG, INFO, WARN, ERROR, QUIET };

      explicit Logger(int rank, std::ostream* os = &std::cout)
          : os_(os), has_os_(false), rank_(rank) {
          level_ = INFO;

          if (const char* env = std::getenv("MPIX_LOG")) {
              if (std::strncmp(env, "DEBUG", 5) == 0) level_ = DEBUG;
              else if (std::strncmp(env, "INFO", 4) == 0) level_ = INFO;
              else if (std::strncmp(env, "WARN", 4) == 0) level_ = WARN;
              else if (std::strncmp(env, "ERROR", 5) == 0) level_ = ERROR;
              else if (std::strncmp(env, "QUIET", 5) == 0) level_ = QUIET;
          }
      } // Logger

      Logger(int rank, const std::string& name) : Logger(rank) {
          if (!m_set_os__(name)) throw log_failed();
          name_ = name;
          has_os_ = true;
      } // Logger

      void operator=(Logger&& log) noexcept {
          os_ = log.os_;
          log.os_ = nullptr;
          has_os_ = log.has_os_;
          level_ = log.level_;
          name_ = std::move(log.name_);
          rank_ = log.rank_;
      } // Logger

      ~Logger() { if (has_os_) delete os_; }


      void rank(int r) {
          rank_ = r;
          if (has_os_) {
              delete os_;
              if (!m_set_os__(name_)) throw log_failed();
          }
      } // rank

      void level(Level l) { level_ = l; }

      std::ostream& debug(const std::string& who = "", int rank = 0) {
          if ((rank > -1) && (rank != rank_)) return nout_;
          if (level_ > DEBUG) return nout_;
          return m_header__(" DEBUG ", who);
      } // debug

      std::ostream& info(const std::string& who = "", int rank = 0) {
          if ((rank > -1) && (rank != rank_)) return nout_;
          if (level_ > INFO) return nout_;
          return m_header__(" INFO ", who);;
      } // info

      std::ostream& warn(const std::string& who = "", int rank = 0) {
          if ((rank > -1) && (rank != rank_)) return nout_;
          if (level_ > WARN) return nout_;
          return m_header__(" WARN ", who);
      } // warn

      std::ostream& error(const std::string& who = "", int rank = 0) {
          if ((rank > -1) && (rank != rank_)) return nout_;
          if (level_ == QUIET) return nout_;
          return m_header__(" ERROR ", who);
      } // error


  private:
      Logger(Logger&) = delete;
      void operator=(Logger&) = delete;

      bool m_set_os__(std::string name) {
          name = (name + std::to_string(rank_) + ".log");
          os_ = new std::ofstream(name);
          if (!(*os_)) return false;
          return true;
      } // m_set_os__

      std::ostream& m_header__(const char* h, const std::string& who) {
          auto t = std::chrono::system_clock::now();
          auto tt = std::chrono::system_clock::to_time_t(t);
          *os_ << std::put_time(std::localtime(&tt), "%Y-%m-%d %X") << " [" << rank_ << "]" << h;
          if (!who.empty()) *os_ << who << ": ";
          return *os_;
      } // m_header__

      std::ostream* os_ = &std::cout;
      bool has_os_ = false;

      null_ostream nout_;

      Level level_ = INFO;

      std::string name_;
      int rank_ = 0;

  }; // class Logger

} // namespace mpix

#endif // MPIX2_LOGGER_HPP
