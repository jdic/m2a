#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <cstdlib>
#include <cstddef>
#include <fstream>
#include <string>
#include <cstdio>
#include <regex>

namespace fs = std::filesystem;

std::string version = "0.0.0-a";

std::string jp2a_args = "--colors";
std::string output_ascii = "";
std::string input_media = "";
std::string fps = "";

bool preview = false;

void help()
{
  std::cout << "Usage: ./m2a [options]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -r, --real-time        Display each frame in real-time during conversion" << std::endl
            << "  -p, --preview          Preview the ASCII video after creation" << std::endl
            << "  -i, --input <file>     Set the input video file (required)" << std::endl
            << "  -j, --jp2a <args>      Set the arguments for jp2a" << std::endl
            << "  -o, --output <file>    Set the output ASCII file" << std::endl
            << "  -f, --fps <number>     Set the frames per second" << std::endl
            << "  -h, --help             Show this help message" << std::endl
            << "  -v, --version          Show version" << std::endl;
  exit(1);
}

std::string get_file_directory(const std::string &file_path)
{
  fs::path path = fs::path(file_path);

  return path.parent_path().string();
}

std::string get_filename_without_ext(const std::string &filename)
{
  return fs::path(filename).stem().string();
}

std::string get_filename_with_ext(const std::string &filename)
{
  return fs::path(filename).filename().string();
}

bool is_numeric(const std::string &str)
{
  return std::regex_match(str, std::regex("[0-9]+"));
}

bool is_url(const std::string &str)
{
  return std::regex_match(str, std::regex(R"((http|https)://([^\s]+))"));
}

void get_version()
{
  std::cout << "v" << version << std::endl;
}

std::string get_temp_dir()
{
  FILE* tempdir_pipe = popen("mktemp -d", "r");

  char buffer[128];
  std::string temp_dir;

  if (fgets(buffer, sizeof(buffer), tempdir_pipe) != nullptr)
  {
    temp_dir = std::string(buffer);

    pclose(tempdir_pipe);

    return temp_dir.substr(0, temp_dir.length() - 1);
  }

  else
  {
    std::cerr << "Error: Could not create temporary directory" << std::endl;
    exit(1);
  }
}

std::string download_file(const std::string &url)
{
  std::string file_name = get_temp_dir() + "/" + get_filename_with_ext(url);

  std::string download_command = "wget -q -O \"" + file_name + "\" \"" + url + "\"";
  int result = system(download_command.c_str());

  if (result != 0)
  {
    std::cerr << "Error: Could not download file from " << url << std::endl;
    exit(1);
  }

  return file_name;
}

