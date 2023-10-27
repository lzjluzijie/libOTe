#include "libOTe/Vole/Noisy/NoisyVoleSender.h"

using namespace oc;

struct F128 {
  block b;
  F128() = default;
  explicit F128(const block &b) : b(b) {}
  OC_FORCEINLINE F128 operator+(const F128 &rhs) const {
    F128 ret;
    ret.b = b ^ rhs.b;
    return ret;
  }
  OC_FORCEINLINE F128 operator-(const F128 &rhs) const {
    F128 ret;
    ret.b = b ^ rhs.b;
    return ret;
  }
  OC_FORCEINLINE F128 operator*(const F128 &rhs) const {
    F128 ret;
    ret.b = b.gf128Mul(rhs.b);
    return ret;
  }
  OC_FORCEINLINE bool operator==(const F128 &rhs) const {
    return b == rhs.b;
  }
  OC_FORCEINLINE bool operator!=(const F128 &rhs) const {
    return b != rhs.b;
  }
};

using u128 = unsigned __int128;
union conv128 {
  u128 u;
  block m;
};
OC_FORCEINLINE u128 fromBlock(const block &b) {
  conv128 c{};
  c.m = b;
  return c.u;
}
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
  static constexpr size_t bitsG = sizeof(T) * 8;
  static constexpr size_t bitsF = N * bitsG;
  static constexpr size_t bytesF = N * sizeof(T);
  static constexpr size_t sizeBlocks = (N * sizeof(T) + sizeof(block) - 1) / sizeof(block);
  union Buf {
    F f;
    block b[sizeBlocks];
  };

  static OC_FORCEINLINE F fromBlock(const block &b) {
    if (N * sizeof(T) <= sizeof(block)) {
      F ret;
      memcpy(ret.v.data(), &b, bytesF);
      return ret;
    } else {
      Buf buf;
      for (u64 i = 0; i < sizeBlocks; ++i) {
        buf.b[i] = b + block(i, i);
      }
      mAesFixedKey.hashBlocks(buf.b, sizeBlocks, buf.b);
      return buf.f;
    }
  }

  static OC_FORCEINLINE F pow(u64 power) {
    F ret{};
    power = power % bitsF;
    ret[power / bitsG] = 1 << (power % bitsG);
    return ret;
  }
};
