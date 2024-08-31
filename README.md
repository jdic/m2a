# Video to ASCII Art Converter

This script converts a video to an ASCII art representation using `ffmpeg`, `ffprobe` and `jp2a`. It generates a text file containing each frame of the video in ASCII format, respecting the original resolution and frames per second (FPS) rate.

## Requirements

Before running the script, make sure you have the following programs installed:

- [ffmpeg](https://ffmpeg.org/) and [ffprobe](https://ffmpeg.org/): Used to extract frames from the video and get information about the video.
- [jp2a](https://github.com/cslarsen/jp2a): Used to convert images to ASCII art.

## You need

To install `ffmpeg` and `jp2a` on Arch Linux, you can run:

```bash
sudo pacman -S ffmpeg jp2a
```

## Options

```text
Usage: ./v2a.sh [options]

Options:
  -p, --preview          Preview the ASCII video after creation
  -i, --input <file>     Set the input video file (required)
  -o, --output <file>    Set the output ASCII file
  -j, --jp2a <args>      Set the arguments for jp2a
  -f, --fps <number>     Set the frames per second
  -h, --help             Show this help message
  -v, --version          Show version
```
