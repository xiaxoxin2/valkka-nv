#!/bin/bash
c++ --std=c++14 -fPIC -I. demo.cpp -c
ar rvs demo.a demo.o
