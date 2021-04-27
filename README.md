 
# LibValkka CUDA Accelerated Decoding

libValkka cuda bindings allow you to decode video using NVidia's [cuda video accelerator](https://developer.nvidia.com/nvidia-video-codec-Sdk)

This is not to be confused with the deprecated VDPAU API or with 
[this](https://docs.nvidia.com/drive/drive-os-5.2.0.0L/drive-os/DRIVE_OS_Linux_SDK_Development_Guide/baggage/group__video__decoder__api.html)

To check if your GPU supports hardware decoding, check out [this](https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new#Encoder).

## Installing

### 1. Install libValkka

This is a libValkka extension module, so you need libValkka installed.  Please see [the valkka main page](https://elsampsa.github.io/valkka-examples/_build/html/index.html).

### 2. Install Nvidia drivers and cuda

[This](https://github.com/elsampsa/darknet-python#0-install-cuda) is a nice reference.

### 3. Install Nvidia video SDK

The video sdk source code directory must be placed into [ext/](ext/)  by the name ``video-sdk-samples``.

You can get the video sdk source code by registering with NVidia [here](https://developer.nvidia.com/nvidia-video-codec-Sdk).  There is also a public version available [here](https://github.com/NVIDIA/video-sdk-samples.git).  You can run [ext/download_nvidia.bash](ext/download_nvidia.bash) to download it to the correct place.

The current version of this software has been developed & tested with that aforementioned version of the SDK.

Remember to conform to NVidia's EULA that comes with the software bundle, when distributing your own code.
If you have created source code that's based on their API examples, it seems that you may distribute it in binary form only (!).

### 4. Build

Run
```
./easy_build.bash
```

### 5. Install

Create a debian package and install it:
```
cd build_dir
make package
sudo dpkg -i valkka_nv-*
```

## Usage

Nvidia and cuda related stuff appear under ``valkka.nv`` namespace.  Test that everything is ok with:
```
from valkka.nv import cuda_ok, NVgetDevices

print("Cuda status:", cuda_ok)
devices = NVgetDevices()
print(devices)
```

Cuda-accelerated decoding is performed with ``NVThread``:
```
from valkka.nv import NVThread
...
avthread        =NVThread("avthread", gl_in_filter, 0) # output filter, gpu index
```

``NVThread`` is a drop-in replacement for ``AVThread``, with which you should be 
very familiar if you have done your libValkka tutorials.  :)

See also [this](python/videotest.py)

``NVThread`` tries to use the nvidia cuda video decoders.  If it fails for some reason,
it defaults back to the normal ffmpeg/libav-based decoder.

## Notes

Nvidia's SDK comes with some binary shared-object files:
```
ext/video-sdk-samples/Samples/NvCodec/Lib/linux/stubs/x86_64/
    libnvidia-encode.so
    libnvcuvid.so
```
These seem to be redundant.  If you have installed cuda correctly, you should
have this file in your system:
```
/usr/lib/x86_64-linux-gnu/
    libnvcuvid.so.1
```

# Authors

Xiao Xoxin

# Copyright

(c) Copyright 2021 Xiao Xoxin

# License

For my code, the WTFL license applies.

This software includes code from nvidia.  When distributing it, be sure to conform to their [EULA](https://developer.nvidia.com/nvidia-video-codec-sdk-license-agreement).

