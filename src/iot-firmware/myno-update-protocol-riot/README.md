# MYNO-Device

## Running on native

1. Start the bridge and update server as described in their READMEs.
2. Clone `RIOT` and its submodules: `git submodule update --init --recursive RIOT`
3. Set up the tap interface for the network connection to the native device:

```bash
sudo ip tuntap add tap0 mode tap user $USER
sudo ip link set tap0 up
sudo ip a a fd00::1/64 dev tap0
```
4. Run the device:

```bash
make flash term
```
5. In the device shell, type `start` to set up the MQTT subscription and publish the device description.
6. Use the update server's web UI or scripts to send RPCs or an update image to the device.

## Running on board

The toolchain for compiling and flashing the boards must be installed first.

...TODO...