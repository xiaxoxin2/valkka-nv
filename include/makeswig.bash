#!/bin/bash
#
# extract everything that we want to expose in python
#
# parameter $1 is the valkka header file directory
valkka_headers="framefilter.h thread.h framefifo.h decoderthread.h"

echo
echo ----------------------
echo GENERATING module.i
echo with valkka header files $valkka_headers 
echo from directory $1
echo -----------------------
echo

headers="*.h"
# init module.i
cat module.i.base > module.i

for header in $valkka_headers
do
  grep -h "<pyapi>" $1/$header | awk '{if ($1=="class" || $1=="enum" || $1=="struct") {print " "}; print $0}' >> module.i
done

for header in $headers
do
  grep -h "<pyapi>" $header | awk '{if ($1=="class" || $1=="enum" || $1=="struct") {print " "}; print $0}' >> module.i
done
