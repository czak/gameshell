# gameshell

A _work-in-progress_ 10-foot[^1] app launcher for Wayland, controlled with gamepad.



## Demo

See current features of the launcher in a [sway](https://swaywm.org/) session:

* toggling overlay on/off
* launching (multiple) processes
* sending `STOP`/`CONT` to child processes
* terminating processes

https://github.com/user-attachments/assets/0ee967e1-7af4-4f69-9ad1-bff91ccdd8cf

## Features

* Overlay UI controlled with a gamepad
* Read commands from a config file
* Launch (multiple) processes
* Terminate child processes with `SIGTERM`
* Pause and resume via `SIGSTOP`/`SIGCONT`
* Detect evdev gamepad connect/disconnect

## Requirements, dependencies

To build, `gameshell` requires:

* `wayland-client`
* `wayland-egl`
* `egl`
* `glesv2`

To run, you need a Wayland compositor supporting the [wlr layer shell](https://wayland.app/protocols/wlr-layer-shell-unstable-v1) protocol.

## How to build

```sh
meson setup build
meson compile -C build
./build/gameshell
```

## How to use

Create the configuration file `$HOME/.config/gameshell/commands`.
Each line in the file is a separate command in the format:

```
label here;/working/directory;command;and;its;arguments
```

Here is the complete configuration file from the demo recording:

```
Baba is You;/home/czak/games/pc/baba;./run.sh
Celeste;/home/czak/games/pc/celeste;./Celeste
DOOM 3;/home/czak/games/pc/doom3bfg;./run.sh
Super Hexagon;/home/czak/games/pc/hexagon;wine;superhexagon.exe
vkcube;/;vkcube
vkgears;/;vkgears
----;/;true
fullscreen;/;swaymsg;fullscreen;toggle
suspend;/;systemctl;suspend
```

Controls:
* <kbd>MODE</kbd> ("Xbox" or "Playstation" button) - toggle UI on/off
* <kbd>SELECT</kbd> ("SHARE" on Dualshock) - quit gameshell
* <kbd>DPAD_UP</kbd> / <kbd>DPAD_DOWN</kbd> - navigate menus up/down
* <kbd>SOUTH</kbd> ("B" on Xbox, "Cross" on Dualshock) - select menu item
* <kbd>EAST</kbd> ("X" on Xbox, "Square" on Dualshock) - return to top menu

## How it works

`gameshell` is intended to be run as a long-running process in a Wayland session. By default it only listens to <kbd>MODE</kbd> button press to activate the UI. When inactive, `gameshell` stays in the background and does nothing.

UI is rendered with OpenGL ES 2 in an `OVERLAY` layer shell. This way it does not take focus away from the child processes.

When active, `gameshell` attempts to grab (via `EVIOCGRAB` ioctl) the gamepad device for itself. This prevents the button presses from being received by the child processes.[^2]

Child processes are execed in a new process group. The `STOP` and `CONT` signals are then sent to the process group id, allowing it to work even if command creates multiple processes. This is needed for e.g. `wine`.

`gameshell` also listens to `SIGCHLD` and updates when child is `CLD_EXITED`/`CLD_KILLED`/`CLD_DUMPED` on its own.

## Notes and caveats

I've had good results running Wine games through `gameshell`. The `STOP`/`CONT` feature work like a "Pause" and "Resume" feature, even for games without a pause.

The "grab" feature only works if child processes also use `evdev` for gamepad input. Unfortunately in Linux there are multiple ways to read the gamepad. For example, if a program uses SDL2, it will probably use https://github.com/libusb/hidapi and the `EVIOCGRAB` will not matter. Personally, I run a [sdl2-nohidapi](https://github.com/czak/pkgbuilds/blob/master/sdl2-nohidapi/PKGBUILD) build of SDL2 for this reason.


[^1]: 10-foot user interface: https://en.wikipedia.org/wiki/10-foot_user_interface
[^2]: This only works if child process also uses evdev for gamepad input
