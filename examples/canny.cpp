// This file is part of VisionCPP, a lightweight C++ template library
// for computer vision and image processing.
//
// Copyright (C) 2016 Codeplay Software Limited. All Rights Reserved.
//
// Contact: visioncpp@codeplay.com
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/// \file edge_detector.cc
///
///             -------Example--------
///              Canny Edge detector
///
/// \brief This example implements the Canny Edge Detector
///

// include OpenCV for camera display
#include <opencv2/opencv.hpp>

// include VisionCpp
#include <visioncpp.hpp>

// \brief Functor which implements the canny edge detector
struct Canny {
  template <typename NeighbourT>
  float operator()(const NeighbourT p) {
    float output;
    float a11 = p.at(p.I_c - 1, p.I_r - 1)[0];
    float a12 = p.at(p.I_c, p.I_r - 1)[0];
    float a13 = p.at(p.I_c + 1, p.I_r - 1)[0];
    float a21 = p.at(p.I_c - 1, p.I_r)[0];
    float a22 = p.at(p.I_c, p.I_r)[0];
    float a23 = p.at(p.I_c + 1, p.I_r)[0];
    float a31 = p.at(p.I_c - 1, p.I_r + 1)[0];
    float a32 = p.at(p.I_c, p.I_r + 1)[0];
    float a33 = p.at(p.I_c + 1, p.I_r + 1)[0];

    float b11 = p.at(p.I_c - 1, p.I_r - 1)[1];
    float b12 = p.at(p.I_c, p.I_r - 1)[1];
    float b13 = p.at(p.I_c + 1, p.I_r - 1)[1];
    float b21 = p.at(p.I_c - 1, p.I_r)[1];
    float b22 = p.at(p.I_c, p.I_r)[1];
    float b23 = p.at(p.I_c + 1, p.I_r)[1];
    float b31 = p.at(p.I_c - 1, p.I_r + 1)[1];
    float b32 = p.at(p.I_c, p.I_r + 1)[1];
    float b33 = p.at(p.I_c + 1, p.I_r + 1)[1];

    float xGrad = a22;
    float yGrad = b22;
    float gradMag = cl::sycl::hypot(xGrad, yGrad);

    // perform non-maximal supression
    float mag11 = cl::sycl::hypot(a11, b11);
    float mag12 = cl::sycl::hypot(a12, b12);
    float mag13 = cl::sycl::hypot(a13, b13);

    float mag21 = cl::sycl::hypot(a21, b21);
    float mag23 = cl::sycl::hypot(a23, b23);

    float mag33 = cl::sycl::hypot(a33, b33);
    float mag32 = cl::sycl::hypot(a32, b32);
    float mag31 = cl::sycl::hypot(a31, b31);

    float tmp;

    if (xGrad * yGrad <= 0.0f                                /*(1)*/
            ? cl::sycl::fabs(xGrad) >= cl::sycl::fabs(yGrad) /*(2)*/
                  ? (tmp = cl::sycl::fabs(xGrad * gradMag)) >=
                            cl::sycl::fabs(yGrad * mag13 -
                                           (xGrad + yGrad) * mag23)
                        /*(3)*/
                        &&
                        tmp > cl::sycl::fabs(yGrad * mag31 -
                                             (xGrad + yGrad) * mag21) /*(4)*/
                  : (tmp = cl::sycl::fabs(yGrad * gradMag)) >=
                            cl::sycl::fabs(xGrad * mag13 -
                                           (yGrad + xGrad) * mag12)
                        /*(3)*/
                        &&
                        tmp > cl::sycl::fabs(xGrad * mag31 -
                                             (yGrad + xGrad) * mag32) /*(4)*/
            : cl::sycl::fabs(xGrad) >= cl::sycl::fabs(yGrad)          /*(2)*/
                  ? (tmp = cl::sycl::fabs(xGrad * gradMag)) >=
                            cl::sycl::fabs(yGrad * mag33 +
                                           (xGrad - yGrad) * mag23)
                        /*(3)*/
                        &&
                        tmp > cl::sycl::fabs(yGrad * mag11 +
                                             (xGrad - yGrad) * mag21) /*(4)*/
                  : (tmp = cl::sycl::fabs(yGrad * gradMag)) >=
                            cl::sycl::fabs(xGrad * mag33 +
                                           (yGrad - xGrad) * mag32)
                        /*(3)*/
                        &&
                        tmp > cl::sycl::fabs(xGrad * mag11 +
                                             (yGrad - xGrad) * mag12) /*(4)*/
        ) {
      output = gradMag;

    } else {
      output = 0;
    }
    return output;
  }
};

