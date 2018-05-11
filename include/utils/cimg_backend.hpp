
#include <cstdio>
#include <memory>
#include <string>

#include "CImg.h"
using namespace cimg_library;

namespace visioncpp {
namespace utils {
namespace cimg {

template <int COLS, int ROWS, int CHANNELS, typename T>
void initMemory(const char *filename, std::shared_ptr<T> &output_ptr,
                CImg<T> &outputImage, std::shared_ptr<T> &input_ptr,
                CImg<T> &inputImage) {
  // creating a pointer to store the results
  output_ptr =
      std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                         [](unsigned char *dataMem) { delete[] dataMem; });

  printf("Using CImg\n");

  inputImage = CImg<unsigned char>(filename);
  inputImage.resize(COLS, ROWS);
  inputImage = inputImage.RGBtoYCbCr().channel(0);
  input_ptr = std::shared_ptr<T>(
      inputImage.data(), [](unsigned char *dataMem) { delete[] dataMem; });

  outputImage =
      CImg<unsigned char>(output_ptr.get(), COLS, ROWS, 1, CHANNELS, true);
}

void save_image_output(char *output_file, CImg<unsigned char> outputImage) {
  outputImage.save(output_file);
}

void display_image_output(char *output_file, CImg<unsigned char> outputImage) {
  outputImage.display(output_file);
}

template <typename T>
void save_image_input(char *output_file, std::shared_ptr<T> input_ptr, int COLS,
                      int ROWS, int CHANNELS) {
  CImg<unsigned char> inputImage =
      CImg<unsigned char>(input_ptr.get(), COLS, ROWS, 1, CHANNELS, true);
  inputImage.save(output_file);
}

template <typename T>
void display_image_input(char *output_file, std::shared_ptr<T> input_ptr,
                         int COLS, int ROWS, int CHANNELS) {
  CImg<unsigned char> inputImage =
      CImg<unsigned char>(input_ptr.get(), COLS, ROWS, 1, CHANNELS, true);
  inputImage.display(output_file);
}

}  // namespace cimg
}  // namespace utils
}  // namespace visioncpp
