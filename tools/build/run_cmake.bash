#!/bin/bash
## So you're doing development yourself with libValkka & this external module
## TODO: put correct directories in-place:

echo "TODO: EDIT THIS"
exit(2)

cmake -Dvalkka_lib=WHERE_IS_YOUR_CUSTOM_BUILT_LIBVALKKA/build_dev/lib \
-Dvalkka_root=WHERE_IS_YOUR_VALKKA_SOURCE_CODE \
-Dffmpeg_root=WHERE_IS_YOUR_VALKKA_SOURCE_CODE/ext/ffmpeg \
-Dlive555_root=WHERE_IS_YOUR_VALKKA_SOURCE_CODE/ext/live \
-Dnvcodec_root=WHERE_IS_THIS_EXTENSION_MODULE_SOURCE_CODE/ext/video-sdk-samples \
.. 