// main program
int main(int argc, char** argv) {
  // open camera using OpenCV
  cv::VideoCapture cap(0);  // open the default camera
  if (!cap.isOpened()) {    // check if we succeeded
    std::cout << "Opening Camera Failed." << std::endl;
    return -1;
  }

  // selecting device
  auto dev = visioncpp::make_device<visioncpp::backend::sycl,
                                    visioncpp::device::cpu>();
  // defining the image size constants
  constexpr size_t COLS = 640;
  constexpr size_t ROWS = 480;

  // init input image
  cv::Mat input;

  // creating a pointer to store the results
  std::shared_ptr<unsigned char> output(
      new unsigned char[COLS * ROWS],
      [](unsigned char* dataMem) { delete[] dataMem; });

  // init output image from OpenCV for displaying results
  cv::Mat outputImage(ROWS, COLS, CV_8UC1, output.get());

  // initializing the mask memories
  float sobel_x[9] = {-1.0f, 0.0f, 1.0f, -2.0f, 0.0f, 2.0f, -1.0f, 0.0f, 1.0f};
  float sobel_y[9] = {-1.0f, -2.0f, -1.0f, 0.0, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f};

  // defining the mean filter size: (filter_size x filter_size)
  constexpr int filter_size = 9;
  float meanMask[filter_size] = {1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
                                 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
                                 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f};

  for (;;) {
    // Starting building the tree (use  {} during the creation of the tree)
    {
      // read frame
      cap.read(input);

      // resize input for the desirable size
      cv::resize(input, input, cv::Size(COLS, ROWS), 0, 0, cv::INTER_CUBIC);

      // the node which gets the input data from OpenCV
      auto in =
          visioncpp::terminal<visioncpp::pixel::U8C3, COLS, ROWS,
                              visioncpp::memory_type::Buffer2D>(input.data);

      // the node which gets the output data
      auto out =
          visioncpp::terminal<visioncpp::pixel::U8C1, COLS, ROWS,
                              visioncpp::memory_type::Buffer2D>(output.get());

      // convert to Float
      auto frgb = visioncpp::point_operation<visioncpp::OP_U8C3ToF32C3>(in);

      // convert to grey scale
      auto fgrey = visioncpp::point_operation<visioncpp::OP_RGBToGREY>(frgb);

      // apply mean filter to smooth the image
      auto mean_col = visioncpp::terminal<float, filter_size, 1,
                                          visioncpp::memory_type::Buffer2D,
                                          visioncpp::scope::Constant>(meanMask);
      auto mean_row = visioncpp::terminal<float, 1, filter_size,
                                          visioncpp::memory_type::Buffer2D,
                                          visioncpp::scope::Constant>(meanMask);
      auto mean_1 = visioncpp::neighbour_operation<visioncpp::OP_SepFilterCol>(
          fgrey, mean_col);
      auto mean = visioncpp::neighbour_operation<visioncpp::OP_SepFilterRow>(
          mean_1, mean_row);

      // applying sobel_x filter
      auto x_filter =
          visioncpp::terminal<float, 3, 3, visioncpp::memory_type::Buffer2D,
                              visioncpp::scope::Constant>(sobel_x);
      auto sobel_x = visioncpp::neighbour_operation<visioncpp::OP_Filter2D_One>(
          mean, x_filter);

      auto y_filter =
          visioncpp::terminal<float, 3, 3, visioncpp::memory_type::Buffer2D,
                              visioncpp::scope::Constant>(sobel_y);
      auto sobel_y = visioncpp::neighbour_operation<visioncpp::OP_Filter2D_One>(
          mean, y_filter);

      auto merge = visioncpp::point_operation<visioncpp::OP_Merge2Chns>(
          sobel_x, sobel_y);

      auto canny = visioncpp::neighbour_operation<Canny, 1, 1, 1, 1>(merge);

      // apply a threshold
      auto thresh_node =
          visioncpp::terminal<float, visioncpp::memory_type::Const>(
              static_cast<float>(0.2));
      auto cannyTresh =
          visioncpp::point_operation<visioncpp::OP_Thresh>(canny, thresh_node);

      auto scale_node =
          visioncpp::terminal<float, visioncpp::memory_type::Const>(
              static_cast<float>(255.0f));
      auto uCannyTresh = visioncpp::point_operation<visioncpp::OP_Scale>(
          cannyTresh, scale_node);

      // assign operation
      auto pipe = visioncpp::assign(out, uCannyTresh);

      // execute the pipe
      visioncpp::execute<visioncpp::policy::Fuse, 8, 8, 8, 8>(pipe, dev);
    }

    // Display results
    cv::imshow("Reference Image", input);
    cv::moveWindow("Reference Image", 0, 0);
    cv::imshow("Canny Edge Detector", outputImage);
    cv::moveWindow("Canny Edge Detector", COLS + 65, 0);

    // check button pressed to finalize program
    if (cv::waitKey(1) >= 0) break;
  }

  return 0;
}
