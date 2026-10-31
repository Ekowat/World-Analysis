# Changelog

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
