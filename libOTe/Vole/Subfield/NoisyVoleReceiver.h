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

#include "cryptoTools/Common/Defines.h"
#include "cryptoTools/Common/Timer.h"
#include "cryptoTools/Crypto/PRNG.h"
#include "libOTe/Tools/Coproto.h"
#include "libOTe/TwoChooseOne/OTExtInterface.h"

namespace osuCrypto {

template <typename TypeTrait>
class NoisySubfieldVoleReceiver : public TimerAdapter {
 public:
  using F = typename TypeTrait::F;
  using G = typename TypeTrait::G;
  task<> receive(span<G> y, span<F> z, PRNG& prng,
                                    OtSender& ot, Socket& chl) {
    MC_BEGIN(task<>, this, y, z, &prng, &ot, &chl,
             otMsg = AlignedUnVector<std::array<block, 2>>{sizeof(F) * 8}); // todo: sizeof(F) * 8

    setTimePoint("NoisyVoleReceiver.ot.begin");

    MC_AWAIT(ot.send(otMsg, prng, chl));

    setTimePoint("NoisyVoleReceiver.ot.end");

    MC_AWAIT(receive(y, z, prng, otMsg, chl));

    MC_END();
  }

  task<> receive(span<G> y, span<F> z, PRNG& _,
                                    span<std::array<block, 2>> otMsg,
                                    Socket& chl) {
    MC_BEGIN(task<>, this, y, z, otMsg, &chl, msg = Matrix<F>{},
             prng = std::move(PRNG{})
    );

    if (otMsg.size() != sizeof(F) * 8) throw RTE_LOC;
    if (y.size() != z.size()) throw RTE_LOC;
    if (z.size() == 0) throw RTE_LOC;

    setTimePoint("NoisyVoleReceiver.begin");

    memset(z.data(), 0, sizeof(F) * z.size());
    msg.resize(otMsg.size(), z.size(), AllocType::Uninitialized);

    for (u64 ii = 0; ii < sizeof(F) * 8; ++ii) {
      prng.SetSeed(otMsg[ii][0], z.size());
      auto& buffer = prng.mBuffer;
      auto pow = TypeTrait::pow(ii);
      for (u64 j = 0; j < (u64)y.size(); ++j) {
        auto bufj = TypeTrait::fromBlock(buffer[j]);
        z[j] = z[j] + bufj;
        F yy = pow * y[j];

        msg(ii, j) = yy + bufj;
      }

      prng.SetSeed(otMsg[ii][1], z.size());

      for (u64 j = 0; j < (u64)y.size(); ++j) {
        // enc one message under the OT msg.
        msg(ii, j) = msg(ii, j) + TypeTrait::fromBlock(buffer[j]);
      }
    }

    MC_AWAIT(chl.send(std::move(msg)));
    // chl.asyncSend(std::move(msg));
    setTimePoint("NoisyVoleReceiver.done");

    MC_END();
  }

};

}  // namespace osuCrypto
#endif
