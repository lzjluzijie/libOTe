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

  static OC_FORCEINLINE F fromBlock(const block &b) {
    conv128 c{};
    c.m = b;
    return c.u;
  }
  static OC_FORCEINLINE F pow(u64 power) {
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

  static OC_FORCEINLINE F fromBlock(const block &b) {
    conv64 c{};
    c.m = b;
    return c.u;
  }
  static OC_FORCEINLINE F pow(u64 power) {
    u64 ret = 1;
    ret <<= power;
    return ret;
  }
};

// array
template<typename T, size_t N>
struct Vec {
  std::array<T, N> v;
  OC_FORCEINLINE Vec operator+(const Vec &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] + rhs.v[i];
    }
    return ret;
  }

  OC_FORCEINLINE Vec operator-(const Vec &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] - rhs.v[i];
    }
    return ret;
  }

  OC_FORCEINLINE Vec operator*(const T &rhs) const {
    Vec ret;
    for (u64 i = 0; i < N; ++i) {
      ret.v[i] = v[i] * rhs;
    }
    return ret;
  }

  OC_FORCEINLINE T operator[](u64 idx) const {
    return v[idx];
  }

  OC_FORCEINLINE T &operator[](u64 idx) {
    return v[idx];
  }

  OC_FORCEINLINE bool operator==(const Vec &rhs) const {
    for (u64 i = 0; i < N; ++i) {
      if (v[i] != rhs.v[i]) return false;
    }
    return true;
  }

  OC_FORCEINLINE bool operator!=(const Vec &rhs) const {
    return !(*this == rhs);
  }
};

// TypeTraitVec for array of integers
template<typename T, size_t N>
struct TypeTraitVec {
  using F = Vec<T, N>;
  using G = T;

  static OC_FORCEINLINE F fromBlock(const block &b) {
    PRNG prng(b, N * sizeof(T));
    F ret{};
    memcpy(ret.v.data(), prng.mBuffer.data(), N * sizeof(T));
    return ret;
  }
  static OC_FORCEINLINE F pow(u64 power) {
    F ret{};
    power = power % (N * sizeof(T) * 8);
    ret[power / (sizeof(T) * 8)] = 1 << (power % (sizeof(T) * 8));
    return ret;
  }
};
