# RoadLife for Android

The Android build uses SDL3, OpenGL ES 3.0, the Android NDK and Gradle.
The desktop OpenGL 4.2 build remains separate and unchanged at the dependency level.

## Requirements

- Android SDK with platform 36 and Build Tools installed
- Android NDK 28.2.13676358
- CMake 3.31.6 (installed through SDK Manager)
- Java 17+
- curl and unzip
- An Android device with OpenGL ES 3.0 (RoadLife currently targets arm64-v8a)

The build script downloads the official SDL3 3.2.16 Android AAR and Gradle 8.9
on first run. CMake downloads GLM 1.0.1 when configuring the native build.

## Build

From the repository root:

```bash
bash scripts/build-android.sh
```

The APK is copied to:

```
build-android/apk/RoadLife-debug.apk
```

## Install and launch

```bash
bash scripts/install-android.sh
```

Or manually:

```bash
adb install -r build-android/apk/RoadLife-debug.apk
adb shell monkey -p com.devayuglass.roadlife 1
```

## Current Android feature scope

- SDL3 Android activity and native lifecycle integration
- OpenGL ES 3.0 renderer path
- Same world, camera, lighting, fog, meshes and scene simulation
- Touch movement on the left side and camera look on the right side
- SDL gamepad support where Android exposes a compatible controller

The desktop Developer Console is not built on Android yet. Desktop-only
wireframe polygon mode is also omitted because it is not part of GLES 3.0.
The Android build uses a landscape fullscreen activity.

## Troubleshooting

- If Gradle reports that Java is too old, use JDK 17 or newer.
- If CMake cannot find SDL3, remove `android/app/.cxx` and rebuild; the
  official SDL3 AAR must exist in `android/app/libs`.
- If the Android SDK or NDK path differs from the default, export
  `ANDROID_HOME` and `ANDROID_NDK_HOME` before running the build script.
- Native logs can be inspected with `adb logcat`.
