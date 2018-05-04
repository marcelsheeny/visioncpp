
#include <cstdio>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

namespace visioncpp {
namespace utils {
namespace opencv {

template <int COLS, int ROWS, int CHANNELS, typename T>
void initMemory(const char *filename, std::shared_ptr<T> &output_ptr,
                cv::Mat &outputImage, std::shared_ptr<T> &input_ptr) {
  // creating a pointer to store the results
  //  output_ptr =
  //      std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
  //                         [](unsigned char *dataMem) { delete[] dataMem; });
  output_ptr =
      std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                         [](unsigned char *dataMem) { delete[] dataMem; });

  printf("Using OpenCV\n");

  cv::Mat input = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
  cv::resize(input, input, cv::Size(COLS, ROWS));
  // input_ptr.reset();
  input_ptr = std::shared_ptr<T>(input.data);
  // input_ptr = input.data;
  outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
}

void save_image_output(char *output_file, cv::Mat outputImage) {
  cv::imwrite(output_file, outputImage);
}

void display_image_output(char *output_file, cv::Mat outputImage) {
  cv::imshow(output_file, outputImage);
  cv::waitKey(0);
}

template <typename T>
void save_image_input(char *output_file, std::shared_ptr<T> input_ptr, int COLS,
                      int ROWS, int CHANNELS) {
  cv::Mat inputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), input_ptr.get());
  cv::imwrite(output_file, inputImage);
}

template <typename T>
void display_image_input(char *str, std::shared_ptr<T> input_ptr, int COLS,
                         int ROWS, int CHANNELS) {
  cv::Mat inputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), input_ptr.get());
  cv::imshow(str, inputImage);
  cv::waitKey(0);
}

}  // namespace opencv
}  // namespace utils
}  // namespace visioncpp
