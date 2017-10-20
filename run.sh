#!/bin/bash
cmake CMakelists.txt
make
sudo nice -n-20 ./gameServer