int main(int argc, char *argv[])
{
  struct option long_options[] =
  {
    { "input", required_argument, 0, 'i' },
    { "output", required_argument, 0, 'o' },
    { "fps", required_argument, 0, 'f' },
    { "jp2a", required_argument, 0, 'j' },
    { "preview", no_argument, 0, 'p' },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { 0, 0, 0, 0 }
  };

  int option_index = 0;
  int option;

  while ((option = getopt_long(argc, argv, "i:o:f:j:phv", long_options, &option_index)) != -1)
  {
    switch (option)
    {
    case 'i':
      input_media = optarg;
      break;

    case 'o':
      output_ascii = optarg;
      break;

    case 'f':
      fps = optarg;
      if (!is_numeric(fps))
      {
        std::cerr << "Error: FPS must be a number" << std::endl;
        exit(1);
      }
      break;

    case 'j':
      jp2a_args = optarg;
      break;

    case 'p':
      preview = true;
      break;

    case 'h':
      help();
      exit(0);

    case 'v':
      get_version();
      exit(0);

    default:
      help();
      exit(1);
    }
  }

  std::string input_media_origin = input_media;

  if (input_media.empty())
  {
    std::cerr << "Error: input file not set" << std::endl << std::endl;
    help();
    exit(1);
  }

  if (is_url(input_media))
  {
    input_media = download_file(input_media);
  }

  if (output_ascii.empty())
  {
    output_ascii = get_file_directory(input_media) + "/" + get_filename_without_ext(input_media) + ".txt";
  }

  if (fs::exists(output_ascii))
  {
    std::string response;

    std::cout << "Output file '" << output_ascii << "' already exists. Overwrite? (y/n) ";
    std::cin >> response;

    if (response != "y" && response != "Y")
    {
      exit(0);
    }

    fs::remove(output_ascii);
  }

  std::string temp_dir = get_temp_dir();

  if (fps.empty())
  {
    std::string fps_str = "ffprobe -v error -select_streams v:0 -show_entries stream=avg_frame_rate -of default=nokey=1:noprint_wrappers=1 \"" + input_media + "\" 2>&1";
    FILE *fps_pipe = popen(fps_str.c_str(), "r");

    char buffer[128];
    std::string result = "";

    while (fgets(buffer, sizeof(buffer), fps_pipe) != nullptr)
    {
      result += buffer;
    }

    pclose(fps_pipe);

    size_t pos = result.find('/');

    int num = std::stoi(result.substr(0, pos));
    int den = std::stoi(result.substr(pos + 1));

    fps = std::to_string(num / den);

    if (!is_numeric(fps))
    {
      fps = "60";
      std::cerr << "Warning: Could not determine FPS of input video, using default FPS of " << fps << std::endl;
    }
  }

  std::string video_info = "ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 \"" + input_media + "\" 2>&1";
  FILE *info_pipe = popen(video_info.c_str(), "r");

  char buffer[128];
  std::string result = "";

  while (fgets(buffer, sizeof(buffer), info_pipe) != nullptr)
  {
    result += buffer;
  }

  pclose(info_pipe);

  std::string width = result.substr(0, result.find('x'));
  std::string height = result.substr(result.find('x') + 1);

  std::cout << "File        " << get_filename_with_ext(input_media) << std::endl;
  std::cout << "Resolution  " << width << "x" << height;
  std::cout << "FPS         " << fps << std::endl << std::endl;
  std::cout << "Handling frames..." << std::endl;

  std::string ffmpeg_command = "ffmpeg -i \"" + input_media + "\" -r " + fps + " \"" + temp_dir + "/frame_%04d.jpg\" > /dev/null 2>&1";
  system(ffmpeg_command.c_str());

  int total_frames = std::distance(fs::directory_iterator(temp_dir), fs::directory_iterator{});

  std::ofstream ascii_file(output_ascii);
  ascii_file << "# FPS: " << fps << "\n\n";

  int current_frame = 0;

  for (const auto &file : fs::directory_iterator(temp_dir))
  {
    std::string jp2a_command = "jp2a " + jp2a_args + " \"" + file.path().string() + "\"";
    FILE *jp2a_pipe = popen(jp2a_command.c_str(), "r");
    std::string jp2a_output;

    while (fgets(buffer, sizeof(buffer), jp2a_pipe) != nullptr)
    {
      jp2a_output += buffer;
    }

    pclose(jp2a_pipe);

    ascii_file << jp2a_output << "\n";

    current_frame++;
    std::cout << "\r" << current_frame << "/" << total_frames << std::flush;
  }

  ascii_file.close();
  fs::remove(temp_dir);

  if (is_url(input_media_origin))
  {
    fs::remove(input_media);
  }

  std::cout << std::endl << "ASCII video saved to " << output_ascii << std::endl;

  if (preview)
  {
    if (fs::exists("./a2s"))
    {
      if (fs::exists(output_ascii))
      {
        std::string preview_command = "./a2s -i  \"" + output_ascii + "\"";
        system(preview_command.c_str());
      }

      else
      {
        std::cerr << "Error: Output file '" << output_ascii << "' not found. Preview not available." << std::endl;
      }
    }

    else
    {
      std::cerr << "Error: View script './a2s' not found. Cannot preview.\n";
    }
  }

  return 0;
}
