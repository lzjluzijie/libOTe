
#define EXCONVCODE_INSTANTIATIONS
#include "ExConvCode.cpp"

using u128 = __uint128_t;

namespace osuCrypto
{

    template void ExConvCode::dualEncode<block>(span<block> e);
    template void ExConvCode::dualEncode<u8>(span<u8> e);
    template void ExConvCode::dualEncode<block>(span<block> e, span<block> w);
    template void ExConvCode::dualEncode<u8>(span<u8> e, span<u8> w);
    template void ExConvCode::dualEncode2<block, u8>(span<block>, span<u8> e);
    template void ExConvCode::dualEncode2<block, block>(span<block>, span<block> e);

    template void ExConvCode::accumulate<block, u8>(span<block>, span<u8> e);
    template void ExConvCode::accumulate<block, block>(span<block>, span<block> e);

    template void ExConvCode::dualEncode<u128>(span<u128> e);
    template void ExConvCode::dualEncode<u128>(span<u128> e, span<u128> w);
    template void ExConvCode::dualEncode2<u128, u8>(span<u128>, span<u8> e);
    template void ExConvCode::dualEncode2<u128, u128>(span<u128>, span<u128> e);

    template void ExConvCode::accumulate<u128, u8>(span<u128>, span<u8> e);
    template void ExConvCode::accumulate<u128, u128>(span<u128>, span<u128> e);
}
