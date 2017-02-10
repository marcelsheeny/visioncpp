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

/// \file anisotropic_diffusion.cpp
///
///              ---------Example---------
///            Simplified Anisotropic Diffusion
///
/// \brief This example implements a simplified version of the Perona-Malik
/// Anisotropic Diffusion.
/// This technique removes noise from an image preserving the edges.
/// \param k - the edge preserving parameter [the smaller is the k, less things
///       are considered edges, the bigger, more things are considered as edges]
/// \param iters - number of iterations [it controls how blurry the image will
///        become, a higher number means a more blurry image]

// include OpenCV for camera display
#include <opencv2/opencv.hpp>

// include VisionCpp
#include <visioncpp.hpp>

// tunable parameters
constexpr float k{15.0f};    // edge preserving parameter
constexpr size_t iters{15};  // controls the blur

// operator which implements the simplified anisotropic diffusion
struct AniDiff {
  template <typename T>
  visioncpp::pixel::F32C3 operator()(T nbr) {
    // init output pixel
    // visioncpp::pixel::F32C3 out(0.0f, 0.0f, 0.0f);
    cl::sycl::float4 out(0, 0, 0, 0);

    // init sum variable, which is used to normalize
    // visioncpp::pixel::F32C3 sum_w(0.0f, 0.0f, 0.0f);
    cl::sycl::float4 sum_w(0, 0, 0, 0);

    // get center pixel
    // visioncpp::pixel::F32C3 p1(nbr.at(nbr.I_c, nbr.I_r)[0],
    //                           nbr.at(nbr.I_c, nbr.I_r)[1],
    //                           nbr.at(nbr.I_c, nbr.I_r)[2]);
    cl::sycl::float4 p1(nbr.at(nbr.I_c, nbr.I_r)[0],
                        nbr.at(nbr.I_c, nbr.I_r)[1],
                        nbr.at(nbr.I_c, nbr.I_r)[2], 0);

    // iterate over a 3x3 neighbourhood
    for (int i = -1; i <= 1; i++) {
      for (int j = -1; j <= 1; j++) {
        // get neighbour pixel
        // visioncpp::pixel::F32C3 p2(nbr.at(nbr.I_c + i, nbr.I_r + j)[0],
        //                           nbr.at(nbr.I_c + i, nbr.I_r + j)[1],
        //                           nbr.at(nbr.I_c + i, nbr.I_r + j)[2]);
        cl::sycl::float4 p2(nbr.at(nbr.I_c + i, nbr.I_r + j)[0],
                            nbr.at(nbr.I_c + i, nbr.I_r + j)[1],
                            nbr.at(nbr.I_c + i, nbr.I_r + j)[2], 0);

        // computes the weight which basically is the difference between pixels
        // visioncpp::pixel::F32C3 absp(cl::sycl::fabs(p1[0] - p2[0]),
        //                             cl::sycl::fabs(p1[1] - p2[1]),
        //                             cl::sycl::fabs(p1[2] - p2[2]));
        // visioncpp::pixel::F32C3 w(cl::sycl::exp((-k) * absp[0]),
        //                          cl::sycl::exp((-k) * absp[1]),
        //                          cl::sycl::exp((-k) * absp[2]));
        cl::sycl::float4 w = cl::sycl::exp((-k) * cl::sycl::fabs(p1 - p2));

        // sum the weights for normalization
        sum_w += w;

        // store the output
        out += w * p2;
      }
    }
    // normalize output and return
    out = out / sum_w;
    return visioncpp::pixel::F32C3(out.x(), out.y(), out.z());
    // return out / sum_w;
  }
};

// main program
int main(int argc, char **argv) {
  // open video or camera
  cv::VideoCapture cap;

  if (argc == 1) {
    cap.open(0);
    std::cout << "To use video" << std::endl;
    std::cout << "example>: ./example path/to/video.avi" << std::endl;
  } else if (argc > 1) {
    cap.open(argv[1]);
  }

  // check if we succeeded
  if (!cap.isOpened()) {
    std::cout << "Opening Camera/Video Failed." << std::endl;
    return -1;
  }

  // selecting device using sycl as backend
  auto dev = visioncpp::make_device<visioncpp::backend::sycl,
                                    visioncpp::device::cpu>();

  // defining size constants
  constexpr size_t COLS = 640;
  constexpr size_t ROWS = 480;
  constexpr size_t CHNS = 3;

  // initialising output pointer
  std::shared_ptr<uchar> output(new uchar[COLS * ROWS * CHNS],
                                [](uchar *dataMem) { delete[] dataMem; });

  // initializing input and output image
  cv::Mat input;
  cv::Mat outImage(ROWS, COLS, CV_8UC(CHNS), output.get());

  /*
   This example contains a small expression tree
   but it uses a device memory which stores a temporary computation.
   So it is possible to create a loop during the computation
   and the data is always in the device. It just comes to the host in the last
   execute.

   Below is the expression tree used for this example

        (in_node)
         |
        (frgb)     [OP_U8C3ToF32C3] (convert uchar to float)
         |
   ---->(anidiff)  [AniDiff] (It iterates several times in the same node
   |     |                    applying the anisotropic diffusion serveral times)
   -------
         |
        (urgb)     [OP_F32C3ToU8C3] (convert float to uchar to display)


  */

  for (;;) {
    // Starting building the tree (use  {} during the creation of the tree)
    {
      // read frame
      cap.read(input);

      // check if image was loaded
      if (!input.data) {
        break;
      }

      // resize image to the desirable size
      cv::resize(input, input, cv::Size(COLS, ROWS), 0, 0, cv::INTER_CUBIC);

      auto in_node =
          visioncpp::terminal<visioncpp::pixel::U8C3, COLS, ROWS,
                              visioncpp::memory_type::Buffer2D>(input.data);
      auto out_node =
          visioncpp::terminal<visioncpp::pixel::U8C3, COLS, ROWS,
                              visioncpp::memory_type::Buffer2D>(output.get());

      // device only memory
      auto device_memory =
          visioncpp::terminal<visioncpp::pixel::F32C3, COLS, ROWS,
                              visioncpp::memory_type::Buffer2D>();

      // convert to float
      auto frgb =
          visioncpp::point_operation<visioncpp::OP_U8C3ToF32C3>(in_node);

      // assign to temporary device memory
      auto exec1 = visioncpp::assign(device_memory, frgb);

      // apply anisotropic diffusion
      auto anidiff =
          visioncpp::neighbour_operation<AniDiff, 1, 1, 1, 1>(device_memory);

      // assign to the temporary device memory
      auto exec2 = visioncpp::assign(device_memory, anidiff);

      // convert to uchar
      auto urgb =
          visioncpp::point_operation<visioncpp::OP_F32C3ToU8C3>(device_memory);

      // assign to the host memory
      auto exec3 = visioncpp::assign(out_node, urgb);

      // execution (convert to float)
      visioncpp::execute<visioncpp::policy::Fuse, 32, 32, 16, 16>(exec1, dev);

      // apply anisotropic diffusion several times
      for (int i = 0; i < iters; i++) {
        visioncpp::execute<visioncpp::policy::Fuse, 32, 32, 16, 16>(exec2, dev);
      }

      // return image to host memory
      visioncpp::execute<visioncpp::policy::Fuse, 32, 32, 16, 16>(exec3, dev);
    }

    // display results
    cv::imshow("Reference Image", input);
    cv::imshow("Simplified Anisotropic Diffusion", outImage);

    // wait for key to finalize program
    if (cv::waitKey(1) >= 0) break;
  }

  // release video/camera
  cap.release();

  return 0;
}
