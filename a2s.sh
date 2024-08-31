#!/bin/bash

version="0.0.0-a"

input_ascii=""
fps=""

help()
{
  echo "Usage: $0 [options]"
  echo
  echo "Options:"
  echo "  -i, --input <file>     Set the input ASCII video file (required)"
  echo "  -f, --fps <number>     Set the frames per second"
  echo "  -h, --help             Show this help message"
  echo "  -v, --version          Show version"
  exit 1
}

version()
{
  echo "v$version"
  exit 0
}

move_cursor_home()
{
  echo -ne "\033[H"
}

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -i|--input)
      input_ascii="$2"
      shift
      shift
      ;;
    -f|--fps)
      fps="$2"
      shift
      shift
      ;;
    -h|--help)
      help
      ;;
    -v|--version)
      version
      ;;
    *)
      echo "Unknown option: $key"
      help
      ;;
  esac
done

if [ -z "$input_ascii" ]; then
  echo "Error: Input ASCII file not set"
  echo
  help
fi

current_frame=""
frame_delay=""

{
  read -r res_info
  read -r current_res_info
  read -r fps_info
} < "$input_ascii"

if [[ -z "$fps" ]]; then
  if [[ $fps_info =~ ^#\ FPS:\ ([0-9]+)$ ]]; then
    fps="${BASH_REMATCH[1]}"
  else
    fps=60
  fi
fi

frame_delay=$(awk "BEGIN { print 1/$fps }")

while IFS= read -r line || [ -n "$line" ]; do
  if [ -n "$line" ]; then
    current_frame+="$line\n"
  else
    move_cursor_home
    echo -e "$current_frame"
    sleep "$frame_delay"
    current_frame=""
  fi
done < <(tail -n +4 "$input_ascii")

if [ -n "$current_frame" ]; then
  move_cursor_home
  echo -e "$current_frame"
fi
