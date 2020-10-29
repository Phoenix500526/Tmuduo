#ifndef TMUDUO_BASE_TIMESTAMP_H_
#define TMUDUO_BASE_TIMESTAMP_H_

#include <boost/operators.hpp>
#include "base/copyable.h"

namespace tmuduo {

class Timestamp : public tmuduo::copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp> {
 public:
  static const int kMicroSecondsPerSecond = 1000 * 1000;
  //不带参数的构造函数用于构造时间戳
  Timestamp() : microSecondsSinceEpoch_(0) {}
  explicit Timestamp(int64_t microSecondsSinceEpochArg)
      : microSecondsSinceEpoch_(microSecondsSinceEpochArg) {}
  ~Timestamp() = default;
  void swap(Timestamp& that) {
    std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }
  std::string toString() const;
  std::string toFormattedString(bool showMicroseconds = true) const;
  bool valid() const { return microSecondsSinceEpoch_ > 0; }
  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const {
    return static_cast<time_t>(microSecondsSinceEpoch_ /
                               kMicroSecondsPerSecond);
  }

  static Timestamp now();
  static Timestamp invalid() { return Timestamp(); }
  static Timestamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }
  static Timestamp fromUnixTime(time_t t, int microseconds) {
    return Timestamp(static_cast<int64_t>(t) * kMicroSecondsPerSecond +
                     microseconds);
  }

 private:
  int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

//此处不采用重载运算符的原因是因为 timeDifference 和 addTime
//的返回值和参数类型并不一致
//这实际上违反了运算符的语义：例如 a + b = c 中，a，b，c 应该是同一种类型
inline double timeDifference(Timestamp high, Timestamp low) {
  int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
  return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
  int64_t delta =
      static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}  // namespace tmuduo

#endif  // TMUDUO_BASE_TIMESTAMP_H_