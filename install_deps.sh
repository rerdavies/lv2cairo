#!/bin/bash
# Install project build dependencies
sudo apt update 
sudo apt install libcairo2-dev libpangocairo-1.0-0 sudo apt install cmake \
    catch2 install librsvg2-dev libserd-dev lv2-dev
sudo apt install liblilv-dev
sudo apt install ninja-build #optional
# Install typescript dependencies for the website
cd website
npm ci
cd ..
