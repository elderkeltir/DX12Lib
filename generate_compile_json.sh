#!/bin/bash

source ~/.zshrc
cd /usr/home/keltir/Code/DX12Lib/
mkdir build/src/shaders
compdb -p build/ list > compile_commands.json
