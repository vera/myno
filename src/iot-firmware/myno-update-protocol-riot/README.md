# MYNO-Device

## Running on native

1. Start the bridge and update server as described in their READMEs.
2. Clone `RIOT` and its submodules: `git submodule update --init --recursive RIOT`
3. Run the device:

```bash
make flash term
```
4. In the device shell, type `start` to set up the MQTT subscription and publish the device description.
5. Use the update server's web UI or scripts to send RPCs or update image slices to the device.

## Running on board

The toolchain for compiling and flashing the boards must be installed first.

...TODO...