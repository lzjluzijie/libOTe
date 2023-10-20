#include "ExConvCode_Tests.h"
#include "libOTe/Tools/ExConvCode/ExConvCode.h"
#include "libOTe/Tools/ExConvCode/ExConvCode.h"
#include <iomanip>


namespace osuCrypto
{
    void ExConvCode_encode_basic_test(const oc::CLP& cmd)
    {

        auto k = cmd.getOr("k", 16ul);
        auto R = cmd.getOr("R", 2.0);
        auto n = cmd.getOr<u64>("n", k * R);
        auto bw = cmd.getOr("bw", 7);
        auto aw = cmd.getOr("aw", 8);

        bool v = cmd.isSet("v");

        for (auto sys : {/* false,*/ true })
        {



            ExConvCode code;
            code.config(k, n, bw, aw, sys);

            auto A = code.getA();
            auto B = code.getB();
            auto G = B * A;

            std::vector<block> m0(k), m1(k), a1(n);

            if (v)
            {
                std::cout << "B\n" << B << std::endl << std::endl;
                std::cout << "A'\n" << code.getAPar() << std::endl << std::endl;
                std::cout << "A\n" << A << std::endl << std::endl;
                std::cout << "G\n" << G << std::endl;

            }

            const auto c0 = [&]() {
                std::vector<block> c0(n);
                PRNG prng(ZeroBlock);
                prng.get(c0.data(), c0.size());
                return c0;
            }();

            auto a0 = c0;
            auto aa0 = a0;
            std::vector<u8> aa1(n);
            for (u64 i = 0; i < n; ++i)
            {
                aa1[i] = aa0[i].get<u8>(0);
            }
            if (code.mSystematic)
            {
                code.accumulate<block>(span<block>(a0.begin() + k, a0.begin() + n));
                code.accumulate<block, u8>(
                    span<block>(aa0.begin() + k, aa0.begin() + n),
                    span<u8>(aa1.begin() + k, aa1.begin() + n)
                    );

                for (u64 i = 0; i < n; ++i)
                {
                    if (aa0[i] != a0[i])
                        throw RTE_LOC;
                    if (aa1[i] != a0[i].get<u8>(0))
                        throw RTE_LOC;
                }
            }
            else
            {
                code.accumulate<block>(a0);
            }
            A.multAdd(c0, a1);
            //A.leftMultAdd(c0, a1);
            if (a0 != a1)
            {
                if (v)
                {

                    for (u64 i = 0; i < k; ++i)
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << (a0[i]) << " ";
                    std::cout << "\n";
                    for (u64 i = 0; i < k; ++i)
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << (a1[i]) << " ";
                    std::cout << "\n";
                }

                throw RTE_LOC;
            }



            for (u64 q = 0; q < n; ++q)
            {
                std::vector<block> c0(n);
                c0[q] = AllOneBlock;

                //auto q = 0;
                auto cc = c0;
                auto cc1 = c0;
                auto mm1 = m1;
                
                std::vector<u8> cc2(cc1.size()), mm2(mm1.size());
                for (u64 i = 0; i < n; ++i)
                    cc2[i] = cc1[i].get<u8>(0);
                for (u64 i = 0; i < k; ++i)
                    mm2[i] = mm1[i].get<u8>(0);
                //std::vector<block> cc(n);
                //cc[q] = AllOneBlock;
                std::fill(m0.begin(), m0.end(), ZeroBlock);
                B.multAdd(cc, m0);


                if (code.mSystematic)
                {
                    std::copy(cc.begin(), cc.begin() + k, m1.begin());
                    code.mExpander.expand<block, true>(
                        span<block>(cc.begin() + k, cc.end()),
                        m1);
                    //for (u64 i = 0; i < k; ++i)
                    //    m1[i] ^= cc[i];
                    std::copy(cc1.begin(), cc1.begin() + k, mm1.begin());
                    std::copy(cc2.begin(), cc2.begin() + k, mm2.begin());

                    code.mExpander.expand<block, u8, true>(
                        span<block>(cc1.begin() + k, cc1.end()),
                        span<u8>(cc2.begin() + k, cc2.end()),
                        mm1, mm2);
                }
                else
                {
                    code.mExpander.expand<block>(cc, m1);
                }
                if (m0 != m1)
                {

                    std::cout << "B\n" << B << std::endl << std::endl;
                    for (u64 i = 0; i < n; ++i)
                        std::cout << (c0[i].get<u8>(0) & 1) << " ";
                    std::cout << std::endl;

                    std::cout << "exp act " << q << "\n";
                    for (u64 i = 0; i < k; ++i)
                    {
                        std::cout << (m0[i].get<u8>(0) & 1) << " " << (m1[i].get<u8>(0) & 1) << std::endl;
                    }
                    throw RTE_LOC;
                }

                if (code.mSystematic)
                {
                    if (mm1 != m1)
                        throw RTE_LOC;

                    for (u64 i = 0; i < k; ++i)
                        if (mm2[i] != m1[i].get<u8>(0))
                            throw RTE_LOC;
                }
            }

            //for (u64 q = 0; q < n; ++q)
            {
                auto q = 0;

                //std::fill(c0.begin(), c0.end(), ZeroBlock);
                //c0[q] = AllOneBlock;
                auto cc = c0;
                auto cc1 = c0;
                std::vector<u8> cc2(cc1.size());
                for (u64 i = 0; i < n; ++i)
                    cc2[i] = cc1[i].get<u8>(0);


                std::fill(m0.begin(), m0.end(), ZeroBlock);
                G.multAdd(c0, m0);

                if (code.mSystematic)
                {
                    code.dualEncode<block>(cc);
                    std::copy(cc.begin(), cc.begin() + k, m1.begin());
                }
                else
                {
                    code.dualEncode<block>(cc, m1);
                }

                if (m0 != m1)
                {
                    std::cout << "G\n" << G << std::endl << std::endl;
                    for (u64 i = 0; i < n; ++i)
                        std::cout << (c0[i].get<u8>(0) & 1) << " ";
                    std::cout << std::endl;

                    std::cout << "exp act " << q << "\n";
                    for (u64 i = 0; i < k; ++i)
                    {
                        std::cout << (m0[i].get<u8>(0) & 1) << " " << (m1[i].get<u8>(0) & 1) << std::endl;
                    }
                    throw RTE_LOC;
                }


                if (code.mSystematic)
                {
                    code.dualEncode2<block, u8>(cc1, cc2);

                    for (u64 i = 0; i < k; ++i)
                    {
                        if (cc1[i] != cc[i])
                            throw RTE_LOC;
                        if (cc2[i] != cc[i].get<u8>(0))
                            throw RTE_LOC;
                    }
                }
            }
        }
    }

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
static inline u128 fromBlock(const block& b) {
  conv128 c{};
  c.m = b;
  return c.u;
}

void ExConvCode_encode_u128_test(const oc::CLP& cmd)
    {
//      {
//        u64 n = 1024;
//        ExConvCode code;
//        code.config(n / 2, n, 7, 24, true);
//
//        PRNG prng(ZeroBlock);
//        block delta = prng.get<block>();
//        std::vector<block> y(n), z0(n), z1(n);
//        prng.get(y.data(), y.size());
//        prng.get(z0.data(), z0.size());
//        for (u64 i = 0; i < n; ++i)
//        {
//          z1[i] = z0[i] ^ delta.gf128Mul(y[i]);
//        }
//
////        y.resize(2*n);
////        z0.resize(2*n);
////        z1.resize(2*n);
//
//        code.dualEncode<block>(z1);
//        code.dualEncode<block>(z0);
//        code.dualEncode<block>(y);
//
//        for (u64 i = 0; i < n; ++i)
//        {
//          block left = delta.gf128Mul(y[i]);
//          block right = z1[i] ^ z0[i];
//          if (left != right)
//            throw RTE_LOC;
//        }
//      }

//      {
//        u64 n = 1024;
//        ExConvCode code;
//        code.config(n / 2, n, 7, 24, true);
//
//        PRNG prng(ZeroBlock);
//        u8 delta = 111;
//        std::vector<u8> y(n), z0(n), z1(n);
//        prng.get(y.data(), y.size());
//        prng.get(z0.data(), z0.size());
//        for (u64 i = 0; i < n; ++i)
//        {
//          z1[i] = z0[i] + delta * y[i];
//        }
//
////        y.resize(2*n);
////        z0.resize(2*n);
////        z1.resize(2*n);
//
//        code.dualEncode<u8>(z1);
//        code.dualEncode<u8>(z0);
//        code.dualEncode<u8>(y);
////        code.dualEncode2<u8, u8>(z0, y);
//
//        for (u64 i = 0; i < n; ++i)
//        {
//          u8 left = delta * y[i];
//          u8 right = z1[i] - z0[i];
//          if (left != right)
//            throw RTE_LOC;
//        }
//      }

      {
        u64 n = 1024;
        ExConvCode code;
        code.config(n / 2, n, 7, 24, true);

        PRNG prng(ZeroBlock);
        u128 delta = fromBlock(prng.get<block>());
        std::vector<u128> y(n), z0(n), z1(n);
        prng.get(y.data(), y.size());
        prng.get(z0.data(), z0.size());
        for (u64 i = 0; i < n; ++i)
        {
          z1[i] = z0[i] + delta * y[i];
        }

//        y.resize(2*n);
//        z0.resize(2*n);
//        z1.resize(2*n);

        code.dualEncode<u128>(z1);
        code.dualEncode<u128>(z0);
        code.dualEncode<u128>(y);

        for (u64 i = 0; i < n; ++i)
        {
          u128 left = delta * y[i];
          u128 right = z1[i] - z0[i];
          if (left != right)
            throw RTE_LOC;
        }
      }

//      {
//
//        auto k = cmd.getOr("k", 16ul);
//        auto R = cmd.getOr("R", 2.0);
//        auto n = cmd.getOr<u64>("n", k * R);
//        auto bw = cmd.getOr("bw", 7);
//        auto aw = cmd.getOr("aw", 8);
//
//        bool v = cmd.isSet("v");
//
//        for (auto sys : {/* false,*/ true })
//        {
//
//
//
//          ExConvCode code;
//          code.config(k, n, bw, aw, sys);
//
//          auto A = code.getA();
//          auto B = code.getB();
//          auto G = B * A;
//
//          std::vector<u128> m0(k), m1(k), a1(n);
//
//          if (v)
//          {
//            std::cout << "B\n" << B << std::endl << std::endl;
//            std::cout << "A'\n" << code.getAPar() << std::endl << std::endl;
//            std::cout << "A\n" << A << std::endl << std::endl;
//            std::cout << "G\n" << G << std::endl;
//
//          }
//
//          const auto c0 = [&]() {
//            std::vector<u128> c0(n);
//            PRNG prng(ZeroBlock);
//            prng.get(c0.data(), c0.size());
//            return c0;
//          }();
//
//          auto a0 = c0;
//          auto aa0 = a0;
//          std::vector<u8> aa1(n);
//          for (u64 i = 0; i < n; ++i)
//          {
//            aa1[i] = aa0[i];
//          }
//          if (code.mSystematic)
//          {
//            code.accumulate<u128>(span<u128>(a0.begin() + k, a0.begin() + n));
//            code.accumulate<u128, u8>(
//                span<u128>(aa0.begin() + k, aa0.begin() + n),
//                span<u8>(aa1.begin() + k, aa1.begin() + n)
//            );
//
//            for (u64 i = 0; i < n; ++i)
//            {
//              if (aa0[i] != a0[i])
//                throw RTE_LOC;
//              if (aa1[i] != u8(a0[i]))
//                throw RTE_LOC;
//            }
//          }
//          else
//          {
//            code.accumulate<u128>(a0);
//          }
//          A.multAdd(c0, a1);
//          //A.leftMultAdd(c0, a1);
//          if (a0 != a1)
//          {
//            if (v)
//            {
//
//              for (u64 i = 0; i < k; ++i)
//                std::cout << std::hex << std::setw(2) << std::setfill('0') << u8(a0[i]) << " ";
//              std::cout << "\n";
//              for (u64 i = 0; i < k; ++i)
//                std::cout << std::hex << std::setw(2) << std::setfill('0') << u8(a1[i]) << " ";
//              std::cout << "\n";
//            }
//
//            throw RTE_LOC;
//          }
//
//
//
//          for (u64 q = 0; q < n; ++q)
//          {
//            std::vector<u128> c0(n);
//            c0[q] = -1;
//
//            //auto q = 0;
//            auto cc = c0;
//            auto cc1 = c0;
//            auto mm1 = m1;
//
//            std::vector<u8> cc2(cc1.size()), mm2(mm1.size());
//            for (u64 i = 0; i < n; ++i)
//              cc2[i] = cc1[i];
//            for (u64 i = 0; i < k; ++i)
//              mm2[i] = mm1[i];
//            //std::vector<block> cc(n);
//            //cc[q] = AllOneBlock;
//            std::fill(m0.begin(), m0.end(), 0);
//            B.multAdd(cc, m0);
//
//
//            if (code.mSystematic)
//            {
//              std::copy(cc.begin(), cc.begin() + k, m1.begin());
//              code.mExpander.expand<u128, true>(
//                  span<u128>(cc.begin() + k, cc.end()),
//                  m1);
//              //for (u64 i = 0; i < k; ++i)
//              //    m1[i] ^= cc[i];
//              std::copy(cc1.begin(), cc1.begin() + k, mm1.begin());
//              std::copy(cc2.begin(), cc2.begin() + k, mm2.begin());
//
//              code.mExpander.expand<u128, u8, true>(
//                  span<u128>(cc1.begin() + k, cc1.end()),
//                  span<u8>(cc2.begin() + k, cc2.end()),
//                  mm1, mm2);
//            }
//            else
//            {
//              code.mExpander.expand<u128>(cc, m1);
//            }
//            if (m0 != m1)
//            {
//
//              std::cout << "B\n" << B << std::endl << std::endl;
//              for (u64 i = 0; i < n; ++i)
//                std::cout << (u8(m0[i]) & 1) << " ";
//              std::cout << std::endl;
//
//              std::cout << "exp act " << q << "\n";
//              for (u64 i = 0; i < k; ++i)
//              {
//                std::cout << (u8(m0[i]) & 1) << " " << (u8(m1[i]) & 1) << std::endl;
//              }
//              throw RTE_LOC;
//            }
//
//            if (code.mSystematic)
//            {
//              if (mm1 != m1)
//                throw RTE_LOC;
//
//              for (u64 i = 0; i < k; ++i)
//                if (mm2[i] != u8(m1[i]))
//                  throw RTE_LOC;
//            }
//          }
//
//          //for (u64 q = 0; q < n; ++q)
//          {
//            auto q = 0;
//
//            //std::fill(c0.begin(), c0.end(), ZeroBlock);
//            //c0[q] = AllOneBlock;
//            auto cc = c0;
//            auto cc1 = c0;
//            std::vector<u8> cc2(cc1.size());
//            for (u64 i = 0; i < n; ++i)
//              cc2[i] = cc1[i];
//
//
//            std::fill(m0.begin(), m0.end(), 0);
//            G.multAdd(c0, m0);
//
//            if (code.mSystematic)
//            {
//              code.dualEncode<u128>(cc);
//              std::copy(cc.begin(), cc.begin() + k, m1.begin());
//            }
//            else
//            {
//              code.dualEncode<u128>(cc, m1);
//            }
//
//            if (m0 != m1)
//            {
//              std::cout << "G\n" << G << std::endl << std::endl;
//              for (u64 i = 0; i < n; ++i)
//                std::cout << (u8(c0[i]) & 1) << " ";
//              std::cout << std::endl;
//
//              std::cout << "exp act " << q << "\n";
//              for (u64 i = 0; i < k; ++i)
//              {
//                std::cout << (u8(m0[i]) & 1) << " " << (u8(m1[i]) & 1) << std::endl;
//              }
//              throw RTE_LOC;
//            }
//
//
//            if (code.mSystematic)
//            {
//              code.dualEncode2<u128, u8>(cc1, cc2);
//
//              for (u64 i = 0; i < k; ++i)
//              {
//                if (cc1[i] != cc[i])
//                  throw RTE_LOC;
//                if (cc2[i] != u8(cc[i]))
//                  throw RTE_LOC;
//              }
//            }
//          }
//        }
//      }
    }
}