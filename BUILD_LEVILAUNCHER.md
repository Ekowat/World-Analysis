# Build World Analysis 1.5.3 for dot60 / Minecraft 1.26.60

Requirements: Android NDK r28c, CMake 3.22+, Ninja, Python 3.10+, Git.

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/android-ndk-r28c/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-28 \
  -DANDROID_STL=c++_static -DCMAKE_BUILD_TYPE=Release
cmake --build build --target WorldAnalysis --parallel 2
python3 -m unittest discover -s tests -v
```

Outputs: `build/libWorldAnalysis.so` and `build/WorldAnalysis.levipack`.
The package contains the ARM64 library, manifest, original font, and white
wireframe globe PNG icon. The editable SVG is included in the source.
LeviLauncher supplies `libpreloader.so`. CMake pins its API to commit
`5d108da350ce3e7f13f51415f81e65344ecd5d14`. Do not import CMake's separately
built Preloader library as another mod. This mod links its C++ runtime
statically and hides archive symbols; ELF load segments use 16 KiB alignment.
Dependencies are fetched through Git on the first build.

The included GitHub Actions workflow uses these same build settings.
The xmake alternative has matching direct dependency pins but was not run
during this port. Use CMake to reproduce the validated build. Its stale,
different-version lockfile was removed.

To repeat the signature check against your own local copy of the supplied binary:

```sh
python3 scripts/verify_dot60.py /path/to/dot60values.so
```

This checks the exact target SHA-256/build ID, 110 signatures and recorded
layout/ABI instruction fingerprints. It is not a gameplay test.
`verify_dot52.py` remains for baseline comparison and supplies shared scan
helpers to the new verifier.

Install through LeviLauncher Android v1.6.0, which added 26.60 support
([release notes](https://github.com/LiteLDev/LeviLaunchroid/releases/tag/v1.6.0)).
The manifest uses `minecraft_versions: ["1.26.60*"]`; the native build-ID check
still refuses unaudited hotfix binaries. Complete the checks in
[VERIFICATION.md](VERIFICATION.md) before claiming device compatibility.

Neither Minecraft binary is included in the output archives.
