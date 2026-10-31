# Updating World Analysis for a new Minecraft Android build

This is the update procedure for this repository. It adapts the supplied
binary-first update method to World Analysis. Other mods' offsets, rendering
owners and gameplay changes are not interchangeable with this project's.

## 1. Preserve a baseline and identify the target

Keep the old source, old game library, new game library, and resulting artifacts
separate. Record SHA-256, GNU build ID, architecture, file size and actual game
version. A file named `dot60values.so` is not enough to identify a binary.
Never redistribute Minecraft libraries in the repository or release package.

The 1.5.3 target is Android AArch64, build ID
`eb291b883dfdfaff70c4999c2eabd61885ae3e15`, SHA-256
`6c7b409d832e182537c0b1dbb8cc3c92f60b7795db7ea5e9f626fb6d6cb8c77f`.
The baseline dot52 build ID is `56de9eed077631e03a31f4f58eb2f0e00071d335`.
Both binding profiles and the historical dot52 report are in `compatibility/`.

## 2. Inventory all native assumptions

Start with these locations, then follow every caller before editing:

| Location | What to audit |
|---|---|
| `src/core/memory/Signatures.cpp` | 110 patterns and the runtime build check |
| `include/worldanalysis/memory/Signatures.hpp` | IDs; names may be inherited incorrectly |
| `include/worldanalysis/sdk/offsets/*.hpp` | Fields, virtual slots, sizes and alignment |
| `src/core/GameHooks.cpp` | Tick, client update, screen lifecycle prototypes |
| `src/core/ui/UiOverlay.cpp` | Text ownership, UI overloads, viewport and flush hooks |
| `src/modules/visual/worldoverlay.cpp` | Renderer, camera, materials and tessellation |
| `src/modules/visual/determine.cpp` | Actor render data, bone matrices, block calibration |
| `src/modules/visual/itemutils.cpp` | ItemStack/Item fields and virtual methods |
| Other module `.cpp` files | Local numeric offsets, ECS hashes/strides, container slots |
| `src/launcher/`, `src/core/Runtime.cpp` | Preloader lifecycle and menu API compatibility |

Search for hexadecimal literals and function-pointer typedefs as well as named
offsets. Some layouts are module-local. Separate live accesses from unused SDK
helpers, and record any retained fields you have not verified.

## 3. Recover function identities before creating patterns

Use an ELF-aware disassembler such as Ghidra, IDA or LLVM/Capstone tooling.
Convert virtual addresses through the correct PT_LOAD segment; do not assume
an RVA is always a file offset. Verify the target is executable code and lies
at the intended function entry, not inside an unrelated function.

1. Scan each old pattern and record zero, one or multiple matches.
2. Compare normalized AArch64 bodies: mask address-dependent ADRP/ADR, branches
   and literal loads only to produce candidates. Field-immediate masking is
   also a candidate-finding tool, not proof.
3. Confirm changed identities with independent native callers/callees, RTTI
   vtables and neighboring anchors, strings, field behavior or whole-body
   equivalence. Follow wrapper calls: short prefixes can identify the wrong
   ECS wrapper even when the normalized match is unique.
4. Inspect the calling convention: x0-x7, floating-point registers, stack
   arguments, and x8 for indirect aggregate returns. Check argument/return
   types, object ownership, destructor obligations and member offsets.
5. Record old/new RVA, method, disassembly evidence, expected field/slot and
   match count. Reject unresolved ambiguities instead of silently choosing
   the first result.

Example: the old ClientInstance local-player prefix first suggested new
`0x9B8659C`, which is a different context wrapper. The correct `0x9B7F610` is
primary slot 31 and calls the independently identified player converter.

## 4. Verify layouts by native operations

Do not apply one global delta to Actor, ClientInstance, renderer or UI fields.
Watch native accessors/constructors and consumers of each value. Shared-pointer
allocations can include control-block storage before the actual object; use
the same `this` origin as the mod's access. Follow inherited and secondary
vtables correctly instead of counting every relocation as a primary method.

The dot52-to-dot60 port changed these live assumptions:

