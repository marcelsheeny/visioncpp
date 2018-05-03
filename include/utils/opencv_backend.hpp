
#include <cstdio>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

namespace visioncpp {
namespace utils {
namespace opencv {

template <int COLS, int ROWS, int CHANNELS, typename T>
void initMemory(char *filename, std::shared_ptr<T> output_ptr,
                cv::Mat outputImage, T *input_ptr) {
  // creating a pointer to store the results
  output_ptr =
      std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                         [](unsigned char *dataMem) { delete[] dataMem; });

  printf("Using OpenCV\n");

  cv::Mat input = cv::imread(filename, -1);
  cv::resize(input, input, cv::Size(COLS, ROWS));
  input_ptr = input.data;
  outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
}

void save(char *output_file, cv::Mat outputImage) {
  cv::imwrite(output_file, outputImage);
  cv::imshow("Edge", outputImage);
  cv::waitKey(0);
}

} // namespace opencv
} // namespace utils
} // namespace visioncpp
