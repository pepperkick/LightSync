# LightSync

A BeatSaber Quest mod that syncs BeatSaber's stage lights to PhilipsHUE.

## Installing

Upload the `lightsync.zip` from release to BeatOn

## Running

Before you start BeatSaber along with the mod, press the link button in your HUE bridge to connect.
All connected lights should animate red to blue when connection is established.

## Building

`ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk`
