
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

/// \file harris.cc
///
///             -------Example--------
///             Harris Corner Detector
///
/// \brief This example implements Harris Corner Detector
/// The Harris Corner Detector creates a matrix M based on its derivatives
/// M = sum w | Dx^2  Dx*Dy |
///            | Dx*Dy Dy^2 |
/// where w is the a 3x3 window of ones
/// the corners are detected based on the formula
/// H = det(M)-k*(trace(M)^2)

// include VisionCpp
#include <visioncpp.hpp>

// tunable parameter for the Harris
constexpr float k_param = 0.04f;   // k parameter (usually 0.02 - 0.04)
constexpr float threshold = 0.5f;  // threhold parameter
constexpr int windowSize = 7;      // window size for non-maximal suppresion
constexpr int halfWindowSize = windowSize / 2;  // half window size

// Below a set of operators created to implement the algorithm

// operator created to perform a power of 2 of the image
struct PowerOf2 {
  template <typename T>
  const float operator()(const T &t) {
    return t * t;
  }
};

// operator for element-wise multiplication of two images
struct Mul {
  template <typename T1, typename T2>
  float operator()(const T1 &t1, const T2 &t2) {
    return t1 * t2;
  }
};

// operator to add two images
struct Add {
  template <typename T1, typename T2>
  float operator()(const T1 &t1, const T2 &t2) {
    return t1 + t2;
  }
};

// operator to subtract two images
struct Sub {
  template <typename T1, typename T2>
  float operator()(const T1 &t1, const T2 &t2) {
    return t1 - t2;
  }
};

// convolution for a custom filter in a one-dimensional image
struct Filter2D {
  template <typename T1, typename T2>
  float operator()(const T1 &nbr, const T2 &fltr) {
    int hs_c = (fltr.cols / 2);
    int hs_r = (fltr.rows / 2);

    float out = 0;
    for (int i2 = -hs_c, i = 0; i2 <= hs_c; i2++, i++)
      for (int j2 = -hs_r, j = 0; j2 <= hs_r; j2++, j++)
        out += (nbr.at(nbr.I_c + i2, nbr.I_r + j2) * fltr.at(i, j));
    return out;
  }
};

// Convert from float to unsigned char one channel
struct FloatToU8C1 {
  visioncpp::pixel::U8C1 operator()(const float &t) {
    return visioncpp::pixel::U8C1(static_cast<unsigned char>(t * 255));
  }
};

// apply threshold operation to the image
struct Thresh {
  template <typename T, typename Thresh>
  float operator()(const T &t, const Thresh &thresh) {
    return t > thresh ? 1.0f : 0.0f;
  }
};
// non-maximal suppresion, supress all values which are not the maximum in a
// neighbourhood
struct NonMaximalSuppresion {
  template <typename T>
  float operator()(const T &im) {
    float currentPixel{im.at(im.I_c, im.I_r)};
    for (int i = -halfWindowSize; i <= halfWindowSize; i++) {
      for (int j = -halfWindowSize; j <= halfWindowSize; j++) {
        if (currentPixel < im.at(im.I_c + i, im.I_r + j)) {
          return 0.0f;
        }
      }
    }
    return currentPixel;
  }
};

