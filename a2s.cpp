#include <iostream>
#include <unistd.h>
#include <getopt.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include <regex>

const std::string version = "0.0.1";

void help()
{
  std::cout << "Usage: ./a2s [options]" << std::endl
            << std::endl
            << "Options:" << std::endl
            << "  -i, --input <file>     Set the input ASCII video file (required)" << std::endl
            << "  -s, --speed <number>   Set the playback speed (e.g., 0.5)" << std::endl
            << "  -f, --fps <number>     Set the frames per second" << std::endl
            << "  -h, --help             Show this help message" << std::endl
            << "  -v, --version          Show version" << std::endl;
  exit(1);
}

void get_version()
{
  std::cout << "v" << version << std::endl;
  exit(0);
}

void move_cursor_home()
{
  std::cout << "\033[H" << std::flush;
}

void hide_cursor()
{
  std::cout << "\033[?25l" << std::flush;
}

void show_cursor()
{
  std::cout << "\033[?25h" << std::flush;
}

int main(int argc, char *argv[])
{
  std::string input_ascii;
  std::string fps;

  double speed = 1.0;

  struct option long_options[] =
  {
    { "input", required_argument, 0, 'i' },
    { "fps", required_argument, 0, 'f' },
    { "speed", required_argument, 0, 's' },
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { 0, 0, 0, 0 }
  };

  int option_index = 0;
  int option;

  while ((option = getopt_long(argc, argv, "i:f:s:hv", long_options, &option_index)) != -1)
  {
    switch (option)
    {
      case 'i':
        input_ascii = optarg;
        break;

      case 'f':
        fps = optarg;
        break;

      case 's':
        speed = std::stod(optarg);
        break;

      case 'h':
        help();
        break;

      case 'v':
        get_version();
        break;

      default:
        std::cerr << "Unknown option: " << option << std::endl;
        help();
        break;
    }
  }

  if (input_ascii.empty())
  {
    std::cerr << "Error: Input ASCII file not set" << std::endl << std::endl;
    help();
  }

  if (speed <= 0)
  {
    std::cerr << "Error: Invalid speed factor. It must be a positive number." << std::endl << std::endl;
    help();
  }

  std::ifstream infile(input_ascii);

  if (!infile.is_open())
  {
    std::cerr << "Error: Cannot open file " << input_ascii << std::endl;
    return 1;
  }

  std::string fps_info;
  std::getline(infile, fps_info);

  if (fps.empty())
  {
    std::regex fps_regex("^# FPS: ([0-9]+)$");
    std::smatch match;

    if (std::regex_search(fps_info, match, fps_regex))
    {
      fps = match[1];
    }

    else
    {
      fps = "60";
    }
  }

  double frame_delay = 1.0 / std::stod(fps);
  frame_delay = frame_delay / speed;

  std::string current_frame;
  std::string line;

  hide_cursor();

  try
  {
    while (std::getline(infile, line))
    {
      if (!line.empty())
      {
        current_frame += line + "\n";
      }

      else
      {
        move_cursor_home();
        std::cout << current_frame << std::flush;
        usleep(static_cast<useconds_t>(frame_delay * 1000000));
        current_frame.clear();
      }
    }

    if (!current_frame.empty())
    {
      move_cursor_home();
      std::cout << current_frame << std::flush;
    }
  }

  catch (...)
  {
    show_cursor();
    throw;
  }

  infile.close();
  show_cursor();
  return 0;
}
