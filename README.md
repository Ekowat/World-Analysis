# World Analysis
Note: This entire Project was Vibecoded, this also means that it should be relatively easy for any AI to read through the files and create new types of Mods based off of this.

World Analysis is an open-source native mod for Minecraft Bedrock on Android, built for LeviLauncher.

It adds tools for inspecting the world, nearby mobs, blocks, updates, containers, and other in-game information.

I hope this open source can prove useful in helping others create Levilauncher Mods. :P

(Pssst, Techvfx has a lot of cool effects you can probably use or find useful in small things. :P )

## Features

- Block and mob analysis
- Container / inventory viewer
- World and system notifications
- Block, redstone, crop, fluid, and spawn update viewer
- Technical world effects
- Analysis console and HUD controls
- Mob counter and mob pathfinding
- World scanning
- Ocean depth detection

## Requirements

- Android
- `arm64-v8a`
- LeviLauncher Android **v1.6.0** (the release adding 26.60 support)
- The audited Minecraft Bedrock Android ARM64 **1.26.60** binary; see [VERIFICATION.md](VERIFICATION.md) for its build ID

## Build

Requirements:

- Android NDK r28c
- CMake 3.22+, Ninja, Git
- Python 3

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/android-ndk-r28c/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28 \
  -DANDROID_STL=c++_static -DCMAKE_BUILD_TYPE=Release
cmake --build build --target WorldAnalysis --parallel 2
```

The build produces:

- `libWorldAnalysis.so`
- `WorldAnalysis.levipack`

See [BUILD_LEVILAUNCHER.md](BUILD_LEVILAUNCHER.md) for the build and local audit commands.

## Compatibility

World Analysis **1.5.3** targets the supplied **dot60 / 1.26.60 ARM64** build. It checks Minecraft's GNU build ID before resolving signatures or installing game hooks. A different hotfix requires another audit even if LeviLauncher accepts the version label.

The ARM64 build, 110 signature matches, layout fingerprints, and six Python checks pass. Android gameplay has not been tested for this port. See [VERIFICATION.md](VERIFICATION.md) for the exact scope and remaining device checks.

For future Minecraft updates, follow [UPDATE_METHOD.md](UPDATE_METHOD.md). The source includes old/new binding evidence, disassembly excerpts, and a Python verifier. Minecraft binaries are not included.

## Credits

World Analysis is a modified derivative work based on **[BedrockTools](https://github.com/QYCottage/BedrockTools)** by **[RadiantByte](https://github.com/RadiantByte)**.

BedrockTools is licensed under the **GNU General Public License v3.0 (GPL-3.0)**. Its license and attribution are preserved in this repository. See [`NOTICE.md`](NOTICE.md) and [`LICENSE`](LICENSE).


## License

The World Analysis source is released under **GNU GPL-3.0-only**. See [`LICENSE`](LICENSE).

Third-party components retain their own licenses. See [`NOTICE.md`](NOTICE.md) for the attribution and licensing notes relevant to this repository.
