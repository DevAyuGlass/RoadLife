# Input controls

RoadLife uses a device-independent input layer. Gameplay code should consume movement/look actions instead of reading platform-specific events directly.

## Desktop

- W / A / S / D or arrow keys: move
- Space / Ctrl: vertical movement
- Shift: sprint
- Mouse: look around
- Escape: release/capture the mouse
- F1 or grave accent: open/close the developer console

## Touch devices

The first touch-control profile uses two independent touch zones:

- Left 46% of the screen: virtual movement stick. Drag from anywhere in this zone to move; release to stop.
- Right side: drag to look around.
- Multiple fingers can control movement and camera simultaneously.

The touch zones currently have no visual joystick/button artwork. A configurable on-screen HUD, action buttons, and layout editor are planned as a separate UI feature.

## Gamepads

SDL's gamepad mapping database is used where available:

- Left stick: move
- Right stick: look
- Left-stick click or right trigger: sprint
- Hot-plugging is supported while the application is running

## Platform status

The shared input code is in place for keyboard, touch, and SDL gamepads. This does not yet mean that Android or iOS builds are ready: the current window and renderer request a desktop OpenGL 4.2 context. Mobile targets still need a GLES 3 renderer/context path, platform packaging, lifecycle handling, and touch UI artwork.

## Engine integration

`InputManager` translates device input into normalized movement and look values. New gameplay systems should add named actions to this layer rather than reading SDL keyboard/touch events themselves.
