#define EXPANDER_INSTANTATIONS
#include "Expander.cpp"
#include "libOTe/Vole/Subfield/Subfield.h"

namespace osuCrypto
{
    using u128 = __uint128_t;

    template void ExpanderCode::expand<block, true>(span<const block> e, span<block> w) const;
    template void ExpanderCode::expand<block, false>(span<const block> e, span<block> w) const;
    template void ExpanderCode::expand<u8, true>(span<const u8> e, span<u8> w) const;
    template void ExpanderCode::expand<u8, false>(span<const u8> e, span<u8> w) const;

    template void ExpanderCode::expand<block, u8, true>(
        span<const block> e, span<const u8> e2,
        span<block> w, span<u8> w2) const;
    template void ExpanderCode::expand<block, u8, false>(
        span<const block> e, span<const u8> e2,
        span<block> w, span<u8> w2) const;


    template void ExpanderCode::expand<block, block, true>(
        span<const block> e, span<const block> e2,
        span<block> w, span<block> w2) const;
    //template void ExpanderCode::expand<block, block, false>(
    //    span<const block> e, span<const block> e2,
    //    span<block> w, span<block> w2) const;


    template void ExpanderCode::expand<u128, true>(span<const u128> e, span<u128> w) const;
    template void ExpanderCode::expand<u128, false>(span<const u128> e, span<u128> w) const;

    template void ExpanderCode::expand<u128, u8, true>(
        span<const u128> e, span<const u8> e2,
        span<u128> w, span<u8> w2) const;
    template void ExpanderCode::expand<u128, u8, false>(
        span<const u128> e, span<const u8> e2,
        span<u128> w, span<u8> w2) const;


    template void ExpanderCode::expand<u128, u128, true>(
        span<const u128> e, span<const u128> e2,
        span<u128> w, span<u128> w2) const;

    template void ExpanderCode::expand<u64, true>(span<const u64> e, span<u64> w) const;
    template void ExpanderCode::expand<u64, false>(span<const u64> e, span<u64> w) const;
    template void ExpanderCode::expand<u64, u64, true>(
        span<const u64> e, span<const u64> e2,
        span<u64> w, span<u64> w2) const;

    template void ExpanderCode::expand<u8, u8, true>(
        span<const u8> e, span<const u8> e2,
        span<u8> w, span<u8> w2) const;

    template void ExpanderCode::expand<TypeTraitVec::F, true>(
        span<const TypeTraitVec::F> e, span<TypeTraitVec::F> w) const;
    template void ExpanderCode::expand<TypeTraitVec::F, false>(
        span<const TypeTraitVec::F> e, span<TypeTraitVec::F> w) const;
    template void ExpanderCode::expand<TypeTraitVec::F, TypeTraitVec::G, true>(
        span<const TypeTraitVec::F> e, span<const TypeTraitVec::G> e2,
        span<TypeTraitVec::F> w, span<TypeTraitVec::G> w2) const;

}