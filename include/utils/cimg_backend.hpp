#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "CImg.h"

using namespace cimg_library;

namespace visioncpp {
namespace utils {

template <int COLS, int ROWS, int CHANNELS, typename T>
class cimg {
 public:
  //  std::shared_ptr<T> output_ptr;
  //  std::shared_ptr<T> input_ptr;
  CImg<T> outputImage;
  CImg<T> inputImage;
  std::vector<std::string> files;
  int i = 0;
  CImgDisplay main_disp;

  cimg() {}

  void imread(const char *filename) {
    inputImage = CImg<T>(filename);
    inputImage.resize(COLS, ROWS);
    inputImage = inputImage.RGBtoYCbCr().channel(0);
    outputImage = CImg<T>(COLS, ROWS, 1, CHANNELS, true);
  }

  T *getInputPointer() { return inputImage.data(); }
  T *getOutputPointer() { return outputImage.data(); }

  void nextFrame() { inputImage = CImg<T>(files[i++].c_str()); }

  void videoCapture(const char *filename) {
    files = list_files(std::string(filename));
    nextFrame();
    inputImage.resize(COLS, ROWS);
    inputImage = inputImage.RGBtoYCbCr().channel(0);
    outputImage = CImg<T>(COLS, ROWS, 1, CHANNELS, true);

    main_disp = CImgDisplay(inputImage, "output result");
  }

  void save_image_output(char *output_file) { outputImage.save(output_file); }

  void display_image_output(char *output_file, const int &time) {
    main_disp = outputImage;
    main_disp.wait(time);
  }

  void display_image_output(char *output_file) {
    outputImage.display(output_file);
  }

  void display_image_input(char *output_file, const int &time) {
    main_disp = inputImage;
    main_disp.wait(time);
  }

  void display_image_input(char *output_file) {
    inputImage.display(output_file);
  }

  void save_image_input(char *output_file) { inputImage.save(output_file); }

  std::vector<std::string> list_files(std::string folder) {
    std::vector<std::string> file_list;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder.c_str())) != NULL) {
      /* print all the files and directories within directory */
      while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
          std::string fullpath = std::string(folder + ent->d_name);
          printf("%s\n", fullpath.c_str());
          file_list.push_back(fullpath);
        }
      }
      closedir(dir);
    } else {
      /* could not open directory */
      perror("");
    }
    std::sort(file_list.begin(), file_list.end());
    return file_list;
  }
};

}  // namespace utils
}  // namespace visioncpp
