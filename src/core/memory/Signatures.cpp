#include <worldanalysis/memory/Signatures.hpp>
#include "BuildIdentity.hpp"

#include <android/log.h>
#include <array>
#include <link.h>
#include <string>
#include <vector>
#include <pl/memory/Signature.hpp>

namespace worldanalysis::memory {
namespace {
std::array<std::uintptr_t, SignatureCount> addresses{};
bool supportedBuild(std::string_view library) {
    struct Search { std::string_view name; bool matches = false; } search{library};
    dl_iterate_phdr([](dl_phdr_info* info, std::size_t, void* opaque) {
        auto& search = *static_cast<Search*>(opaque);
        const std::string_view path(info->dlpi_name ? info->dlpi_name : "");
        if (path.substr(path.find_last_of('/') + 1) != search.name) return 0;
        for (ElfW(Half) i=0; i<info->dlpi_phnum; ++i) {
            const auto& note = info->dlpi_phdr[i];
            if (note.p_type != PT_NOTE || !note.p_memsz) continue;
            for (ElfW(Half) j=0; j<info->dlpi_phnum; ++j) {
                const auto& load = info->dlpi_phdr[j];
                if (load.p_type != PT_LOAD || !(load.p_flags & PF_R) || note.p_vaddr < load.p_vaddr) continue;
                const auto offset = note.p_vaddr-load.p_vaddr;
                if (offset > load.p_memsz || note.p_memsz > load.p_memsz-offset) continue;
                const auto* bytes = reinterpret_cast<const std::uint8_t*>(info->dlpi_addr + note.p_vaddr);
                if (detail::matchesBuildId({bytes, static_cast<std::size_t>(note.p_memsz)})) search.matches = true;
            }
        }
        return 1;
    }, &search);
    return search.matches;
}
// ARM64 dot60 profile. New patterns are exact .text bytes from the supplied ELF.
// PC-relative instructions remain unchanged under ASLR. See compatibility/dot60-bindings.json.
const std::array<SignatureDefinition, SignatureCount> definitions{{
    SignatureDefinition{SignatureId::VersionString, "FF 03 01 D1 FD 7B 02 A9 F4 4F 03 A9 FD 83 00 91 54 D0 3B D5 F3 03 08 AA 88 16 40 F9 A8 83 1F F8 E8 03 00 91 AC FF FF 97 C2 4B F9 B0 42 E4 31 91"},
    SignatureDefinition{SignatureId::Nametag, "FF 03 07 D1 EB 2B 14 6D E9 23 15 6D FD 7B 16 A9 FC 6F 17 A9 FA 67 18 A9 F8 5F 19 A9 F6 57 1A A9"},
    SignatureDefinition{SignatureId::Fullbright, "FD 7B BF A9 FD 03 00 91 08 00 40 F9 41 06 80 52 08 09 40 F9 00 01 3F D6 FD 7B C1 A8 91 07 7C 14"},
    SignatureDefinition{SignatureId::SetupFogPlayer, "FF C3 02 D1 EA 2B 00 FD E9 23 06 6D FD 7B 07 A9 F8 5F 08 A9 F6 57 09 A9 F4 4F 0A A9 FD C3 01 91 57 D0 3B D5 F3 03 00 AA E0 03 01 AA E8 16 40 F9"},
    SignatureDefinition{SignatureId::RaknetUpdate, "FD 7B BC A9 FC 5F 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 03 00 91 FF C3 0C D1 55 D0 3B D5 F3 03 00 AA"},
    SignatureDefinition{SignatureId::NormalTick, "E8 0F 19 FC FD 7B 01 A9 FC 6F 02 A9 FA 67 03 A9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 43 00 91 FF C3 09 D1 54 D0 3B D5 F3 03 00 AA 88 16 40 F9"},
    SignatureDefinition{SignatureId::Time, "FF 43 01 D1 FD 7B 02 A9 F5 1B 00 F9 F4 4F 04 A9 FD 83 00 91 55 D0 3B D5 A8 16 40 F9 A8 83 1F F8 08 00 40 F9 08 49 46 F9 00 01 3F D6 08 00 40 F9"},
    SignatureDefinition{SignatureId::SetTime, "FF 03 01 D1 FD 7B 02 A9 F4 4F 03 A9 FD 83 00 91 54 D0 3B D5 F3 03 01 2A 88 16 40 F9 A8 83 1F F8 08 00 40 F9 08 4D 46 F9 00 01 3F D6 08 00 40 F9"},
    SignatureDefinition{SignatureId::EduMultiplayer, "FF C3 01 D1 FD 7B 04 A9 F5 2B 00 F9 F4 4F 06 A9 FD 03 01 91 55 D0 3B D5 F3 03 00 AA A8 16 40 F9 A8 83 1F F8 08 00 40 F9 09 7D 43 F9 E8 83 00 91"},
    SignatureDefinition{SignatureId::HudCursor, "E9 23 B9 6D FD 7B 01 A9 FC 6F 02 A9 FA 67 03 A9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 43 00 91 FF 43 12 D1 5C D0 3B D5 F4 03 00 AA 00 40 00 91"},
    SignatureDefinition{SignatureId::LevelInit, "FF 03 07 D1 FD 7B 16 A9 FC 6F 17 A9 FA 67 18 A9 F8 5F 19 A9 F6 57 1A A9 F4 4F 1B A9 FD 83 05 91 E5 87 01 A9 5C D0 3B D5 BB AB 01 B0 7B 23 21 91"},
    SignatureDefinition{SignatureId::LevelDtor, "FF 43 02 D1 FD 7B 05 A9 F8 5F 06 A9 F6 57 07 A9 F4 4F 08 A9 FD 43 01 91 56 D0 3B D5 09 98 01 90"},
    SignatureDefinition{SignatureId::ActorManagerList, "FD 7B BA A9 FC 6F 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 03 00 91 18 0C 40 F9 1F 7D 00 A9 1F 09 00 F9 D8 06 00 B4 F4 03 00 AA"},
    SignatureDefinition{SignatureId::DimensionTick, "FF C3 06 D1 EC 93 00 FD EB 2B 13 6D E9 23 14 6D FD 7B 15 A9 FC 6F 16 A9 FA 67 17 A9 F8 5F 18 A9 F6 57 19 A9 F4 4F 1A A9 FD 43 05 91 5C D0 3B D5"},
    SignatureDefinition{SignatureId::WeatherTick, "FD 7B BE A9 F4 4F 01 A9 FD 03 00 91 F3 03 00 AA 00 2C 40 F9 08 80 43 39 88 07 00 34 68 32 40 B9"},
    SignatureDefinition{SignatureId::WeatherGetRainLevel, "08 2C 40 F9 08 81 43 39 C8 00 00 34 01 88 46 2D 42 38 21 1E 40 08 20 1E 20 28 20 1E C0 03 5F D6"},
    SignatureDefinition{SignatureId::WeatherGetLightningLevel, "08 2C 40 F9 08 81 43 39 C8 00 00 34 01 08 48 2D 42 38 21 1E 40 08 20 1E 20 28 20 1E C0 03 5F D6"},
    SignatureDefinition{SignatureId::WeatherIsRaining, "28 00 80 52 09 2C 40 F9 00 01 22 1E 28 81 43 39 68 01 00 34 01 88 46 2D 42 38 21 1E 40 08 20 1E"},
    SignatureDefinition{SignatureId::WeatherIsLightning, "08 2C 40 F9 08 81 43 39 48 02 00 34 00 04 48 2D 28 00 80 52 02 8C 46 2D 04 01 22 1E C8 CC 8C 52"},
    SignatureDefinition{SignatureId::RenderItem, "EF 3B B6 6D ED 33 01 6D EB 2B 02 6D E9 23 03 6D FD 7B 04 A9 FC 6F 05 A9 FA 67 06 A9 F8 5F 07 A9 F6 57 08 A9 F4 4F 09 A9 FD 03 01 91 FF 83 17 D1"},
    SignatureDefinition{SignatureId::GetFov, "EA 0F 1C FC E9 A3 00 6D FD FB 01 A9 F5 17 00 F9 F4 4F 03 A9 FD 63 00 91 08 40 20 1E F4 03 01 2A"},
    SignatureDefinition{SignatureId::GetPerspective, "FD 7B BF A9 FD 03 00 91 00 04 40 F9 08 00 40 F9 08 B5 42 F9 00 01 3F D6 08 00 40 F9 01 E5 41 F9"},
    SignatureDefinition{SignatureId::CameraPlayerVisibilityTick, "FF 03 02 D1 FD 7B 04 A9 F8 5F 05 A9 F6 57 06 A9 F4 4F 07 A9 FD 03 01 91 57 D0 3B D5 F3 03 00 AA E0 63 00 91 E8 16 40 F9 21 A0 00 91 F4 03 03 AA F5 03 02 AA A8 83 1F F8 13 3A 58 94 E0 63 00 91"},
    SignatureDefinition{SignatureId::ClientInstanceUpdate, "E8 0F 19 FC FD 7B 01 A9 FC 6F 02 A9 FA 67 03 A9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 43 00 91 FF 03 13 D1 59 D0 3B D5 DA 1B 98 52 F5 03 02 2A"},
    SignatureDefinition{SignatureId::ClientInstanceGetLocalPlayer, "FF 43 01 D1 FD 7B 03 A9 F3 23 00 F9 FD C3 00 91 53 D0 3B D5 E8 03 00 AA E0 23 00 91 69 16 40 F9 01 01 09 91 A9 83 1F F8 A2 C0 32 95 E0 23 00 91"},
    SignatureDefinition{SignatureId::ContainerScreenControllerDtor, "FF 83 01 D1 FD 7B 01 A9 FA 67 02 A9 F8 5F 03 A9 F6 57 04 A9 F4 4F 05 A9 FD 43 00 91 56 D0 3B D5 F3 03 00 AA C8 16 40 F9 E8 07 00 F9 28 D7 04 D0"},
    SignatureDefinition{SignatureId::ContainerScreenControllerOpen, "FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 F3 03 00 AA 0B C9 2A 94 60 F2 44 F9 E1 03 1F 2A B4 19 1F 94"},
    SignatureDefinition{SignatureId::ChatScreenDtor, "FD 7B BD A9 F5 0B 00 F9 F4 4F 02 A9 FD 03 00 91 68 D4 04 90 08 01 11 91 F3 03 00 AA 09 01 07 91"},
    SignatureDefinition{SignatureId::ChatScreenOpen, "FF 43 03 D1 FD 7B 0A A9 F6 57 0B A9 F4 4F 0C A9 FD 83 02 91 56 D0 3B D5 F3 03 00 AA C8 16 40 F9 A8 83 1F F8 14 6C 44 F9 AA 9F B9 94 F5 03 00 AA"},
    SignatureDefinition{SignatureId::BiomeGetTemperature, "FF C3 01 D1 E9 23 01 6D FD 7B 02 A9 F9 1B 00 F9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 83 00 91 57 D0 3B D5 F6 03 00 AA E0 03 01 AA E8 16 40 F9"},
    SignatureDefinition{SignatureId::GetDestroyProgress, "FF C3 01 D1 FD 7B 02 A9 F9 1B 00 F9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 83 00 91 59 D0 3B D5 F6 03 01 AA F5 03 00 AA 28 17 40 F9 A8 83 1F F8"},
    SignatureDefinition{SignatureId::RenderLevel, "FF 83 07 D1 EE AB 00 FD ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC 67 1A A9 F8 5F 1B A9"},
    SignatureDefinition{SignatureId::TessellatorBegin, "FD 7B BC A9 F8 5F 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 03 00 91 08 60 4A 39 09 54 49 39 08 01 09 2A"},
    SignatureDefinition{SignatureId::TessellatorColor, "E8 6F A8 52 0C 50 49 39 04 01 27 1E 00 08 24 1E 21 08 24 1E 42 08 24 1E 63 08 24 1E 08 00 38 1E"},
    SignatureDefinition{SignatureId::TessellatorVertex, "FF C3 02 D1 EA 1B 00 FD E9 23 04 6D FD 7B 05 A9 FC 6F 06 A9 FA 67 07 A9 F8 5F 08 A9 F6 57 09 A9 F4 4F 0A A9 FD 43 01 91 5B D0 3B D5 68 17 40 F9"},
    SignatureDefinition{SignatureId::MeshHelpersRenderMeshImmediately, "FD 7B BB A9 FC 0B 00 F9 F8 5F 02 A9 F6 57 03 A9 F4 4F 04 A9 FD 03 00 91 FF C3 09 D1 58 D0 3B D5 F7 03 00 AA E0 03 01 AA 08 17 40 F9 F4 03 04 AA"},
    SignatureDefinition{SignatureId::MeshHelpersRenderMeshImmediately2, "FD 7B BC A9 FC 5F 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 03 00 91 FF C3 09 D1 57 D0 3B D5 F6 03 00 AA"},
    SignatureDefinition{SignatureId::RenderMaterialGroupCommon, "A0 1B 05 F0 00 80 1F 91 A1 83 00 D1 A2 43 01 D1 E4 83 01 91 E3 03 14 AA 25 00 80 52 66 D9 3B 94"},
    SignatureDefinition{SignatureId::SurvivalModeAttack, "08 20 43 39 E3 03 02 AA C8 00 00 34 A8 09 02 D0 08 01 69 39 68 00 00 34 E0 03 1F 2A C0 03 5F D6"},
    SignatureDefinition{SignatureId::GameModeAttack, "FF 43 07 D1 FD 7B 18 A9 FC CB 00 F9 F8 5F 1A A9 F6 57 1B A9 F4 4F 1C A9 FD 03 06 91 58 D0 3B D5 F3 03 00 AA F4 03 03 AA 08 17 40 F9 F5 03 02 2A"},
    SignatureDefinition{SignatureId::LevelGetHitResult, "00 EC 40 F9 C0 03 5F D6 00 EC 40 F9 BC AE 16 14 FD 7B BD A9 F6 57 01 A9 F4 4F 02 A9 FD 03 00 91"},
    // I18nImpl singleton accessor recovered from the supplied 26.60 target binary.
    SignatureDefinition{SignatureId::I18nGetInstance, "FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 E8 A1 03 B0 08 41 0D 91 08 FD DF 08 C8 00 00 36 E0 A1 03 90"},
    SignatureDefinition{SignatureId::BlockSourceGetBiome, "FF 03 01 D1 FD 7B 01 A9 F5 13 00 F9 F4 4F 03 A9 FD 43 00 91 55 D0 3B D5 F3 03 00 AA F4 03 01 AA A8 16 40 F9 E8 07 00 F9 08 00 40 F9 08 9D 40 F9"},
    SignatureDefinition{SignatureId::BlockSourceGetBlock, "FF 03 01 D1 FD 7B 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 43 00 91 56 D0 3B D5 C8 16 40 F9 E8 07 00 F9 28 04 40 B9 09 84 C0 79 1F 01 09 6B 6B 03 00 54"},
    SignatureDefinition{SignatureId::BlockSourceGetBrightness, "FD 7B BD A9 F5 0B 00 F9 F4 4F 02 A9 FD 03 00 91 F3 03 00 AA 00 1C 40 F9 F4 03 01 AA 08 00 40 F9"},
    SignatureDefinition{SignatureId::BlockSourceIsSolidBlockingBlock, "09 00 40 F9 23 08 40 B9 28 08 40 29 24 DD 40 F9 E1 03 08 2A 80 00 1F D6 FF 03 01 D1 FD 7B 02 A9"},
    SignatureDefinition{SignatureId::NavigationComponentGetPath, "00 2C 40 F9 C0 03 5F D6 FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 28 00 40 F9 3F 00 00 F9 13 2C 40 F9"},
    // NavigationComponent::setPath(unique_ptr<Path>) in Minecraft 26.45.
    // Unique in dot45values.so; swaps the incoming unique_ptr into mPath at +0x58.
    SignatureDefinition{SignatureId::NavigationComponentSetPath, "FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 28 00 40 F9 3F 00 00 F9 13 2C 40 F9 08 2C 00 F9 F3 00 00 B4"},
    SignatureDefinition{SignatureId::PathGetNodeCount, "09 20 40 A9 08 01 09 CB 00 FD 44 93 C0 03 5F D6 09 20 40 A9 08 01 09 CB 08 FD 44 93 28 00 08 EB"},
    SignatureDefinition{SignatureId::PathGetCurrentIndex, "00 0C 40 F9 C0 03 5F D6 01 0C 00 F9 C0 03 5F D6 EA 0F 1C FC E9 23 01 6D FD 7B 02 A9 F4 4F 03 A9"},
    SignatureDefinition{SignatureId::PathGetPosAtNode, "EA 0F 1C FC E9 23 01 6D FD 7B 02 A9 F4 4F 03 A9 FD 83 00 91 08 24 40 A9 29 01 08 EB 20 04 00 54"},
    SignatureDefinition{SignatureId::PathNavigationAssignPath, "FF 43 01 D1 E8 0B 00 FD FD FB 01 A9 F7 17 00 F9 F6 57 03 A9 F4 4F 04 A9 FD 63 00 91 57 D0 3B D5 E8 16 40 F9 E8 07 00 F9 68 00 40 F9 08 01 00 B4"},
    // Minecraft 26.45 PathNavigation::tick implementations. These are the common owner-aware
    // capture points for the native navigation families used by mobs in this target binary.
    SignatureDefinition{SignatureId::PathNavigationTickBase, "FF 03 04 D1 EA 43 00 FD E9 23 09 6D FD 7B 0A A9 FC 6F 0B A9 FA 67 0C A9 F8 5F 0D A9 F6 57 0E A9 F4 4F 0F A9 FD 83 02 91 57 D0 3B D5 F5 03 00 AA"},
    SignatureDefinition{SignatureId::PathNavigationTickGeneric, "FF 83 03 D1 EE 2B 00 FD ED 33 06 6D EB 2B 07 6D E9 23 08 6D FD 7B 09 A9 FA 67 0A A9 F8 5F 0B A9"},
    SignatureDefinition{SignatureId::PathNavigationTickFlying, "FF 03 02 D1 EA 13 00 FD E9 A3 02 6D FD FB 03 A9 F9 27 00 F9 F8 5F 05 A9 F6 57 06 A9 F4 4F 07 A9 FD E3 00 91 58 D0 3B D5 F5 03 00 AA E0 03 01 AA"},
    SignatureDefinition{SignatureId::PathNavigationTickHover, "FF 43 02 D1 EA 1B 00 FD E9 23 04 6D FD 7B 05 A9 F8 5F 06 A9 F6 57 07 A9 F4 4F 08 A9 FD 43 01 91 58 D0 3B D5 F6 03 00 AA E0 03 01 AA 08 17 40 F9"},
    SignatureDefinition{SignatureId::PathNavigationTickWallClimber, "FF C3 02 D1 EA 1B 00 FD E9 23 04 6D FD 7B 05 A9 FC 6F 06 A9 FA 67 07 A9 F8 5F 08 A9 F6 57 09 A9 F4 4F 0A A9 FD 43 01 91 58 D0 3B D5 99 48 89 52"},
    SignatureDefinition{SignatureId::PathNavigationTickWaterBound, "FF 83 02 D1 EB 2B 04 6D E9 23 05 6D FD 7B 06 A9 F8 5F 07 A9 F6 57 08 A9 F4 4F 09 A9 FD 83 01 91 57 D0 3B D5 F5 03 00 AA E0 03 01 AA E8 16 40 F9"},
    SignatureDefinition{SignatureId::CameraStateApply, "00 80 C0 3C 28 30 04 91 01 E4 00 2F 5F 00 00 72 00 01 80 3D 28 A0 04 91 00 80 C7 3C 00 01 80 3D"},
    SignatureDefinition{SignatureId::LocalPlayerApplyTurnDelta, "FF C3 02 D1 E8 33 00 FD FD 7B 07 A9 F8 5F 08 A9 F6 57 09 A9 F4 4F 0A A9 FD C3 01 91 56 D0 3B D5 F3 03 00 AA F4 03 01 AA C8 16 40 F9 A8 83 1E F8 00 54 46 F9 08 00 40 F9 09 F9 42 F9 E8 83 00 91"},
    SignatureDefinition{SignatureId::BaseOptionRegistryGetHideItemInHand, "FD 7B BF A9 FD 03 00 91 08 00 40 F9 C1 05 80 52 08 09 40 F9 00 01 3F D6 FD 7B C1 A8 83 02 7C 14"},
    SignatureDefinition{SignatureId::HitResultGetEntity, "FF 43 01 D1 FD 7B 03 A9 F3 23 00 F9 FD C3 00 91 53 D0 3B D5 E8 03 00 AA E0 23 00 91 69 16 40 F9 01 E1 00 91 A9 83 1F F8 95 9A 8E 94 E0 23 00 91"},
    SignatureDefinition{SignatureId::ActorSetPos, "FD 7B BE A9 F4 4F 01 A9 FD 03 00 91 F3 03 01 AA F4 03 00 AA 21 36 F9 B0 21 E4 1F 91 E0 03 13 AA D9 CA F5 97 88 16 41 F9 69 02 40 F9 6A 0A 40 B9"},
    SignatureDefinition{SignatureId::ActorSetPosDelta, "08 14 41 F9 29 08 40 B9 2A 00 40 F9 09 21 00 B9 0A 0D 00 F9 C0 03 5F D6 FD 7B BE A9 F4 4F 01 A9"},
    SignatureDefinition{SignatureId::ActorIsPlayer, "09 08 40 F9 EA B1 84 52 AA 2B BF 72 2B A1 43 A9 08 01 0B CB 08 FD 43 D3 08 05 00 51 0C 01 0A 8A 08 18 40 B9 6B 0D 0C 8B 2C 29 40 F9 6B 01 40 F9 7F 05 00 B1 40 03 00 54 8B 15 0B 8B 6D 09 40 B9"},
    SignatureDefinition{SignatureId::ActorIsInvisible, "00 20 00 91 A1 00 80 52 AA 91 85 14 FF 83 01 D1 FD 7B 04 A9 F4 4F 05 A9 FD 03 01 91 54 D0 3B D5"},
    // query.health/query.max_health call these two Actor wrappers in this exact 26.45 binary.
    SignatureDefinition{SignatureId::ActorGetHealth, "00 20 00 91 BC 9E 10 14 FF 83 03 D1 E9 23 07 6D FD 7B 08 A9 FC 6F 09 A9 FA 67 0A A9 F8 5F 0B A9"},
    SignatureDefinition{SignatureId::ActorGetMaxHealth, "00 20 00 91 C0 49 10 14 FF 43 01 D1 FD 7B 03 A9 F4 4F 04 A9 FD C3 00 91 54 D0 3B D5 F3 03 01 2A"},
    // ActorHasTargetTest uses this resolver: it loads Actor+0x278 target ID and resolves it through Level.
    SignatureDefinition{SignatureId::ActorGetTarget, "E8 03 00 AA 00 F8 40 F9 00 01 00 B4 01 49 41 F9 3F 04 00 B1 A0 00 00 54 08 00 40 F9 E2 03 1F 2A"},
    // AttributesComponent attribute lookup by canonical string. This 40-byte form is unique in dot45values.so.
    SignatureDefinition{SignatureId::AttributesGetInstanceByName, "FF C3 01 D1 FD 7B 05 A9 F4 4F 06 A9 FD 43 01 91 54 D0 3B D5 F3 03 00 AA E0 03 01 AA 88 16 40 F9 A8 83 1F F8 ED FE FF 97 08 00 40 39 C8 00 00 37"},
    // Actor equipment accessor: slots 0/1 are hands, slots 2..6 are equippable/armor slots.
    SignatureDefinition{SignatureId::ActorGetEquipmentItem, "3F 04 00 71 A8 01 00 54 FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 00 20 00 91 F3 03 01 2A 85 66 1D 94"},
    SignatureDefinition{SignatureId::ActorFetchNearbyActorsSorted, "FF 83 03 D1 FD 7B 08 A9 FC 6F 09 A9 FA 67 0A A9 F8 5F 0B A9 F6 57 0C A9 F4 4F 0D A9 FD 03 02 91 5B D0 3B D5 F3 03 08 AA F5 03 02 2A 68 17 40 F9"},
    SignatureDefinition{SignatureId::ActorGetNameTag, "FF C3 01 D1 FD 7B 04 A9 F5 2B 00 F9 F4 4F 06 A9 FD 03 01 91 55 D0 3B D5 F3 03 08 AA F4 03 00 AA A8 16 40 F9 A8 83 1F F8 E8 03 00 91 95 EC FF 97"},
    SignatureDefinition{SignatureId::ActorSetNameTag, "FF 83 01 D1 FD 7B 03 A9 F6 57 04 A9 F4 4F 05 A9 FD C3 00 91 56 D0 3B D5 F3 03 00 AA 00 00 05 91 C8 16 40 F9 F4 03 01 AA A8 83 1F F8 43 A4 F2 97"},
    SignatureDefinition{SignatureId::MinecraftUIRenderContextDrawText, "FF 83 01 D1 FD 7B 04 A9 F4 4F 05 A9 FD 03 01 91 54 D0 3B D5 F3 03 00 AA 88 16 40 F9 A8 83 1F F8 08 A4 48 A9 A0 43 1E BC E5 23 00 B9 1F 01 09 EB"},
    SignatureDefinition{SignatureId::ScreenViewRender, "EC 0F 17 FC EB 2B 01 6D E9 23 02 6D FD 7B 03 A9 FC 6F 04 A9 FA 67 05 A9 F8 5F 06 A9 F6 57 07 A9 F4 4F 08 A9 FD C3 00 91 FF C3 08 D1 48 D0 3B D5 F3 03 00 AA E8 07 00 F9 08 15 40 F9 A8 83 1B F8"},
    SignatureDefinition{SignatureId::ContainerScreenControllerOnContainerSlotSelected, "FD 7B BF A9 FD 03 00 91 08 00 40 F9 08 E9 40 F9 00 01 3F D6 E0 03 1F 2A FD 7B C1 A8 C0 03 5F D6"},
    SignatureDefinition{SignatureId::ContainerScreenControllerGetItemStack, "FF C3 01 D1 FD 7B 05 A9 F4 4F 06 A9 FD 43 01 91 54 D0 3B D5 F3 03 00 AA E0 23 00 91 88 16 40 F9 A8 83 1F F8 53 CE 8E 95 68 02 40 F9 09 F1 40 F9"},
    SignatureDefinition{SignatureId::ContainerManagerControllerGetItemInstance, "FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 00 A0 00 91 F3 03 02 2A BE 70 3F 96 E0 00 00 B4 00 14 40 F9"},
    SignatureDefinition{SignatureId::ClientNetworkHandlerHandleSetTitle, "FF C3 04 D1 FD 7B 0E A9 FC 7B 00 F9 F8 5F 10 A9 F6 57 11 A9 F4 4F 12 A9 FD 83 03 91 55 D0 3B D5 F4 03 00 AA F3 03 02 AA A8 16 40 F9 A8 83 1F F8"},
    SignatureDefinition{SignatureId::ClientNetworkHandlerHandleText, "FF 83 05 D1 FD 7B 12 A9 FC 5F 13 A9 F6 57 14 A9 F4 4F 15 A9 FD 83 04 91 57 D0 3B D5 F4 03 00 AA 40 C0 00 91 E8 16 40 F9 F3 03 02 AA A8 83 1F F8"},
    SignatureDefinition{SignatureId::LoopbackPacketSenderSendToServer, "FD 7B BD A9 FC 57 01 A9 F4 4F 02 A9 FD 03 00 91 FF C3 08 D1 54 D0 3B D5 88 16 40 F9 A8 83 1F F8"},
    SignatureDefinition{SignatureId::ClientInstanceGetPacketSender, "00 E4 40 F9 C0 03 5F D6 00 D0 40 F9 08 00 40 F9 01 F1 40 F9 20 00 1F D6 00 D0 40 F9 08 00 40 F9"},
    SignatureDefinition{SignatureId::MinecraftPacketsCreatePacket, "1F A4 05 71 08 01 00 54 E9 03 00 2A 6A 20 FB D0 4A 81 15 91 8B 00 00 10 4C 79 69 78 6B 09 0C 8B"},
    SignatureDefinition{SignatureId::LocalPlayerChangeDimension, "FF 03 04 D1 FD 7B 0A A9 FC 6F 0B A9 FA 67 0C A9 F8 5F 0D A9 F6 57 0E A9 F4 4F 0F A9 FD 83 02 91 5A D0 3B D5 F6 03 01 AA F3 03 00 AA 48 17 40 F9"},
    SignatureDefinition{SignatureId::NbtTreeFind, "FD 7B BC A9 F8 5F 01 A9 F6 57 02 A9 F4 4F 03 A9 FD 03 00 91 F3 03 00 AA 77 8E 40 F8 77 08 00 B4 34 58 40 A9 F5 03 13 AA 04 00 00 14 F5 03 17 AA F7 6A 69 F8 37 05 00 B4 E8 82 40 39 28 02 00 36 E1 1A 40 F9 28 02 00 36 F8 16 40 F9 1F 03 16 EB E0 03 14 AA 02 33 96 9A 2A 04 E9 94 20 02 00 34"},
    SignatureDefinition{SignatureId::ItemStackBaseLoadItem, "FF 83 03 D1 FD 7B 08 A9 FC 6F 09 A9 FA 67 0A A9 F8 5F 0B A9 F6 57 0C A9 F4 4F 0D A9 FD 03 02 91 5B D0 3B D5 F4 03 01 AA F3 03 00 AA 68 17 40 F9 A8 83 1F F8 E8 C3 00 91 D1 9E F0 97 61 EB F8 F0"},
    // ItemStackBase current damage getter. The previous pattern pointed at an unrelated
    // component routine and was unsafe when called as int(ItemStackBase const*). This exact
    // 26.45 helper follows the stack item/shared counter and Damage component, returning 0
    // for non-damageable/empty stacks. Unique in the supplied dot45values.so.
    SignatureDefinition{SignatureId::ItemStackBaseGetDamageValue, "08 04 40 F9 28 01 00 B4 08 01 40 F9 E8 00 00 B4 00 08 40 F9 A0 00 00 B4 A1 65 F9 F0 21 04 11 91"},
    // ItemStackBase::getItem() const. Unique in dot45values.so; follows mItem WeakPtr and returns Item* or nullptr.
    SignatureDefinition{SignatureId::ItemStackBaseGetItem, "08 04 40 F9 68 00 00 B4 00 01 40 F9 C0 03 5F D6 E0 03 1F AA C0 03 5F D6 09 0C 42 78 E8 FF 8F 52"},
    // Legacy ItemStackBase id/aux-style accessor recovered from the 26.45 binary. Do not use this as item identity.
    SignatureDefinition{SignatureId::ItemStackBaseGetId, "09 0C 42 78 E8 FF 8F 52 0A 80 5F F8 3F 01 08 6B 44 19 40 FA 48 A1 04 91 08 00 88 9A 00 01 40 79"},
    SignatureDefinition{SignatureId::ItemActorClientInitialize, "FF C3 01 D1 FD 7B 02 A9 FA 67 03 A9 F8 5F 04 A9 F6 57 05 A9 F4 4F 06 A9 FD 83 00 91 5A D0 3B D5 F8 03 00 AA E0 03 01 AA 48 17 40 F9 F5 03 06 2A"},
    SignatureDefinition{SignatureId::ItemActorPostNormalTick, "FF 83 03 D1 E8 3B 00 FD FD 7B 08 A9 FC 6F 09 A9 FA 67 0A A9 F8 5F 0B A9 F6 57 0C A9 F4 4F 0D A9 FD 03 02 91 5A D0 3B D5 F3 03 00 AA 48 17 40 F9"},
    SignatureDefinition{SignatureId::ItemActorTryGetFromEntity, "FD 7B BE A9 F3 0B 00 F9 FD 03 00 91 09 04 40 F9 0A 00 87 52 2A B7 B0 72 2B A1 43 A9 08 01 0B CB 08 FD 43 D3 08 05 00 51 0C 01 0A 8A 08 10 40 B9 6B 0D 0C 8B 2C 29 40 F9 6B 01 40 F9 7F 05 00 B1 00 05 00 54 8B 15 0B 8B 6D 09 40 B9 BF 01 0A 6B 41 FF FF 54 29 2D 40 F9 7F 01 09 EB 20 04 00 54"},
    SignatureDefinition{SignatureId::BaseActorRenderContextCtor, "FF 43 01 D1 FD 7B 02 A9 F6 57 03 A9 F4 4F 04 A9 FD 83 00 91 56 D0 3B D5 F3 03 00 AA F4 03 02 AA C8 16 40 F9 F5 03 01 AA A8 83 1F F8 68 5E 04 B0"},
    SignatureDefinition{SignatureId::ItemRendererRenderGuiItemNew, "FF C3 05 D1 EC 73 00 FD EB 2B 0F 6D E9 23 10 6D FD 7B 11 A9 FC 6F 12 A9 FA 67 13 A9 F8 5F 14 A9 F6 57 15 A9 F4 4F 16 A9 FD 43 04 91 5B D0 3B D5"},
    // MinecraftUIRenderContext's complete item helper. Unlike a raw
    // ItemRenderer::renderGuiItemNew call, this 26.45 wrapper constructs and
    // destroys BaseActorRenderContext, submits the item pass, and performs the
    // enchanted/overlay follow-up required for the icon to become visible.
    SignatureDefinition{SignatureId::MinecraftUIRenderContextRenderItem, "FF 03 05 D1 EA 6B 00 FD E9 23 0E 6D FD 7B 0F A9 FC 67 10 A9 F8 5F 11 A9 F6 57 12 A9 F4 4F 13 A9"},
    SignatureDefinition{SignatureId::ControlOptionEditorTick, "FF 83 05 D1 EF 3B 0C 6D ED 33 0D 6D EB 2B 0E 6D E9 23 0F 6D FD 7B 10 A9 FC 6F 11 A9 FA 67 12 A9 F8 5F 13 A9 F6 57 14 A9 F4 4F 15 A9 FD 03 04 91 5B D0 3B D5 F4 03 00 AA F8 03 02 AA 68 17 40 F9"},
    SignatureDefinition{SignatureId::ControlOptionEditorRender, "FF C3 03 D1 EE 3B 00 FD ED B3 07 6D EB AB 08 6D E9 A3 09 6D FD FB 0A A9 F9 5F 00 F9 F8 5F 0C A9 F6 57 0D A9 F4 4F 0E A9 FD A3 02 91 55 D0 3B D5"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceDown, "FF 83 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC 67 1A A9 F8 5F 1B A9 F6 57 1C A9 F4 4F 1D A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 CA 5A 7E 95"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceUp, "FF 83 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC 67 1A A9 F8 5F 1B A9 F6 57 1C A9 F4 4F 1D A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 94 58 7E 95"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceNorth, "FF C3 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC D3 00 F9 FA 67 1B A9 F8 5F 1C A9 F6 57 1D A9 F4 4F 1E A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 5D 56 7E 95 04 00 00 14 80 0A 43 F9 81 C2 17 91 61 56 7E 95 08 08 40 F9 00 00 C0 3D 88 02 03 F9 28 00 80 52 80 7E 81 3D 88 22 18 39 E0 03 13 AA 41 00 80 52 B0 2F 07 94 02 87 40 2D 85 FE 45 BD"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceSouth, "FF C3 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC D3 00 F9 FA 67 1B A9 F8 5F 1C A9 F6 57 1D A9 F4 4F 1E A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 13 54 7E 95 04 00 00 14 80 0A 43 F9 81 C2 17 91 17 54 7E 95 08 08 40 F9 00 00 C0 3D 88 02 03 F9 28 00 80 52 80 7E 81 3D 88 22 18 39 E0 03 13 AA 61 00 80 52 66 2D 07 94 02 87 40 2D 85 FE 45 BD"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceWest, "FF C3 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC D3 00 F9 FA 67 1B A9 F8 5F 1C A9 F6 57 1D A9 F4 4F 1E A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 CD 51 7E 95 04 00 00 14 80 0A 43 F9 81 C2 17 91 D1 51 7E 95 08 08 40 F9 00 00 C0 3D 88 02 03 F9 28 00 80 52 80 7E 81 3D 88 22 18 39 E0 03 13 AA 81 00 80 52 20 2B 07 94 02 87 40 2D 85 06 46 BD"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellateFaceEast, "FF C3 07 D1 EF 3B 15 6D ED 33 16 6D EB 2B 17 6D E9 23 18 6D FD 7B 19 A9 FC D3 00 F9 FA 67 1B A9 F8 5F 1C A9 F6 57 1D A9 F4 4F 1E A9 FD 43 06 91 57 D0 3B D5 0A 60 00 91 F6 03 03 AA E8 16 40 F9 F5 03 02 AA F4 03 00 AA F3 03 01 AA A8 83 1B F8 08 C4 41 39 09 20 58 39 1F 01 00 71 98 00 8A 9A 69 02 00 37 88 CE 41 39 08 01 00 34 81 06 40 F9 C1 00 00 B4 80 0A 43 F9 82 62 18 91 83 C2 17 91 8C 4F 7E 95 04 00 00 14 80 0A 43 F9 81 C2 17 91 90 4F 7E 95 08 08 40 F9 00 00 C0 3D 88 02 03 F9 28 00 80 52 80 7E 81 3D 88 22 18 39 E0 03 13 AA A1 00 80 52 DF 28 07 94 02 87 40 2D 85 06 46 BD"},
    SignatureDefinition{SignatureId::BlockTessellatorTessellatePane, "EB 2B B8 6D E9 23 01 6D FD 7B 02 A9 FC 6F 03 A9 FA 67 04 A9 F8 5F 05 A9 F6 57 06 A9 F4 4F 07 A9 FD 83 00 91 FF 03 11 D1 5A D0 3B D5 F6 03 00 AA"},
    SignatureDefinition{SignatureId::BlockSourceGetBlockForTessellation, "5F 04 00 71 A0 00 00 54 E2 00 00 35 08 00 40 F9 02 09 40 F9 40 00 1F D6 08 00 40 F9 02 15 40 F9"},
    SignatureDefinition{SignatureId::TextureUVCoordinateSetCopyCtor, "FD 7B BE A9 F4 4F 01 A9 FD 03 00 91 20 C0 C0 3C 21 00 C0 3D F4 03 01 AA F3 03 00 AA 00 C0 80 3C"},
    SignatureDefinition{SignatureId::TextureUVCoordinateSetDtor, "08 A0 40 39 48 00 00 37 C0 03 5F D6 00 1C 40 F9 33 FC F3 14 88 AC 04 90 0A 1D 41 F9 0A 05 00 B4"},
    SignatureDefinition{SignatureId::RenderChunkCoordinatorSetAllDirty, "FF 03 02 D1 FD 7B 02 A9 FC 6F 03 A9 FA 67 04 A9 F8 5F 05 A9 F6 57 06 A9 F4 4F 07 A9 FD 83 00 91 48 D0 3B D5 F4 03 02 2A F3 03 00 AA E8 03 00 F9"},
    // DataDrivenModel::render. Hooked after the native render so Determine can
    // consume the exact current-frame BoneOrientation matrices generated by the
    // supplied 26.60 client instead of guessing entity body-part boxes.
    SignatureDefinition{SignatureId::DataDrivenModelRender, "EF 3B B6 6D ED 33 01 6D EB 2B 02 6D E9 23 03 6D FD 7B 04 A9 FC 6F 05 A9 FA 67 06 A9 F8 5F 07 A9 F6 57 08 A9 F4 4F 09 A9 FD 03 01 91 FF C3 06 D1 E3 93 03 A9 57 D0 3B D5 F8 03 00 AA E8 16 40 F9"},
}};
}

bool resolveAll(std::string_view libraryName) {
    addresses.fill(0);
    if (!supportedBuild(libraryName)) {
        __android_log_write(ANDROID_LOG_ERROR, "WorldAnalysis", "Unsupported Minecraft build: expected audited dot60 / 1.26.60; game hooks disabled.");
        return false;
    }
    const std::string library(libraryName);
    std::vector<std::string> patterns;
    patterns.reserve(definitions.size());
    for (const auto& definition : definitions) patterns.emplace_back(definition.pattern);
    const auto resolved = pl::memory::resolveSignatures(patterns, library.c_str());
    addresses.fill(0);
    bool any = false;
    for (std::size_t i = 0; i < definitions.size(); ++i) {
        const auto it = resolved.find(patterns[i]);
        if (it == resolved.end() || it->second == 0) continue;
        addresses[static_cast<std::size_t>(definitions[i].id)] = it->second;
        any = true;
    }
    return any;
}

std::uintptr_t resolve(SignatureId id) {
    const auto index = static_cast<std::size_t>(id);
    return index < addresses.size() ? addresses[index] : 0;
}

void clear() {
    addresses.fill(0);
}

}
