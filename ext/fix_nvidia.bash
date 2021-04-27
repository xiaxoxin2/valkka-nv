#!/bin/bash
# find video-sdk-samples/* -exec sed -i -r "s/LogLevel/NvLogLevel/g" {} \;
# find video-sdk-samples/ -name "*.cpp" -exec echo {} \;
# find video-sdk-samples/ -name "*.h" -exec echo {} \;
find video-sdk-samples/ -name "*.cpp" -exec sed -i -r "s/LogLevel/NvLogLevel/g" {} \;
find video-sdk-samples/ -name "*.h" -exec sed -i -r "s/LogLevel/NvLogLevel/g" {} \;
