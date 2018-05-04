
#include <iostream>
#include <string>

#ifdef USE_CIMG
#include "CImg.h"
using namespace cimg_library;
#else
#include <opencv2/opencv.hpp>
#endif

/* Implemented interface. */
class AbstractInterface {
 public:
  virtual void someFunctionality() = 0;
};

/* Interface for internal implementation that Bridge uses. */
class ImplementationHandler {
 public:
  virtual void read(char* filename) = 0;
  virtual void save(char* output_file) = 0;
  virtual void display(char* output_file) = 0;

 private:
  std::shared_ptr<T> output_ptr;
  std::shared_ptr<T> input_ptr;
#ifdef USE_CIMG
  CImg<unsigned char> outputImage;
#else
  cv::Mat outputImage;
#endif
};

/* The Bridge */
class Bridge : public AbstractInterface {
 protected:
  ImplementationHandler* implementation;

 public:
  Bridge(ImplementationHandler* backend) { implementation = backend; }
};

/* Different special cases of the interface. */

class UseCase : public Bridge {
 public:
  UseCase(ImplementationHandler* backend) : Bridge(backend) {}

  void read(char* filename) { implementation->read(filename); }
  void save(char* filename) { implementation->save(filename); }
  void display(char* filename) { implementation->display(filename); }
};

/* Different background implementations. */
template <int COLS, int ROWS, int CHANNELS, typename T>
class opencv_backend : public ImplementationHandler {
 public:
  void read(const char* filename) {
    output_ptr =
        std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                           [](unsigned char* dataMem) { delete[] dataMem; });

    printf("Using OpenCV\n");

    cv::Mat input = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(input, input, cv::Size(COLS, ROWS));
    input_ptr = std::shared_ptr<T>(input.data);
    outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
  }

  void save(char* output_file) { cv::imwrite(output_file, outputImage); }

  void display(char* output_file) {
    cv::imshow(output_file, outputImage);
    cv::waitKey(0);
  }
};

class cimg_backend : public ImplementationHandler {
 public:
  void read(const char* filename) {
    output_ptr =
        std::shared_ptr<T>(new unsigned char[COLS * ROWS * CHANNELS],
                           [](unsigned char* dataMem) { delete[] dataMem; });

    printf("Using OpenCV\n");

    cv::Mat input = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
    cv::resize(input, input, cv::Size(COLS, ROWS));
    input_ptr = std::shared_ptr<T>(input.data);
    outputImage = cv::Mat(ROWS, COLS, CV_8UC(CHANNELS), output_ptr.get());
  }

  void save(char* output_file) { outputImage.save(output_file); }

  void display(char* output_file) { outputImage.display(output_file); }
};

class IOHandler {
  IOHandler() {
#ifdef USE_CIMG
    ImplementationInterface* backend = new opencv_backend;
#elif USE_OPENCV
    ImplementationInterface* backend = new cimg_backend;
#else
    ImplementationInterface* backend = new opencv_backend;
}
void read(char* filename){
  useCase->read(filename);
}

void save(char* filename){
  useCase->save(filename);
}

void display(char* filename){
  useCase->display(filename);
}

private:
  UseCase useCase = new UseCase(backend);
  ImplementationInterface* backend = 0;
  AbstractInterface* useCase = 0;
};
