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
template <typename G, typename F>
class NoisySubfieldVoleSender : public TimerAdapter {
 public:
  task<> send(F x, span<F> z, PRNG& prng,
                               OtReceiver& ot, Socket& chl) {
    MC_BEGIN(task<>, this, x, z, &prng, &ot, &chl, bv = BitVector((u8*)&x, 128),
             otMsg = AlignedUnVector<block>{128});

    setTimePoint("NoisyVoleSender.ot.begin");

    MC_AWAIT(ot.receive(bv, otMsg, prng, chl));
    setTimePoint("NoisyVoleSender.ot.end");

    MC_AWAIT(send(x, z, prng, otMsg, chl));

    MC_END();
  }

  task<> send(F x, span<F> z, PRNG& prng,
                               span<block> otMsg, Socket& chl) {
    MC_BEGIN(task<>, this, x, z, &prng, otMsg, &chl, msg = Matrix<F>{},
             buffer = std::vector<block>{}, xIter = BitIterator{});

    if (otMsg.size() != 128) throw RTE_LOC;
    setTimePoint("NoisyVoleSender.main");

    msg.resize(otMsg.size(), z.size());
    memset(z.data(), 0, sizeof(F) * z.size());

    MC_AWAIT(chl.recv(msg));

    setTimePoint("NoisyVoleSender.recvMsg");
    buffer.resize(z.size());

    xIter = BitIterator((u8*)&x);

    for (u64 i = 0; i < otMsg.size(); ++i, ++xIter) {
      PRNG pi(otMsg[i]);
      pi.get<block>(buffer);
      F *buf = (F *)buffer.data();

      if (*xIter) {
        for (u64 j = 0; j < z.size(); ++j) {
          buf[j] = msg(i, j) - buf[j];
        }
      }

      for (u64 j = 0; j < (u64)z.size(); ++j) {
        z[j] = z[j] + buf[j];
      }
    }
    setTimePoint("NoisyVoleSender.done");

    MC_END();
  }
};
}  // namespace osuCrypto

#endif