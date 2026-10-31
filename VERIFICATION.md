# World Analysis 1.5.3 — dot60 / Minecraft 1.26.60

## Exact target

- Android ARM64 `dot60values.so` supplied for this update.
- GNU build ID: `eb291b883dfdfaff70c4999c2eabd61885ae3e15`.
- SHA-256: `6c7b409d832e182537c0b1dbb8cc3c92f60b7795db7ea5e9f626fb6d6cb8c77f`.
- The old dot52 profile is retained for comparison, not enabled at runtime.

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
python3 scripts/verify_dot60.py /path/to/dot60values.so
```

## Changes and evidence

Only 51 of the old 110 patterns remained unique in dot60. New bindings were
recovered with normalized code comparisons, independent interior anchors,
native caller/callee relationships and RTTI/vtable context. New exact patterns
are intentional: ASLR preserves PC-relative instructions, and the runtime first
requires the audited GNU build ID.

- `compatibility/dot60-bindings.json`: old/new RVAs, method, pattern, match count.
- `compatibility/dot60-evidence.json`: old/new instruction excerpts and function sizes.
- `compatibility/dot60-layouts.json`: 21 changed constants and 23 instruction proofs.
- `compatibility/module-source-hashes.json`: uploaded/current hashes for all 29 module files.

Important corrections include Actor component/level/dimension/category fields,
Level ActorManager/HitResult fields, renderer pointers, ScreenContext tessellator,
ClientInstance methods, and the inserted UI drawText overload. Client update
now forwards both boolean arguments. Analysis Menu reads health as an integer.
The owning-string UI drawText ABI copies overlay text and moves forwarded text.

The camera origin is `0x614` for this mod's Nametag/world-space subtraction.
The separate vector at `0x620` must not be substituted solely because another
mod uses it. Dimension's weather pointer remains `0x1B8`: its call reaches the
verified WeatherTick. The following `0x1C0` member belongs to a different path.

The baseline `GetDestroyProgress` ID actually identified Warden vtable slot 134.
Determine no longer follows that unrelated call graph to guess block hardness;
its existing calibrated/known-value fallback is retained. Custom/unrecognized
blocks may still display estimated hardness until calibration succeeds.
`CameraStateApply` is another inherited misleading ID (an orbital-light
destructor), and is unused. The unused NbtTreeFind binding selects a generic
tree-lookup implementation whose entire normalized body and comparator calls
match; equivalent compiler clones exist. These names are not semantic proof.

All 11 modules and their menu registration remain. Of the 29 module files,
27 are unchanged; Analysis Menu has the health ABI correction and Determine
has the invalid hardness-discovery path removed. License, author attribution,
font, icon, settings, and the remaining module implementations are preserved.

Selected retained item, attribute/ECS, block, weather, matrix and bone paths
were compared. Every retained SDK field was not independently proved. In
particular, unused legacy SDK fields are not certified for new consumers.
Signature uniqueness does not establish ABI or object lifetime correctness.

## Android checks still required

Use LeviLauncher Android v1.6.0 and the exact audited game binary. Test each
module separately, then together, in a disposable world:

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
