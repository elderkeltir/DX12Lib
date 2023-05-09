#!/bin/bash

source ~/.zshrc
cd /home/keltir/code/DX12Lib/
mkdir build/src/shaders
compdb -p build/ list > compile_commands.json
