
#include <cstdio>
#include <memory>
#include <string>

#ifdef USE_CIMG
#include "cimg_backend.hpp"
using namespace visioncpp::utils::cimg;
#else
#include "opencv_backend.hpp"
using namespace visioncpp::utils::opencv;
#endif

namespace visioncpp {
namespace utils {

template <int COLS, int ROWS, int CHANNELS, typename T> class IOHandler {
public:
  IOHandler(char *filename) {
    initMemory<COLS, ROWS, CHANNELS, T>(filename, output_ptr, outputImage,
                                        input_ptr);
  }

  void save(char *output_file) { save(output_file); }

  T *getInputPointer() { return input_ptr; }

  T *getOutputPointer() { return output_ptr.get(); }

private:
  std::shared_ptr<T> output_ptr;
  T *input_ptr;
#ifdef USE_CIMG
  CImg<unsigned char> outputImage;
#else
  cv::Mat outputImage;
#endif
};
} // namespace utils
} // namespace visioncpp
