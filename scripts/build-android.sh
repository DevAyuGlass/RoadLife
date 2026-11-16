#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ANDROID_DIR="$ROOT_DIR/android"
SDK_DIR="${ANDROID_HOME:-${ANDROID_SDK_ROOT:-$HOME/Android/Sdk}}"
SDL_VERSION="3.4.16"
GRADLE_VERSION="8.13"
CACHE_DIR="${XDG_CACHE_HOME:-$HOME/.cache}/roadlife-android"

for tool in curl unzip; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "ERROR: Required tool not found: $tool" >&2
        exit 1
    fi
done

if ! command -v java >/dev/null 2>&1; then
    echo "ERROR: Java is missing. Install OpenJDK 17." >&2
    exit 1
fi

JAVA_MAJOR="$(java -version 2>&1 | sed -n '1s/.*version "\([0-9]*\).*/\1/p')"
if [[ -n "$JAVA_MAJOR" && "$JAVA_MAJOR" -lt 17 ]]; then
    echo "ERROR: Gradle Android Plugin requires Java 17 or newer (found $JAVA_MAJOR)." >&2
    exit 1
fi

if [[ ! -d "$SDK_DIR" ]]; then
    echo "ERROR: Android SDK not found at $SDK_DIR" >&2
    exit 1
fi
export ANDROID_HOME="$SDK_DIR"
export ANDROID_SDK_ROOT="$SDK_DIR"
export ANDROID_NDK_HOME="${ANDROID_NDK_HOME:-$SDK_DIR/ndk/28.2.13676358}"

if [[ ! -f "$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" ]]; then
    echo "ERROR: Android NDK 28.2.13676358 not found at $ANDROID_NDK_HOME" >&2
    exit 1
fi

mkdir -p "$CACHE_DIR" "$ANDROID_DIR/app/libs"

# SDL3's official Android AAR provides SDLActivity and the Prefab CMake package.
SDL_AAR="$ANDROID_DIR/app/libs/SDL3-$SDL_VERSION.aar"
if [[ ! -f "$SDL_AAR" ]]; then
    ARCHIVE="$CACHE_DIR/SDL3-devel-$SDL_VERSION-android.zip"
    URL="https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/SDL3-devel-$SDL_VERSION-android.zip"
    echo "Downloading SDL3 Android package $SDL_VERSION..."
    curl -fL --retry 3 "$URL" -o "$ARCHIVE"
    AAR_ENTRY="$(unzip -Z1 "$ARCHIVE" | grep -E '(^|/)SDL3-'"$SDL_VERSION"'\.aar$' | head -n 1 || true)"
    if [[ -z "$AAR_ENTRY" ]]; then
        echo "ERROR: SDL3 AAR was not found inside $ARCHIVE" >&2
        exit 1
    fi
    unzip -p "$ARCHIVE" "$AAR_ENTRY" > "$SDL_AAR"
fi

GRADLE_HOME="$CACHE_DIR/gradle-$GRADLE_VERSION"
GRADLE_BIN="$GRADLE_HOME/bin/gradle"
if [[ ! -x "$GRADLE_BIN" ]]; then
    DIST="$CACHE_DIR/gradle-$GRADLE_VERSION-bin.zip"
    echo "Downloading Gradle $GRADLE_VERSION..."
    curl -fL --retry 3 "https://services.gradle.org/distributions/gradle-$GRADLE_VERSION-bin.zip" -o "$DIST"
    unzip -q "$DIST" -d "$CACHE_DIR"
fi

echo "Building RoadLife Android APK (arm64-v8a)..."
"$GRADLE_BIN" --no-daemon -p "$ANDROID_DIR" :app:assembleDebug

APK="$ANDROID_DIR/app/build/outputs/apk/debug/app-debug.apk"
if [[ ! -f "$APK" ]]; then
    echo "ERROR: Gradle finished but APK was not found at $APK" >&2
    exit 1
fi

OUT="$ROOT_DIR/build-android/apk"
mkdir -p "$OUT"
cp "$APK" "$OUT/RoadLife-debug.apk"
echo
echo "APK ready: $OUT/RoadLife-debug.apk"