// main program
int main(int argc, char **argv) {
  // selecting device
  auto dev = visioncpp::make_device<visioncpp::backend::sycl,
                                    visioncpp::device::cpu>();
  // defining the image size constants
  constexpr size_t COLS = 1280;
  constexpr size_t ROWS = 720;
  constexpr size_t CHANNELS = 1;

  // Shared Memory variable
  constexpr size_t SM = 16;

  visioncpp::utils::IOHandler<COLS, ROWS, CHANNELS, unsigned char> ioHandler;

  ioHandler.videoCapture(argv[1]);

  // initializing the mask memories
  float sobel_x[9] = {-1.0f, 0.0f, 1.0f, -2.0f, 0.0f, 2.0f, -1.0f, 0.0f, 1.0f};
  float sobel_y[9] = {-1.0f, -2.0f, -1.0f, 0.0, 0.0f, 0.0f, 1.0f, 2.0f, 1.0f};
  float sum_mask[9] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

  for (;;) {
    // Starting building the tree (use  {} during the creation of the tree)
    {
      // read frame
      if (ioHandler.nextFrame() < 0) break;

      // the node which gets the input data from OpenCV
      auto in = visioncpp::terminal<visioncpp::pixel::U8C1, COLS, ROWS,
                                    visioncpp::memory_type::Buffer2D>(
          ioHandler.getInputPointer());

      // the node which gets the output data
      auto out = visioncpp::terminal<visioncpp::pixel::U8C1, COLS, ROWS,
                                     visioncpp::memory_type::Buffer2D>(
          ioHandler.getOutputPointer());

      // convert to Float
      auto fgrey = visioncpp::point_operation<visioncpp::OP_U8C1ToFloat>(in);

      // applying derivative in X direction
      auto px_filter =
          visioncpp::terminal<float, 3, 3, visioncpp::memory_type::Buffer2D,
                              visioncpp::scope::Constant>(sobel_x);
      auto px = visioncpp::neighbour_operation<Filter2D>(fgrey, px_filter);

      // applying derivative in Y direction
      auto py_filter =
          visioncpp::terminal<float, 3, 3, visioncpp::memory_type::Buffer2D,
                              visioncpp::scope::Constant>(sobel_y);
      auto py = visioncpp::neighbour_operation<Filter2D>(fgrey, py_filter);

      // starting building the M matrix
      auto px2 = visioncpp::point_operation<PowerOf2>(px);
      auto py2 = visioncpp::point_operation<PowerOf2>(py);
      auto pxy = visioncpp::point_operation<Mul>(px, py);

      // breaking the tree before convolution for a better use of shared memory
      auto kpx2 =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(px2);
      auto kpy2 =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(py2);
      auto kpxy =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(pxy);

      // Summing neighbours
      auto sum_mask_node =
          visioncpp::terminal<float, 3, 3, visioncpp::memory_type::Buffer2D,
                              visioncpp::scope::Constant>(sum_mask);

      auto sumpx2 =
          visioncpp::neighbour_operation<Filter2D>(kpx2, sum_mask_node);
      auto sumpy2 =
          visioncpp::neighbour_operation<Filter2D>(kpy2, sum_mask_node);
      auto sumpxy =
          visioncpp::neighbour_operation<Filter2D>(kpxy, sum_mask_node);

      // breaking the tree after convolution
      auto ksumpx2 =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(sumpx2);
      auto ksumpy2 =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(sumpy2);
      auto ksumpxy =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(sumpxy);

      // applying the formula det(M)-k*(trace(M)^2)
      // det(M) = (ksumpx2*ksumpy2 - ksumpxy*ksumpxy)
      auto mul1 = visioncpp::point_operation<Mul>(ksumpx2, ksumpy2);
      auto mul2 = visioncpp::point_operation<PowerOf2>(ksumpxy);
      auto det = visioncpp::point_operation<Sub>(mul1, mul2);

      // trace(M) = ksumpx2 + ksumpy2
      auto trace = visioncpp::point_operation<Add>(ksumpx2, ksumpy2);

      // trace(M)^2
      auto trace2 = visioncpp::point_operation<PowerOf2>(trace);

      // k*(trace(M)^2)
      auto k_node =
          visioncpp::terminal<float, visioncpp::memory_type::Const>(k_param);
      auto ktrace2 = visioncpp::point_operation<Mul>(trace2, k_node);

      // harris = det(M)-k*(trace(M)^2)
      auto harris = visioncpp::point_operation<Sub>(det, ktrace2);

      // break tree before neighbour_operation
      auto kharris =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(harris);

      auto harris_non_maximum =
          visioncpp::neighbour_operation<NonMaximalSuppresion, halfWindowSize,
                                         halfWindowSize, halfWindowSize,
                                         halfWindowSize>(kharris);

      // break tree after neighbour_operation
      auto kharris_non_maximum =
          visioncpp::schedule<visioncpp::policy::Fuse, SM, SM, SM, SM>(
              harris_non_maximum);

      // apply a threshold
      auto thresh_node =
          visioncpp::terminal<float, visioncpp::memory_type::Const>(
              static_cast<float>(threshold));
      auto harrisTresh =
          visioncpp::point_operation<Thresh>(kharris_non_maximum, thresh_node);

      // convert to unsigned char for displaying purposes
      // scale threhold to display
      auto scale_node =
          visioncpp::terminal<float, visioncpp::memory_type::Const>(
              static_cast<float>(255.0f));
      auto display = visioncpp::point_operation<visioncpp::OP_Scale>(
          harrisTresh, scale_node);

      // assign to the output
      auto exec = visioncpp::assign(out, display);

      // execute expression tree
      visioncpp::execute<visioncpp::policy::Fuse, SM, SM, SM, SM>(exec, dev);
    }

    ioHandler.displayOutput("Harris", 1);
  }

  return 0;
}
