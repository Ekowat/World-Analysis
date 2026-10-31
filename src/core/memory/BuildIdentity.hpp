#pragma once

#include <array>
#include <cstdint>
#include <span>

namespace worldanalysis::memory::detail {
inline constexpr std::array<std::uint8_t, 20> MinecraftBuildId{
    0xeb,0x29,0x1b,0x88,0x3d,0xfd,0xfa,0xff,0x70,0xc4,
    0x99,0x9c,0x2e,0xab,0xd6,0x18,0x85,0xae,0x3e,0x15
};

// ELF notes use 4-byte alignment, including on ELF64. Bound every field first.
constexpr bool matchesBuildId(std::span<const std::uint8_t> notes) {
    const auto word = [&](std::size_t at) {
        return std::uint32_t(notes[at]) | (std::uint32_t(notes[at+1]) << 8)
            | (std::uint32_t(notes[at+2]) << 16) | (std::uint32_t(notes[at+3]) << 24);
    };
    while (notes.size() >= 12) {
        const std::uint64_t nameSize = word(0), valueSize = word(4);
        const auto type = word(8);
        const std::uint64_t paddedName = (nameSize + 3) & ~std::uint64_t(3);
        const std::uint64_t paddedValue = (valueSize + 3) & ~std::uint64_t(3);
        if (paddedName > notes.size()-12 || paddedValue > notes.size()-12-paddedName) return false;
        if (type == 3 && nameSize == 4 && notes[12]=='G' && notes[13]=='N'
            && notes[14]=='U' && notes[15]==0) {
            if (valueSize != MinecraftBuildId.size()) return false;
            for (std::size_t i=0; i<MinecraftBuildId.size(); ++i)
                if (notes[12+paddedName+i] != MinecraftBuildId[i]) return false;
            return true;
        }
        notes = notes.subspan(static_cast<std::size_t>(12+paddedName+paddedValue));
    }
    return false;
}

constexpr auto exampleBuildNote() {
    std::array<std::uint8_t,36> note{4,0,0,0,20,0,0,0,3,0,0,0,'G','N','U',0};
    for (std::size_t i=0; i<MinecraftBuildId.size(); ++i) note[16+i]=MinecraftBuildId[i];
    return note;
}
static_assert(matchesBuildId(exampleBuildNote()));
static_assert(!matchesBuildId(std::span<const std::uint8_t>{}));
static_assert(!matchesBuildId(std::span<const std::uint8_t>(exampleBuildNote()).first(35)));
static_assert([] { auto n=exampleBuildNote(); n[16]^=1; return !matchesBuildId(n); }());
static_assert([] { auto n=exampleBuildNote(); n[0]=255; n[3]=255; return !matchesBuildId(n); }());
} // namespace worldanalysis::memory::detail