| Binding | Old | dot60 |
|---|---:|---:|
| Actor state-vector / AABB pointer / rotation pointer | `208 / 210 / 218` | `228 / 230 / 238` |
| Actor Dimension / Level / categories | `1C0 / 1D0 / 200` | `1E0 / 1F0 / 220` |
| Level ActorManager / HitResult | `470 / 1D0` | `478 / 1D8` |
| ClientInstance level renderer | `1A0` | `1B8` |
| ClientInstance region / camera / GuiData slots (decimal) | `31 / 205 / 236` | `30 / 203 / 233` |
| LevelRenderer player renderer | `420` | `430` |
| This mod's world-space camera origin | `61C` | `614` |
| ScreenContext tessellator | `B8` | `B0` |
| UI drawText owning-string slot (decimal) | `6` | `6` |
| UI flushText / image / flushImages / fill / texture slots (decimal) | `7 / 8 / 10 / 16 / 32` | `8 / 9 / 11 / 17 / 33` |

Field offsets above are hexadecimal without `0x`. Full changes and instruction
addresses are recorded in `compatibility/dot60-layouts.json`.

Two details differ from values that might be copied from another mod's guide:
Nametag subtracts the current camera vector at `0x614`; `0x620` is a distinct
vector. Dimension's `0x1B8` call reaches WeatherTick; `0x1C0` does not. Follow
the particular consumer instead of substituting a similarly named value.

Important ABI checks in this port:

- ClientInstance update takes `(this, bool, bool)`; forward both booleans.
- UI primary slot 6 takes an owning `std::string`; slot 7 is a new overload.
  Overlay draws copy their text; the detour moves forwarded text into the original.
- Actor health/max-health return integers in w0, not floats in s0.
- ActorManager's list returns a C++ aggregate through x8; keep its return type.
- Do not call or follow `GetDestroyProgress` as a player method. Its inherited
  binding is Warden slot 134. Determine now uses its existing block calibration
  and known-value fallback. `CameraStateApply` is also a misleading unused
  inherited name, identifying an orbital-light destructor.

## 5. Update one coherent target profile

Update the signature table and a new `compatibility/dotNN-bindings.json` together.
Choose patterns that match only the intended executable RVA. The dot60 patterns
use exact bytes, including PC-relative instructions, because those survive
ASLR and the runtime deliberately supports one audited build ID.

Update `src/core/memory/BuildIdentity.hpp` only after the new layout/ABI audit.
Keep its bounds checks and compile-time self-tests. The loaded library's GNU
build ID is read from mapped, readable ELF notes before game signature scanning;
an unaudited binary leaves game hooks disabled. A build ID is a compatibility
identity check, not a security signature or proof the code was never patched.

Change all callers/typedefs affected by ABI or ownership changes, not just the
pattern. Preserve module settings and behavior unless the old code is invalid.
Record unsupported or estimated behavior rather than inventing an offset.

Update the mod version, manifest's `minecraft_versions` field, README, build
instructions, verification report, CI, and source hash evidence. Pin the
Preloader API revision and inspect the launcher's actual native/menu contracts.
Reference sources used here:
[Preloader](https://github.com/LiteLDev/preloader-android),
[LeviLauncher v1.6.0](https://github.com/LiteLDev/LeviLaunchroid/releases/tag/v1.6.0).
The pinned SDK commit is `5d108da350ce3e7f13f51415f81e65344ecd5d14`.

## 6. Validate and publish honestly

```sh
python3 -m unittest discover -s tests -v
python3 scripts/verify_dot60.py /path/to/dot60values.so
```

For the next version, adapt the verifier to that version's new profile/hash;
do not weaken the existing hash check to make an unrelated binary pass. The
verifier checks all 110 patterns and the recorded layout instruction bytes,
not every field or the semantics of every inherited SDK name.

Build with the pinned NDK and settings in `BUILD_LEVILAUNCHER.md`. Inspect the
result for AArch64, `PLGetModRegistration`, expected shared-library imports and
16 KiB load alignment. Reopen the levipack and compare its library/font/icon
bytes to the build inputs; check `entry` and `minecraft_versions`.

Run the Android checklist in `VERIFICATION.md`: menu, world transitions,
rendering/camera/GUI scales, containers, all 11 modules, and rejection of an
unsupported build. Isolate other mods while investigating crashes. Preserve
logs and distinguish static checks, launcher smoke tests and real gameplay.

Publish a clean source ZIP with this guide, build files, scripts, profiles,
evidence, resources, GPL license and original attribution. Exclude game ELF
files, dependency checkouts, build products, caches and machine-specific paths.
Ship the compiled levipack separately if desired. A successful build and unique
patterns are necessary checks, but never a promise of universal update support.
