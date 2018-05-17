//
// #include <cstdio>
// #include <memory>
// #include <string>
//
// #include <opencv2/opencv.hpp>
//
// namespace visioncpp {
// namespace utils {
// namespace opencv {
//
// template <int COLS, int ROWS, int CHANNELS, typename T>
// void initMemory(const char *filename, std::shared_ptr<T> &output_ptr,
//                 cv::Mat &outputImage, std::shared_ptr<T> &input_ptr,
//                 cv::Mat &inputImage) {
//   printf("Using OpenCV\n");
//
//   output_ptr = std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS]);
//   inputImage = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
//   cv::resize(inputImage, inputImage, cv::Size(COLS, ROWS));
//   outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
//   input_ptr = std::shared_ptr<T>(inputImage.data);
// }
//
// void save_image_output(char *output_file, cv::Mat outputImage) {
//   cv::imwrite(output_file, outputImage);
// }
//
// void display_image_output(char *output_file, cv::Mat outputImage) {
//   cv::imshow(output_file, outputImage);
//   cv::waitKey(0);
// }
//
// template <typename T>
// void save_image_input(char *output_file, std::shared_ptr<T> input_ptr, int
// COLS,
//                       int ROWS, int CHANNELS) {
//   cv::Mat inputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS),
//   input_ptr.get()); cv::imwrite(output_file, inputImage);
// }
//
// template <typename T>
// void display_image_input(char *str, std::shared_ptr<T> input_ptr, int COLS,
//                          int ROWS, int CHANNELS) {
//   cv::Mat inputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS),
//   input_ptr.get()); cv::imshow(str, inputImage); cv::waitKey(0);
// }
//
// }  // namespace opencv
// }  // namespace utils
// }  // namespace visioncpp
#pragma once
#include <cstdio>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

namespace visioncpp {
namespace utils {
template <int COLS, int ROWS, int CHANNELS, class T> class opencv {

public:
  std::shared_ptr<T> output_ptr;
  std::shared_ptr<T> input_ptr;
  cv::Mat outputImage;
  cv::Mat inputImage;

  opencv() {}
  opencv(const char *filename) {
    printf("Using OpenCV\n");

    output_ptr = std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS]);
    inputImage = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(inputImage, inputImage, cv::Size(COLS, ROWS));
    outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
    input_ptr = std::shared_ptr<T>(inputImage.data);
  }

  void save_image_output(char *output_file) {
    cv::imwrite(output_file, outputImage);
  }

  void display_image_output(char *output_file) {
    cv::imshow(output_file, outputImage);
    cv::waitKey(0);
  }

  void save_image_input(char *output_file) {
    cv::imwrite(output_file, inputImage);
  }

  void display_image_input(char *str) {
    cv::imshow(str, inputImage);
    cv::waitKey(0);
  }
}; // namespace utils
} // namespace utils
} // namespace visioncpp
