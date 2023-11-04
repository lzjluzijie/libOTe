#include "libOTe/Vole/Noisy/NoisyVoleSender.h"
#include "cryptoTools/Common/BitIterator.h"

namespace osuCrypto::Subfield {

    struct F128 {
        block b;
        F128() = default;
        explicit F128(const block& b) : b(b) {}
        OC_FORCEINLINE F128 operator+(const F128& rhs) const {
            F128 ret;
            ret.b = b ^ rhs.b;
            return ret;
        }
        OC_FORCEINLINE F128 operator-(const F128& rhs) const {
            F128 ret;
            ret.b = b ^ rhs.b;
            return ret;
        }
        OC_FORCEINLINE F128 operator*(const F128& rhs) const {
            F128 ret;
            ret.b = b.gf128Mul(rhs.b);
            return ret;
        }
        OC_FORCEINLINE bool operator==(const F128& rhs) const {
            return b == rhs.b;
        }
        OC_FORCEINLINE bool operator!=(const F128& rhs) const {
            return b != rhs.b;
        }
    };

    //using u128 = __int128;
    //union conv128 {
    //  u128 u;
    //  block m;
    //};
    //OC_FORCEINLINE u128 fromBlock(const block &b) {
    //  conv128 c{};
    //  c.m = b;
    //  return c.u;
    //}
    //inline std::string u128ToString(u128 value) {
    //  if (value == 0) {
    //    return "0";
    //  }
    //
    //  std::string result;
    //  while (value > 0) {
    //    uint64_t digit = value % 10;
    //    result.push_back(static_cast<char>('0' + digit));
    //    value /= 10;
    //  }
    //  reverse(result.begin(), result.end());
    //  return result;
    //}
    //struct TypeTrait128 {
    //  using F = u128;
    //  using G = u128;
    //
    //  static OC_FORCEINLINE F fromBlock(const block &b) {
    //    conv128 c{};
    //    c.m = b;
    //    return c.u;
    //  }
    //  static OC_FORCEINLINE F pow(u64 power) {
    //    u128 ret = 1;
    //    ret <<= power;
    //    return ret;
    //  }
    //};

    struct TypeTrait64 {
        using F = u64;
        using G = u64;

        static OC_FORCEINLINE F fromBlock(const block& b) {
            return b.get<F>()[0];
        }
        static OC_FORCEINLINE F pow(u64 power) {
            F ret = 1;
            ret <<= power;
            return ret;
        }
    };

    // array
    template<typename T, size_t N>
    struct Vec {
        std::array<T, N> v;
        OC_FORCEINLINE Vec operator+(const Vec& rhs) const {
            Vec ret;
            for (u64 i = 0; i < N; ++i) {
                ret.v[i] = v[i] + rhs.v[i];
            }
            return ret;
        }

        OC_FORCEINLINE Vec operator-(const Vec& rhs) const {
            Vec ret;
            for (u64 i = 0; i < N; ++i) {
                ret.v[i] = v[i] - rhs.v[i];
            }
            return ret;
        }

        OC_FORCEINLINE Vec operator*(const T& rhs) const {
            Vec ret;
            for (u64 i = 0; i < N; ++i) {
                ret.v[i] = v[i] * rhs;
            }
            return ret;
        }

        OC_FORCEINLINE T operator[](u64 idx) const {
            return v[idx];
        }

        OC_FORCEINLINE T& operator[](u64 idx) {
            return v[idx];
        }

        OC_FORCEINLINE bool operator==(const Vec& rhs) const {
            for (u64 i = 0; i < N; ++i) {
                if (v[i] != rhs.v[i]) return false;
            }
            return true;
        }

        OC_FORCEINLINE bool operator!=(const Vec& rhs) const {
            return !(*this == rhs);
        }
    };

    // TypeTraitVec for array of integers
    template<typename T, size_t N>
    struct TypeTraitVec {
        using F = Vec<T, N>;
        using G = T;
        static constexpr size_t bitsG = sizeof(T) * 8;
        static constexpr size_t bitsF = sizeof(F) * 8;
        static constexpr size_t bytesF = sizeof(F);
        static constexpr size_t sizeBlocks = (bytesF + sizeof(block) - 1) / sizeof(block);
        static constexpr size_t size = N;

        static OC_FORCEINLINE F fromBlock(const block& b) {
            F ret;
            if (N * sizeof(T) <= sizeof(block)) {
                memcpy(ret.v.data(), &b, bytesF);
                return ret;
            }
            else {
                std::array<block, sizeBlocks> buf;
                for (u64 i = 0; i < sizeBlocks; ++i) {
                    buf[i] = b + block(i, i);
                }
                mAesFixedKey.hashBlocks<sizeBlocks>(buf.data(), buf.data());
                memcpy(&ret, &buf, sizeof(F));
                return ret;
            }
        }

        static OC_FORCEINLINE F pow(u64 power) {
            F ret;
            memset(&ret, 0, sizeof(ret));
            *BitIterator((u8*)&ret, power) = 1;
            return ret;
        }
    };

}
