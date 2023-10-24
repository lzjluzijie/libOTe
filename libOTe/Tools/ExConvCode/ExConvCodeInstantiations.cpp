
#define EXCONVCODE_INSTANTIATIONS
#include "ExConvCode.cpp"
#include "libOTe/Vole/Subfield/Subfield.h"

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
    template void ExConvCode::dualEncode2<u8, u8>(span<u8>, span<u8> e);
    template void ExConvCode::dualEncode2<u128, u8>(span<u128>, span<u8> e);
    template void ExConvCode::dualEncode2<u128, u128>(span<u128>, span<u128> e);

    template void ExConvCode::accumulate<u128, u8>(span<u128>, span<u8> e);
    template void ExConvCode::accumulate<u128, u128>(span<u128>, span<u128> e);

    template void ExConvCode::dualEncode<u64>(span<u64> e);
    template void ExConvCode::dualEncode<u64>(span<u64> e, span<u64> w);
    template void ExConvCode::dualEncode2<u64, u64>(span<u64>, span<u64> e);
    template void ExConvCode::accumulate<u64, u64>(span<u64>, span<u64> e);

    template void ExConvCode::dualEncode2<TypeTraitVec::F, TypeTraitVec::G>(span<TypeTraitVec::F>, span<TypeTraitVec::G> e);
    template void ExConvCode::dualEncode<TypeTraitVec::F>(span<TypeTraitVec::F> e);
    template void ExConvCode::accumulate<TypeTraitVec::F, TypeTraitVec::G>(span<TypeTraitVec::F>, span<TypeTraitVec::G> e);
}
