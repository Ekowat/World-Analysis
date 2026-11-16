# World Analysis 1.5.4 — dot62 / Minecraft 1.26.62

## Exact target

- Android ARM64 `dot62values.so` supplied for this update (`BR_DEBUG 1.26.62.01_RC0`).
- GNU build ID: `c60fa32908019eecae39ebfe7bac5755dc35cde2`.
- SHA-256: `4de40e1dcbbca6d4773af57fa6753afd991b95878bc5d51dcceaebf0d65283d8`.
- The old dot52 and dot60 profiles are retained for comparison, not enabled at runtime.

This is a cross-compiled and statically audited port. Minecraft gameplay and
the LeviLauncher menu were not run on a physical Android device for this port.
Do not describe it as device-tested or universally compatible.

## Checks completed

- Android NDK r28c, API 28, `arm64-v8a`, Release, C++20 CMake build passes.
- All 110 source patterns match exactly once at their recorded executable RVAs.
- 23 recorded layout/ABI instruction fingerprints match the target ELF.
- Six Python tests pass: ELF note alignment, truncation, oversized lengths,
  program-header bounds, pattern boundaries/overlaps, and launcher manifest key.
- The native build-ID note parser has compile-time checks for the correct ID,
  wrong ID, empty/truncated input, and invalid note lengths.
- The library exports `PLGetModRegistration`, targets AArch64, and has 16 KiB
  ELF load alignment. It uses private static libc++ and imports Preloader plus
  Android platform libraries; it does not require a mod-bundled libc++ shared library.
- The levipack manifest, library, original font and icon are checked during packaging.

Repeat locally with:

```sh
python3 -m unittest discover -s tests -v
python3 scripts/verify_dot62.py /path/to/dot62values.so
```

## Changes and evidence

dot60 to dot62 is a hotfix-level delta. 82 of the 110 dot60 patterns are
byte-identical in dot62 and were kept as-is. The 28 remaining functions were
recovered by fuzzy word matching with a position prior derived from the 82
anchors (RVAs moved by a smooth +0xc0..+0x6ec), then verified by comparing each
candidate's normalized disassembly with the recorded dot60 excerpt: all 28
match at similarity 1.000, differing only in branch targets and address/field
immediates. Every one of the 28 new patterns is unique in the dot62 executable
sections; no pattern required a wildcard extension.

- `compatibility/dot62-bindings.json`: dot60/dot62 RVAs, method, pattern, match count.
- `compatibility/dot62-evidence.json`: dot60/dot62 instruction excerpts.
- `compatibility/dot62-layouts.json`: zero changed constants and 23 re-verified instruction proofs.
- `compatibility/module-source-hashes.json`: uploaded/current hashes for all 29 module files.

No live SDK constant changed from dot60. The 21 dot60 constants and all 23
instruction proofs were re-verified at their dot62 locations: Actor
state-vector/rotation/level/dimension/categories/target fields, Level
ActorManager/HitResult, ClientInstance level renderer and camera/GuiData/
region/local-player accessors, LevelRendererPlayer player renderer and camera
origin `0x614`, ScreenContext tessellator `0xB0`, Dimension weather `0x1B8`,
weather fields, ItemStack/Item/ChestBlockActor/DataDrivenModel layouts.
Vtable slots were re-verified through the binary's relative relocations and
surviving RTTI: ClientInstance getRegion 30 / getLocalPlayer 31 /
getMinecraftGame 82 / getCamera 203 / getGuiData 233, MinecraftUIRenderContext
getLineLength 2 / drawText 6 (owning string) with the slot 7 overload /
flushText 8 / drawImage 9 / flushImages 11 / fillRectangle 17 / getTexture 33,
Level getTime 133, BlockSource getDimensionId 19, Item max-damage 37 (field
`0x110`), ChestBlockActor container 45 (subobject `+0x100`).

ABI checks re-verified in the dot62 binary: ClientInstance update consumes both
`w1` and `w2` (two booleans); the UI slot 6 draw hollows the incoming
`std::string` (owning), so overlay draws keep copying and the detour keeps
moving the forwarded text; ActorManagerList still returns its aggregate through
`x8`; Actor health/max-health still return integers in `w0`.

Retained from dot60 without an independent dot62 proof, matching the previous
audit scope: the LevelRendererPlayer fog float group, the fallback-only
`mSelectionOverlayMaterial 0x1028`, GuiData viewport pairs (runtime-validated
with plausibility checks), Player name/skin, Actor hurt-time/name-tag fields,
HitResult fields, `Dimension::mBlockSource`, `Biome::mHash`, NBT and Shulker
preview offsets. Every retained SDK field was not independently proved, and
signature uniqueness does not establish ABI or object lifetime correctness.

All 11 modules and their menu registration remain. The Determine and Analysis
Menu corrections from the dot60 port (no hardness-discovery through the
misidentified Warden method; health read as an integer) carry over unchanged.

## Android checks still required

Use LeviLauncher Android and the exact audited game binary. Test each module
separately, then together, in a disposable world:

1. Enable the mod, confirm all 11 menu entries and saved settings, then enter,
   leave, and reopen a world. Check Android logs for missing hooks/crashes.
2. Determine/Analysis Menu: blocks, mobs, health, bones, text and camera movement;
   compare first/third person and touch/GUI scales.
3. Instore Viewer: empty/full chests, double chests, item icons/names/counts,
   rapid opening/closing, and remote versus local inventories.
4. World Scan/Mob Counter/Pathfinding: spawning, despawning, navigation, chunks,
   and Overworld/Nether/End transitions. Check stability and frame time.
5. System Notifications/Update Viewer/Tech VFX/Analyze/Ocean Depth: weather/time,
   damage/recovery, updates, animations, console input, water and world changes.
6. Launch a different game build and verify the unsupported-build log appears
   and game hooks stay disabled. Test other mods only after isolated checks pass.

The xmake alternative and GitHub-hosted CI were not executed here. The included
workflow runs the tested CMake build and the Python tests without a game binary.
Future hotfixes need a fresh audit; the version-label wildcard is not approval.
