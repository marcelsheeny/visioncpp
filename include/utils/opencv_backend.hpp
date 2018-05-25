#pragma once
#include <cstdio>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

namespace visioncpp {
namespace utils {
template <int COLS, int ROWS, int CHANNELS, class T>
class opencv {
 public:
  cv::Mat outputImage;
  cv::Mat inputImage;
  cv::VideoCapture cap;

  opencv() { std::cout << "OpenCV IO Backend" << std::endl; }

  void videoCapture(const char *filename) {
    cap = cv::VideoCapture(filename);
    outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS));
    cap >> inputImage;
    cvtColor(inputImage, inputImage, cv::COLOR_BGR2GRAY);
    cv::resize(inputImage, inputImage, cv::Size(COLS, ROWS));
  }

  T *getInputPointer() { return inputImage.data; }

  T *getOutputPointer() { return outputImage.data; }

  void imread(const char *filename) {
    outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS));
    inputImage = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(inputImage, inputImage, cv::Size(COLS, ROWS));
  }

  int nextFrame() {
    cap >> inputImage;
    if (inputImage.data != 0) {
      cvtColor(inputImage, inputImage, cv::COLOR_BGR2GRAY);
      return 0;
    } else {
      return -1;
    }
  }

  void save_image_output(char *output_file) {
    cv::imwrite(output_file, outputImage);
  }

  void display_image_output(char *output_file, const int &time) {
    cv::imshow(output_file, outputImage);
    cv::waitKey(time);
  }

  void display_image_output(char *output_file) {
    cv::imshow(output_file, outputImage);
    cv::waitKey(0);
  }

  void display_image_input(char *str) {
    cv::imshow(str, inputImage);
    cv::waitKey(0);
  }

  void display_image_input(char *str, const int &time) {
    cv::imshow(str, inputImage);
    cv::waitKey(time);
  }

  void save_image_input(char *output_file) {
    cv::imwrite(output_file, inputImage);
  }

};  // namespace utils
}  // namespace utils
}  // namespace visioncpp
