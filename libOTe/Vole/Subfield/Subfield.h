#include "libOTe/Vole/Noisy/NoisyVoleSender.h"

using namespace oc;

using u128 = unsigned __int128;
union conv128 {
  u128 u;
  block m;
};
inline std::string u128ToString(u128 value) {
  if (value == 0) {
    return "0";
  }

  std::string result;
  while (value > 0) {
    uint64_t digit = value % 10;
    result.push_back(static_cast<char>('0' + digit));
    value /= 10;
  }
  reverse(result.begin(), result.end());
  return result;
}
struct TypeTrait128 {
  using F = u128;
  using G = u128;

  static inline F fromBlock(const block &b) {
    conv128 c{};
    c.m = b;
    return c.u;
  }
  static inline F pow(u64 power) {
    u128 ret = 1;
    ret <<= power;
    return ret;
  }
};

struct TypeTrait64 {
  using F = u64;
  using G = u64;

  union conv64 {
    u64 u;
    block m;
  };

  static inline F fromBlock(const block &b) {
    conv64 c{};
    c.m = b;
    return c.u;
  }
  static inline F pow(u64 power) {
    u64 ret = 1;
    ret <<= power;
    return ret;
  }
};

template<typename T, size_t N>
struct Vec {
  std::array<T, N> v;
  inline Vec operator+(const Vec &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] + rhs.v[i];
    }
    return ret;
  }

  inline Vec operator-(const Vec &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] - rhs.v[i];
    }
    return ret;
  }

  inline Vec operator*(const T &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] * rhs;
    }
    return ret;
  }

  inline T operator[](u64 idx) const {
    return v[idx];
  }

  inline T &operator[](u64 idx) {
    return v[idx];
  }

  inline bool operator==(const Vec &rhs) const {
    for (u64 i = 0; i < N; ++i) {
      if (v[i] != rhs.v[i]) return false;
    }
    return true;
  }

  inline bool operator!=(const Vec &rhs) const {
    return !(*this == rhs);
  }
};

struct TypeTraitVec {
  static constexpr u64 N = 4;
  using F = Vec<u32, N>;
  using G = u32;

  union conv {
    F u;
    block m;
  };

  static inline F fromBlock(const block &b) {
    conv c{};
    c.m = b;
    return c.u;
  }
  static inline F pow(u64 power) {
    F ret{};
    power = power % 128;
    ret[power/ 32] = 1 << (power % 32);
    return ret;
  }
};
