#!/bin/bash

version="0.0.0-a"

output_ascii=""
input_video=""
jp2a_args="--colors"
fps=""
preview=false

get_filename_without_ext()
{
  local filename="${1##*/}"
  echo "${filename%.*}"
}

get_filename_with_ext()
{
  echo "${1##*/}"
}

is_numeric()
{
  [[ "$1" =~ ^[0-9]+$ ]]
}

help()
{
  echo "Usage: $0 [options]"
  echo
  echo "Options:"
  echo "  -p, --preview          Preview the ASCII video after creation"
  echo "  -i, --input <file>     Set the input video file (required)"
  echo "  -o, --output <file>    Set the output ASCII file"
  echo "  -j, --jp2a <args>      Set the arguments for jp2a"
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

while [[ $# -gt 0 ]]; do
  key="$1"
  case $key in
    -i|--input)
      input_video="$2"
      shift
      shift
      ;;
    -o|--output)
      output_ascii="$2"
      shift
      shift
      ;;
    -f|--fps)
      fps="$2"
      if ! is_numeric "$fps"; then
        echo "Error: FPS must be a number"
        exit 1
      fi
      shift
      shift
      ;;
    -j|--jp2a)
      jp2a_args="$2"
      shift
      shift
      ;;
    -p|--preview)
      preview=true
      shift
      ;;
    -h|--help)
      help
      ;;
    -v|--version)
      version
      ;;
    *)
      echo "Unknown option: $1"
      help
      ;;
  esac
done

if [ -z "$input_video" ]; then
  echo "Error: input file not set"
  echo
  help
fi

if [ -f "$output_ascii" ]; then
  read -p "Output file '$output_ascii' already exists. Overwrite? (y/n) " -n 1 -r
  echo
  if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$output_ascii"
  else
    exit 0
  fi
fi

if [ -z "$output_ascii" ]; then
  output_ascii="$(get_filename_without_ext "$input_video").txt"
fi

temp_dir=$(mktemp -d)

if [ -z "$fps" ]; then
  fps_str=$(ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=nokey=1:noprint_wrappers=1 "$input_video")
  IFS='/' read -ra parts <<< "$fps_str"
  fps=$((${parts[0]} / ${parts[1]}))
  if ! is_numeric "$fps"; then
    fps=60
    echo "Warning: Could not determine FPS of input video, using default FPS of $fps"
  fi
fi

video_info=$(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 "$input_video")
IFS='x' read -ra video_resolution <<< "$video_info"
width="${video_resolution[0]}"
height="${video_resolution[1]}"

echo "File        $(get_filename_with_ext "$input_video")"
echo "Input Res   ${width}x${height}"
echo "Output Res  $(tput cols)x$(tput lines)"
echo "FPS         $fps"
echo
echo "Handling frames..."

ffmpeg -i "$input_video" -r "$fps" "$temp_dir/frame_%04d.jpg" > /dev/null 2>&1

tput cup 5 0
echo "                    "

total_frames=$(ls "$temp_dir"/frame_*.jpg | wc -l)

echo "# Initial Res: ${width}x${height}" > "$output_ascii"
echo "# Current Res: $(tput cols)x$(tput lines)" >> "$output_ascii"
echo "# FPS: $fps" >> "$output_ascii"
echo >> "$output_ascii"

current_frame=0

for file in "$temp_dir"/frame_*.jpg; do
  jp2a $jp2a_args "$file" >> "$output_ascii"
  echo >> "$output_ascii"

  current_frame=$((current_frame + 1))

  tput cup 5 0
  printf "\r%02d/%02d" "$current_frame" "$total_frames"
done

rm -rf "$temp_dir"

tput cup 5 0
echo "ASCII video saved to $output_ascii"

if [ "$preview" = true ]; then
  if [ -f "./a2s.sh" ]; then
    if [ -f "$output_ascii" ]; then
      ./a2s.sh -i "$output_ascii"
    else
      echo "Error: Output file '$output_ascii' not found. Preview not available."
    fi
  else
    echo "Error: View script './a2s.sh' not found. Cannot preview."
  fi
fi
