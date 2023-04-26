#!/bin/bash

source ~/.zshrc

compdb -p build/ list > compile_commands.json
