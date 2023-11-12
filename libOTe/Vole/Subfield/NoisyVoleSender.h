#pragma once
// © 2022 Visa.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// This code implements features described in [Silver: Silent VOLE and Oblivious
// Transfer from Hardness of Decoding Structured LDPC Codes,
// https://eprint.iacr.org/2021/1150]; the paper is licensed under Creative
// Commons Attribution 4.0 International Public License
// (https://creativecommons.org/licenses/by/4.0/legalcode).

#include <libOTe/config.h>
#if defined(ENABLE_SILENT_VOLE) || defined(ENABLE_SILENTOT)

#include "cryptoTools/Common/BitVector.h"
#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Timer.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "libOTe/Tools/Coproto.h"
#include "libOTe/TwoChooseOne/OTExtInterface.h"

namespace osuCrypto::Subfield {
template <typename TypeTrait>
class NoisySubfieldVoleSender : public TimerAdapter {
 public:
  using F = typename TypeTrait::F;
  using G = typename TypeTrait::G;
  task<> send(F x, span<F> z, PRNG& prng,
                               OtReceiver& ot, Socket& chl) {
    MC_BEGIN(task<>, this, x, z, &prng, &ot, &chl,
             bv = BitVector((u8*)&x, sizeof(F) * 8),
             otMsg = AlignedUnVector<block>{sizeof(F) * 8}); // todo: sizeof(F) * 8

    setTimePoint("NoisyVoleSender.ot.begin");

    MC_AWAIT(ot.receive(bv, otMsg, prng, chl));
    setTimePoint("NoisyVoleSender.ot.end");

    MC_AWAIT(send(x, z, prng, otMsg, chl));

    MC_END();
  }

  task<> send(F x, span<F> z, PRNG& _,
              span<block> otMsg, Socket& chl) {
    if constexpr (!std::is_same_v<F, G> && sizeof(F) > 16) {
      return send1(x, z, _, otMsg, chl);
    } else {
      return send0(x, z, _, otMsg, chl);
    }
  }

  task<> send0(F x, span<F> z, PRNG& _,
                               span<block> otMsg, Socket& chl) {
    MC_BEGIN(task<>, this, x, z, prng = std::move(PRNG{}), otMsg, &chl, msg = Matrix<F>{},
             xb = BitVector{});

    if (otMsg.size() != sizeof(F) * 8) throw RTE_LOC;
    setTimePoint("NoisyVoleSender.main");

    memset(z.data(), 0, sizeof(F) * z.size());
    msg.resize(otMsg.size(), z.size(), AllocType::Uninitialized);

    MC_AWAIT(chl.recv(msg));

    setTimePoint("NoisyVoleSender.recvMsg");

    xb = BitVector((u8*)&x, sizeof(F) * 8);
    for (u64 i = 0; i < sizeof(F) * 8; ++i) {
      prng.SetSeed(otMsg[i], z.size());

      for (u64 j = 0; j < (u64)z.size(); ++j) {
        F bufj = TypeTrait::fromBlock(prng.mBuffer[j]);
        z[j] = z[j] + (xb[i] ? msg(i,j) - bufj : bufj);
      }
    }
    setTimePoint("NoisyVoleSender.done");

    MC_END();
  }

  task<> send1(F x, span<F> z, PRNG& _,
               span<block> otMsg, Socket& chl) {
    MC_BEGIN(task<>, this, x, z, prng = std::move(PRNG{}), otMsg, &chl, msg = Matrix<G>{},
             xb = BitVector{}, N = TypeTrait::size, k = (size_t)0);

    if (otMsg.size() != sizeof(F) * 8) throw RTE_LOC;
    setTimePoint("NoisyVoleSender.main");

    memset(z.data(), 0, sizeof(F) * z.size());

    msg.resize(N * sizeof(G) * 8, z.size(), AllocType::Uninitialized);
    MC_AWAIT(chl.recv(msg));

    for (; k < N; k++) {
      setTimePoint("NoisyVoleSender.recvMsg");

      xb = BitVector((u8 *) &(x[k]), sizeof(G) * 8);
      for (size_t i = 0; i < sizeof(G) * 8; ++i) {
        prng.SetSeed(otMsg[k*sizeof(G)*8 + i], 1 + (z.size() * sizeof(G)) / 16);
        G* buf = (G*)prng.mBuffer.data();

        for (size_t j = 0; j < z.size(); ++j) {
          z[j][k] = z[j][k] + (xb[i] ? msg(k*sizeof(G)*8 + i, j) - buf[j] : buf[j]);
        }
      }
    }
    setTimePoint("NoisyVoleSender.done");
    MC_END();
  }
};
}  // namespace osuCrypto

#endif