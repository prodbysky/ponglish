#!/usr/bin/env bash
set -xe

build_assets () {
    gcc bin2c.c -o bin2c
    ./bin2c SpaceMono.ttf
    ./bin2c background.png
    ./bin2c player_bounce.wav
    ./bin2c side_bounce.wav
}


build_assets
gcc main.c -o main -Wall -Wextra --pedantic -lraylib -lm
