#pragma once
#include <cstdio>
#include <memory>
#include <string>

namespace visioncpp {
namespace utils {
template <int COLS, int ROWS, int CHANNELS, class T>
class rawdata {
 public:
  std::shared_ptr<T> outputImage;
  std::shared_ptr<T> inputImage;

  rawdata() {
    inputImage =
        std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                           [](unsigned char *dataMem) { delete[] dataMem; });

    outputImage =
        std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                           [](unsigned char *dataMem) { delete[] dataMem; });
  }

  void videoCapture(const char *filename) {}

  T *getInputPointer() { return inputImage.get(); }

  T *getOutputPointer() { return outputImage.get(); }

  void imread(const char *filename) {}

  void imread(const T *data) { inputImage = std::shared_ptr<T>(data); }

  void nextFrame() {}

  void save_image_output(char *output_file) {}

  void display_image_output(char *output_file, const int &time) {}

  void display_image_output(char *output_file) {}

  void display_image_input(char *str) {}

  void display_image_input(char *str, const int &time) {}

  void save_image_input(char *output_file) {}
};
}  // namespace utils
}  // namespace visioncpp
