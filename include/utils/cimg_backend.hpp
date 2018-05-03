
#include <cstdio>
#include <memory>
#include <string>

#include "CImg.h"
using namespace cimg_library;

namespace visioncpp {
namespace utils {
namespace cimg {

template <int COLS, int ROWS, int CHANNELS, typename T>
void initMemory(char *filename, std::shared_ptr<T> output_ptr,
                CImg<T> outputImage, T *input_ptr) {

  // creating a pointer to store the results
  output_ptr =
      std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                         [](unsigned char *dataMem) { delete[] dataMem; });

  //  createMemory<COLS, ROWS, CHANNELS, T>(filename);

  printf("Using CImg\n");

  CImg<unsigned char> input(filename);
  input.resize(COLS, ROWS);
  input = input.RGBtoYCbCr().channel(0);
  input_ptr = input.data();
  // output_ptr = output.get();

  outputImage =
      CImg<unsigned char>(output_ptr.get(), COLS, ROWS, 1, CHANNELS, true);
}

void save(char *output_file, CImg<unsigned char> outputImage) {
  outputImage.save(output_file);
}

} // namespace cimg
} // namespace utils
} // namespace visioncpp
