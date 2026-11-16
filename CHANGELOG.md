# Changelog

## 1.5.4 — Minecraft Android dot62 / 1.26.62

- Audited the supplied 1.26.62 ARM64 binary (build ID
  `c60fa32908019eecae39ebfe7bac5755dc35cde2`) and updated the build-ID gate.
- Kept 82 byte-identical dot60 patterns; recovered the other 28 by normalized
  body comparison (similarity 1.000) with position priors from the kept
  anchors. All 110 patterns are unique in the new executable code.
- Re-verified all 23 layout/ABI instruction proofs, the 21 dot60 constants,
  the ClientInstance/UI/Level/BlockSource/Item/ChestBlockActor vtable slots,
  the two-boolean ClientInstance update, the owning-string UI drawText ABI,
  the x8 aggregate ActorManagerList return and integer health returns.
- No live SDK constant changed; Determine and Analysis Menu keep the corrected
  dot60 behavior. Manifest now targets `minecraft_versions: ["1.26.62*"]`.
- Added the dot62 bindings/evidence/layouts profile and `verify_dot62.py`.

Cross-compilation and static checks pass; Android gameplay is not yet tested.

## 1.5.3 — Minecraft Android dot60 / 1.26.60

- Recovered the 110 native bindings and updated 21 SDK field/virtual-slot constants.
- Forward both ClientInstance update booleans; fix health's integer return ABI.
- Match UI owning-string drawing and the inserted overload's shifted slots.
- Reject unaudited Minecraft build IDs before installing game hooks.
- Remove invalid block-hardness discovery through a misidentified Warden method;
  retain Determine's existing calibration and known-value fallback.
- Pin updated Preloader headers, use private static libc++, and correct the
  launcher's `minecraft_versions` manifest field.
- Include the update procedure, old/new evidence, verifier, tests and device checklist.

Cross-compilation and static checks pass; Android gameplay is not yet tested.
