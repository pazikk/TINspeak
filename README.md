# TINspeak

Project dependencies:
- libasound2-dev
- libopus-dev
- JRTPLIB
- JThread
- CMake

Make sure you have basic build tools and CMake installed. If not you can get them with:
```
sudo apt install build-essential
sudo apt install cmake
```

ALSA and Opus can be installed with:
```
sudo apt install libasound2-dev
sudo apt install libopus-dev
```
[JThread](https://github.com/j0r1/JThread) and [JRTPLIB](https://github.com/j0r1/JRTPLIB) have to be cloned, builded and installed manually. 

```
git clone https://github.com/j0r1/JThread.git
cd JThread
cmake .
make
sudo make install
```
```
git clone https://github.com/j0r1/JRTPLIB.git
cd JRTPLIB
cmake .
make
sudo make install
```


Before running use ```alsamixer``` tool in terminal to make sure that capture and playback devices are set to desired gain/volume (and that neither is muted). 
