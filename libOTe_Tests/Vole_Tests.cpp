#include "Vole_Tests.h"
#include "libOTe/Vole/Noisy/NoisyVoleSender.h"
#include "libOTe/Vole/Noisy/NoisyVoleReceiver.h"
#include "libOTe/Vole/Silent/SilentVoleSender.h"
#include "libOTe/Vole/Silent/SilentVoleReceiver.h"
#include "libOTe/Vole/Subfield/NoisyVoleSender.h"
#include "libOTe/Vole/Subfield/NoisyVoleReceiver.h"
#include "libOTe/Vole/Subfield/SilentVoleSender.h"
#include "libOTe/Vole/Subfield/SilentVoleReceiver.h"
#include "cryptoTools/Network/Session.h"
#include "cryptoTools/Network/IOService.h"
#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Timer.h"
#include "cryptoTools/Common/Range.h"
#include "cryptoTools/Common/TestCollection.h"
#include "Common.h"
#include "coproto/Socket/BufferingSocket.h"

using namespace oc;

#include <libOTe/config.h>
using namespace tests_libOTe;


#if defined(ENABLE_SILENT_VOLE) || defined(ENABLE_SILENTOT)

void Vole_Noisy_test(const oc::CLP& cmd)
{
    Timer timer;
    timer.setTimePoint("start");
    u64 n = cmd.getOr("n", 123);
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();
    std::vector<block> y(n), z0(n), z1(n);
    prng.get<block>(y);

    NoisyVoleReceiver recv;
    NoisyVoleSender send;

    recv.setTimer(timer);
    send.setTimer(timer);

    //IOService ios;
    //auto chl1 = Session(ios, "localhost:1212", SessionMode::Server).addChannel();
    //auto chl0 = Session(ios, "localhost:1212", SessionMode::Client).addChannel();

    auto chls = cp::LocalAsyncSocket::makePair();
    timer.setTimePoint("net");


    BitVector recvChoice((u8*)&x, 128);
    std::vector<block> otRecvMsg(128);
    std::vector<std::array<block, 2>> otSendMsg(128);
    prng.get<std::array<block, 2>>(otSendMsg);
    for (u64 i = 0; i < 128; ++i)
        otRecvMsg[i] = otSendMsg[i][recvChoice[i]];
    timer.setTimePoint("ot");

    auto p0 = recv.receive(y, z0, prng, otSendMsg, chls[0]);
    auto p1 = send.send(x, z1, prng, otRecvMsg, chls[1]);

    eval(p0, p1);

    for (u64 i = 0; i < n; ++i)
    {
        if (y[i].gf128Mul(x) != (z0[i] ^ z1[i]))
        {
            throw RTE_LOC;
        }
    }
    timer.setTimePoint("done");

    //std::cout << timer << std::endl;

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
struct TypeTrait128
{
  using F = u128;
  using G = u128;

  static inline F fromBlock(const block& b) {
    conv128 c{};
    c.m = b;
    return c.u;
  }
  static inline F powerOf2(u64 power) {
    u128 ret = 1;
    ret <<= power;
    return ret;
  }
};

struct TypeTrait64
{
  using F = u64;
  using G = u64;

  union conv64 {
    u64 u;
    block m;
  };

  static inline F fromBlock(const block& b) {
    conv64 c{};
    c.m = b;
    return c.u;
  }
  static inline F powerOf2(u64 power) {
    u64 ret = 1;
    ret <<= power;
    return ret;
  }
};

struct F128: block {
  F128() : block(){

  }
  inline F128 operator+(const F128& rhs) const {
    return {};
  }
};

void Vole_Subfield_test(const oc::CLP& cmd)
{
    {
    Timer timer;
    timer.setTimePoint("start");
    u64 n = cmd.getOr("n", 123);
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    u128 x = prng.get();
    std::vector<u128> y(n);
    std::vector<u128> z0(n), z1(n);
    prng.get(y.data(), y.size());

    NoisySubfieldVoleReceiver<TypeTrait128> recv;
    NoisySubfieldVoleSender<TypeTrait128> send;

    recv.setTimer(timer);
    send.setTimer(timer);

    //IOService ios;
    //auto chl1 = Session(ios, "localhost:1212", SessionMode::Server).addChannel();
    //auto chl0 = Session(ios, "localhost:1212", SessionMode::Client).addChannel();

    auto chls = cp::LocalAsyncSocket::makePair();
    timer.setTimePoint("net");


    BitVector recvChoice((u8*)&x, 128);
    std::vector<block> otRecvMsg(128);
    std::vector<std::array<block, 2>> otSendMsg(128);
    prng.get<std::array<block, 2>>(otSendMsg);
    for (u64 i = 0; i < 128; ++i)
        otRecvMsg[i] = otSendMsg[i][recvChoice[i]];
    timer.setTimePoint("ot");

    auto p0 = recv.receive(y, z0, prng, otSendMsg, chls[0]);
    auto p1 = send.send(x, z1, prng, otRecvMsg, chls[1]);

    eval(p0, p1);

    for (u64 i = 0; i < n; ++i)
    {
        if (x * y[i] != (z1[i] - z0[i]))
        {
            throw RTE_LOC;
        }
    }
    timer.setTimePoint("done");

    //std::cout << timer << std::endl;
    }
}

#else 
void Vole_Noisy_test(const oc::CLP& cmd)
{
    throw UnitTestSkipped(
        "ENABLE_SILENT_VOLE not defined. "
    );
}
#endif

#ifdef ENABLE_SILENT_VOLE

namespace {
    void fakeBase(
        u64 n,
        u64 threads,
        PRNG& prng,
        block delta,
        SilentVoleReceiver& recver,
        SilentVoleSender& sender)
    {
        sender.configure(n, SilentBaseType::Base, 128);
        recver.configure(n, SilentBaseType::Base, 128);


        std::vector<std::array<block, 2>> msg2(sender.silentBaseOtCount());
        BitVector choices = recver.sampleBaseChoiceBits(prng);
        std::vector<block> msg(choices.size());

        if (choices.size() != msg2.size())
            throw RTE_LOC;

        for (auto& m : msg2)
        {
            m[0] = prng.get();
            m[1] = prng.get();
        }

        for (auto i : rng(msg.size()))
            msg[i] = msg2[i][choices[i]];

        auto y = recver.sampleBaseVoleVals(prng);;
        std::vector<block> c(y.size()), b(y.size());
        prng.get(c.data(), c.size());
        for (auto i : rng(y.size()))
        {
            b[i] = delta.gf128Mul(y[i]) ^ c[i];
        }
        sender.setSilentBaseOts(msg2, b);

        // fake base OTs.
        recver.setSilentBaseOts(msg, c);
    }

}

void Vole_Silent_QuasiCyclic_test(const oc::CLP& cmd)
{
#if defined(ENABLE_SILENTOT) && defined(ENABLE_BITPOLYMUL)
    Timer timer;
    timer.setTimePoint("start");
    u64 n = cmd.getOr("n", 102043);
    u64 nt = cmd.getOr("nt", std::thread::hardware_concurrency());
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();
    std::vector<block> c(n), z0(n), z1(n);

    SilentVoleReceiver recv;
    SilentVoleSender send;

    recv.mMultType = MultType::QuasiCyclic;
    send.mMultType = MultType::QuasiCyclic;

    recv.setTimer(timer);
    send.setTimer(timer);

    recv.mDebug = true;
    send.mDebug = true;

    auto chls = cp::LocalAsyncSocket::makePair();

    timer.setTimePoint("net");

    timer.setTimePoint("ot");
    fakeBase(n, nt, prng, x, recv, send);

    // c * x = z + m

    auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
    auto p1 = send.silentSend(x, z1, prng, chls[1]);

    eval(p0, p1);
    timer.setTimePoint("send");
    for (u64 i = 0; i < n; ++i)
    {
        if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
        {
            std::cout << "bad " << i << "\n  c[i] " << c[i] << " * x " << x << " = " << c[i].gf128Mul(x) << std::endl;
            std::cout << "  z0[i] " << z0[i] << " ^ z1 " << z1[i] << " = " << (z0[i] ^ z1[i]) << std::endl;
            throw RTE_LOC;
        }
    }
    timer.setTimePoint("done");
#else
    throw UnitTestSkipped("ENABLE_BITPOLYMUL not defined." LOCATION);
#endif
}

void Vole_Silent_Subfield_test(const oc::CLP& cmd) {
#if defined(ENABLE_SILENTOT)
  Timer timer;
  timer.setTimePoint("start");
  u64 n = cmd.getOr("n", 102043);
  u64 nt = cmd.getOr("nt", std::thread::hardware_concurrency());
  block seed = block(0, cmd.getOr("seed", 0));

  {
    PRNG prng(seed);
    u128 x = TypeTrait128::fromBlock(prng.get<block>());
    std::vector<u128> c(n), z0(n), z1(n);

    SilentSubfieldVoleReceiver<TypeTrait128> recv;
    SilentSubfieldVoleSender<TypeTrait128> send;

    recv.mMultType = MultType::ExConv7x24;
    send.mMultType = MultType::ExConv7x24;

    recv.setTimer(timer);
    send.setTimer(timer);

    recv.mDebug = true;
    send.mDebug = true;

    auto chls = cp::LocalAsyncSocket::makePair();

    timer.setTimePoint("net");

    timer.setTimePoint("ot");
//  fakeBase(n, nt, prng, x, recv, send);

    // c * x = z + m

    auto p0 = recv.silentReceive(span<u128>(c), span<u128>(z0), prng, chls[0]);
    auto p1 = send.silentSend(x, span<u128>(z1), prng, chls[1]);

    eval(p0, p1);
    timer.setTimePoint("send");
    for (u64 i = 0; i < n; ++i) {
      u128 left = c[i] * x;
      u128 right = z0[i] - z1[i];
      if (left != right) {
        std::cout << "bad " << i << "\n  c[i] " << u128ToString(c[i]) << " * x " << u128ToString(x) << " = "
                  << u128ToString(left) << std::endl;
        std::cout << "z0[i] " << u128ToString(z0[i]) << " ^ z1 " << u128ToString(z1[i]) << " = "
                  << u128ToString(right) << std::endl;
        throw RTE_LOC;
      }
    }
  }

  {
    PRNG prng(seed);
    u64 x = TypeTrait128::fromBlock(prng.get<block>());
    std::vector<u64> c(n), z0(n), z1(n);

    SilentSubfieldVoleReceiver<TypeTrait64> recv;
    SilentSubfieldVoleSender<TypeTrait64> send;

    recv.mMultType = MultType::ExConv7x24;
    send.mMultType = MultType::ExConv7x24;

    recv.setTimer(timer);
    send.setTimer(timer);

    recv.mDebug = true;
    send.mDebug = true;

    auto chls = cp::LocalAsyncSocket::makePair();

    timer.setTimePoint("net");

    timer.setTimePoint("ot");
//  fakeBase(n, nt, prng, x, recv, send);

    // c * x = z + m

    auto p0 = recv.silentReceive(span<u64>(c), span<u64>(z0), prng, chls[0]);
    auto p1 = send.silentSend(x, span<u64>(z1), prng, chls[1]);

    eval(p0, p1);
    timer.setTimePoint("send");
    for (u64 i = 0; i < n; ++i) {
      u64 left = c[i] * x;
      u64 right = z0[i] - z1[i];
      if (left != right) {
        std::cout << "bad " << i << "\n  c[i] " << c[i] << " * x " << x << " = " << left << std::endl;
        std::cout << "z0[i] " << z0[i] << " ^ z1 " << z1[i] << " = " << right << std::endl;
        throw RTE_LOC;
      }
    }
  }
  timer.setTimePoint("done");
#else
  throw UnitTestSkipped("not defined." LOCATION);
#endif
}

void Vole_Silent_paramSweep_test(const oc::CLP& cmd)
{

    Timer timer;
    timer.setTimePoint("start");
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();
    u64 threads = 0;


    auto chls = cp::LocalAsyncSocket::makePair();
    timer.setTimePoint("net");

    timer.setTimePoint("ot");

    //recv.mDebug = true;
    //send.mDebug = true;

    SilentVoleReceiver recv;
    SilentVoleSender send;
    // c * x = z + m

    //for (u64 n = 5000; n < 10000; ++n)
    for (u64 n : {12,/* 123,465,*/1642,/*4356,34254,*/93425})
    {
        std::vector<block> c(n), z0(n), z1(n);

        fakeBase(n, threads, prng, x, recv, send);

        recv.setTimer(timer);
        send.setTimer(timer);

        auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
        auto p1 = send.silentSend(x, z1, prng, chls[1]);
        timer.setTimePoint("send");

        eval(p0, p1);

        for (u64 i = 0; i < n; ++i)
        {
            if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
            {
                throw RTE_LOC;
            }
        }
        timer.setTimePoint("done");
    }
}

void Vole_Silent_Silver_test(const oc::CLP& cmd)
{

#ifdef ENABLE_INSECURE_SILVER
    Timer timer;
    timer.setTimePoint("start");
    u64 n = cmd.getOr("n", 102043);
    u64 nt = cmd.getOr("nt", std::thread::hardware_concurrency());
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();
    std::vector<block> c(n), z0(n), z1(n);

    SilentVoleReceiver recv;
    SilentVoleSender send;

    recv.mMultType = MultType::slv5;
    send.mMultType = MultType::slv5;

    recv.setTimer(timer);
    send.setTimer(timer);

    recv.mDebug = false;
    send.mDebug = false;

    auto chls = cp::LocalAsyncSocket::makePair();

    timer.setTimePoint("net");

    timer.setTimePoint("ot");
    fakeBase(n, nt, prng, x, recv, send);

    // c * x = z + m

    auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
    auto p1 = send.silentSend(x, z1, prng, chls[1]);

    eval(p0, p1);
    timer.setTimePoint("send");
    for (u64 i = 0; i < n; ++i)
    {
        if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
        {
            std::cout << "bad " << i << "\n  c[i] " << c[i] << " * x " << x << " = " << c[i].gf128Mul(x) << std::endl;
            std::cout << "  z0[i] " << z0[i] << " ^ z1 " << z1[i] << " = " << (z0[i] ^ z1[i]) << std::endl;
            throw RTE_LOC;
        }
    }
    timer.setTimePoint("done");
#endif
}



void Vole_Silent_baseOT_test(const oc::CLP& cmd)
{

    Timer timer;
    timer.setTimePoint("start");
    u64 n = 123;
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();



    auto chls = cp::LocalAsyncSocket::makePair();

    timer.setTimePoint("net");

    timer.setTimePoint("ot");

    //recv.mDebug = true;
    //send.mDebug = true;

    SilentVoleReceiver recv;
    SilentVoleSender send;
    // c * x = z + m

    //for (u64 n = 5000; n < 10000; ++n)
    {
        std::vector<block> c(n), z0(n), z1(n);


        recv.setTimer(timer);
        send.setTimer(timer);
        auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
        auto p1 = send.silentSend(x, z1, prng, chls[1]);

        eval(p0, p1);

        for (u64 i = 0; i < n; ++i)
        {
            if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
            {
                throw RTE_LOC;
            }
        }
        timer.setTimePoint("done");
    }
}



void Vole_Silent_mal_test(const oc::CLP& cmd)
{

    Timer timer;
    timer.setTimePoint("start");
    u64 n = 12343;
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();



    auto chls = cp::LocalAsyncSocket::makePair();
    timer.setTimePoint("net");

    timer.setTimePoint("ot");

    //recv.mDebug = true;
    //send.mDebug = true;

    SilentVoleReceiver recv;
    SilentVoleSender send;

    send.mMalType = SilentSecType::Malicious;
    recv.mMalType = SilentSecType::Malicious;
    // c * x = z + m

    //for (u64 n = 5000; n < 10000; ++n)
    {
        std::vector<block> c(n), z0(n), z1(n);


        recv.setTimer(timer);
        send.setTimer(timer);
        auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
        auto p1 = send.silentSend(x, z1, prng, chls[1]);


        eval(p0, p1);

        for (u64 i = 0; i < n; ++i)
        {
            if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
            {
                throw RTE_LOC;
            }
        }
        timer.setTimePoint("done");
    }
}


inline u64 eval(
    macoro::task<>& t1, macoro::task<>& t0,
    cp::BufferingSocket& s1, cp::BufferingSocket& s0)
{
    auto e = macoro::make_eager(macoro::when_all_ready(std::move(t0), std::move(t1)));

    u64 rounds = 0;

    {
        auto b1 = s1.getOutbound();
        if (b1)
        {
            s0.processInbound(*b1);
            ++rounds;
        }
    }

    u64 idx = 0;
    while (e.is_ready() == false)
    {
        if (idx % 2 == 0)
        {
            auto b0 = s0.getOutbound();
            if (!b0)
                throw RTE_LOC;
            s1.processInbound(*b0);

        }
        else
        {
            auto b1 = s1.getOutbound();
            if (!b1)
                throw RTE_LOC;
            s0.processInbound(*b1);
        }


        ++rounds;

        ++idx;
    }

    auto r = macoro::sync_wait(std::move(e));
    std::get<0>(r).result();
    std::get<1>(r).result();
    return rounds;
}


void Vole_Silent_Rounds_test(const oc::CLP& cmd)
{

    Timer timer;
    timer.setTimePoint("start");
    u64 n = 12343;
    block seed = block(0, cmd.getOr("seed", 0));
    PRNG prng(seed);

    block x = prng.get();


    cp::BufferingSocket chls[2];

    SilentVoleReceiver recv;
    SilentVoleSender send;

    send.mMalType = SilentSecType::SemiHonest;
    recv.mMalType = SilentSecType::SemiHonest;
    for (u64 jj : {0, 1})
    {

        send.configure(n, SilentBaseType::Base);
        recv.configure(n, SilentBaseType::Base);
        // c * x = z + m

        //for (u64 n = 5000; n < 10000; ++n)
        {

            recv.setTimer(timer);
            send.setTimer(timer);
            if (jj)
            {
                std::vector<block> c(n), z0(n), z1(n);
                auto p0 = recv.silentReceive(c, z0, prng, chls[0]);
                auto p1 = send.silentSend(x, z1, prng, chls[1]);


                auto rounds = eval(p0, p1, chls[1], chls[0]);
                if (rounds != 3)
                    throw std::runtime_error(std::to_string(rounds) + "!=3. " +COPROTO_LOCATION);


                for (u64 i = 0; i < n; ++i)
                {
                    if (c[i].gf128Mul(x) != (z0[i] ^ z1[i]))
                    {
                        throw RTE_LOC;
                    }
                }
            }
            else
            {


                auto p0 = send.genSilentBaseOts(prng, chls[0], x);
                auto p1 = recv.genSilentBaseOts(prng, chls[1]);

                auto rounds = eval(p0, p1, chls[1], chls[0]);
                if (rounds != 3)
                    throw RTE_LOC;

                p0 = send.silentSendInplace(x, n, prng, chls[0]);
                p1 = recv.silentReceiveInplace(n, prng, chls[1]);
                rounds = eval(p0, p1, chls[1], chls[0]);



                for (u64 i = 0; i < n; ++i)
                {
                    if (recv.mC[i].gf128Mul(x) != (send.mB[i] ^ recv.mA[i]))
                    {
                        throw RTE_LOC;
                    }
                }
            }

        }

        timer.setTimePoint("done");
    }
}

#else

namespace {
    void throwDisabled()
    {
        throw UnitTestSkipped(
            "ENABLE_SILENT_VOLE not defined. "
        );
    }
}


void Vole_Silent_QuasiCyclic_test(const oc::CLP& cmd) { throwDisabled(); }
void Vole_Silent_Silver_test(const oc::CLP& cmd) { throwDisabled(); }
void Vole_Silent_paramSweep_test(const oc::CLP& cmd) { throwDisabled(); }
void Vole_Silent_baseOT_test(const oc::CLP& cmd) { throwDisabled(); }
void Vole_Silent_mal_test(const oc::CLP& cmd) { throwDisabled(); }
void Vole_Silent_Rounds_test(const oc::CLP& cmd) { throwDisabled(); }

#endif
