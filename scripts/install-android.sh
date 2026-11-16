#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
"$ROOT_DIR/scripts/build-android.sh"

APK="$ROOT_DIR/build-android/apk/RoadLife-debug.apk"
PACKAGE="com.devayuglass.roadlife"

adb install -r "$APK"
adb shell monkey -p "$PACKAGE" 1
