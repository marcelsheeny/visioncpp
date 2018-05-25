#include <cstdio>
#include <memory>
#include <string>

#include <dirent.h>

#include <fcntl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <ctime>
#include <iostream>

#ifdef USE_CIMG
#include "cimg_backend.hpp"
#else
#include "opencv_backend.hpp"
#endif

using namespace visioncpp::utils;

namespace visioncpp {
namespace utils {

/**
 * IOHandler class handles the input output in a agnostic way.
 * Currently using OpenCV and CImg to load/display images
 */
template <int COLS, int ROWS, int CHANNELS, typename T>
class IOHandler {
 public:
  IOHandler() : backend() {}

  /* Constructor receives the filename path, which initialize input and output
   * memories */
  IOHandler(const char *filename) : backend(filename) {}

  void imread(const char *filename) { backend.imread(filename); }

  void saveOutput(char *output_file) { backend.save_image_output(output_file); }

  void displayOutput(char *str, const int &time) {
    backend.display_image_output(str, time);
  }

  void displayInput(char *str, const int &time) {
    backend.display_image_input(str, time);
  }

  void displayOutput(char *str) { backend.display_image_output(str); }

  void displayInput(char *str) { backend.display_image_input(str); }

  void saveInput(char *output_file) { backend.save_image_input(output_file); }

  void videoCapture(const char *str) { backend.videoCapture(str); }

  int nextFrame() { return backend.nextFrame(); }

  T *getInputPointer() { return backend.getInputPointer(); }

  T *getOutputPointer() { return backend.getOutputPointer(); }

  void displayGreyscaleImageOnFrameBuffer(int pos_x, int pos_y) {
    int width = COLS;
    int height = ROWS;
    int image[width * height];  // first number here is pixels in my image, 3 is
                                // for RGB values

    // printf("width %d, height %d \n", width, height);
    //      cout << width << "x" << height << endl;
    //	printf("start the display array filling\n");
    int i = 0;
    for (int r = height; r > 0; r--) {
      // printf("ROW %d\n", r);
      for (int c = 0; c < width; c++) {
        // printf("R:%u G:%u B:%u, r:%d,
        // c:%d\n",inputimage(c,r,0,2),inputimage(c,r,0,1),inputimage(c,r,0,0),r,c);
        // image[i] = inputimage(c,r,0,0);  // R
        image[i] = backend.getInputPointer()[r * COLS + c];
        i++;
      }
    }
    //	printf("created the array of pixels \n");
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;
    //	printf("open the frame buffer \n");
    // Open the framebuffer for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
      perror("Error: cannot open framebuffer device");
      exit(1);
    }
    // printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
      perror("Error reading fixed information");
      exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
      perror("Error reading variable information");
      exit(3);
    }

    // printf("%dx%d, %dbpp\n", vinfo.xoffset, vinfo.yoffset,
    // vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd,
                       0);
    //    if ((int)fbp == -1) {
    //        perror("Error: failed to map framebuffer device to memory");
    //        exit(4);
    //    }
    //    printf("The framebuffer device was mapped to memory successfully.\n");

    int countpixel = 0;

    for (y = pos_y + height; y > pos_y; y--)
      for (x = pos_x; x < pos_x + width; x++) {
        location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                   (y + vinfo.yoffset) * finfo.line_length;
        *(fbp + location) =
            image[countpixel];  //*(fbp + location) = image[countpixel][2];
        *(fbp + location + 1) =
            image[countpixel];  //*(fbp + location + 1) = image[countpixel][1];
        *(fbp + location + 2) =
            image[countpixel];  //*(fbp + location + 2) = image[countpixel][0];
        //*(fbp + location + 3) = 0;
        countpixel++;
      }
    munmap(fbp, screensize);
    close(fbfd);
  }

  void displayImageOnFrameBuffer(int pos_x, int pos_y) {
    int width = COLS;
    int height = ROWS;
    int image[width * height][3];  // first number here is pixels in my image, 3
                                   // is for RGB values

    //	printf("width %d, height %d \n", width, height);
    //      cout << width << "x" << height << endl;
    //	printf("start the display array filling\n");
    int i = 0;
    for (int r = height; r > 0; r--) {
      // printf("ROW %d\n", r);
      for (int c = 0; c < width; c++) {
        // printf("R:%u G:%u B:%u, r:%d,
        // c:%d\n",inputimage(c,r,0,2),inputimage(c,r,0,1),inputimage(c,r,0,0),r,c);
        //        image[i][2] = inputimage(c,r,0,2);  // B
        //        image[i][1] = inputimage(c,r,0,1);  // G
        //        image[i][0] = inputimage(c,r,0,0);  // R

        image[i][2] = backend.getInputPointer()[r * COLS + c + 2];
        image[i][2] = backend.getInputPointer()[r * COLS + c + 1];
        image[i][2] = backend.getInputPointer()[r * COLS + c + 0];
        i++;
      }
    }
    //	printf("created the array of pixels \n");
    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;
    //	printf("open the frame buffer \n");
    // Open the framebuffer for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
      perror("Error: cannot open framebuffer device");
      exit(1);
    }
    // printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
      perror("Error reading fixed information");
      exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
      perror("Error reading variable information");
      exit(3);
    }

    //	printf("%dx%d, %dbpp\n", vinfo.xoffset, vinfo.yoffset,
    // vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd,
                       0);
    //    if ((int)fbp == -1) {
    //        perror("Error: failed to map framebuffer device to memory");
    //        exit(4);
    //    }
    //    printf("The framebuffer device was mapped to memory successfully.\n");

    int countpixel = 0;

    for (y = pos_y + height; y > pos_y; y--)
      for (x = pos_x; x < pos_x + width; x++) {
        location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                   (y + vinfo.yoffset) * finfo.line_length;
        *(fbp + location) =
            image[countpixel][0];  //*(fbp + location) = image[countpixel][2];
        *(fbp + location + 1) =
            image[countpixel]
                 [1];  //*(fbp + location + 1) = image[countpixel][1];
        *(fbp + location + 2) =
            image[countpixel]
                 [2];  //*(fbp + location + 2) = image[countpixel][0];
        *(fbp + location + 3) = 0;
        countpixel++;
      }
    munmap(fbp, screensize);
    close(fbfd);
  }

  void showImageOnFrameBuffer(char *filepath, int pos_x, int pos_y) {
    int image[230400][3];  // first number here is 236160 pixels in my image, 3
                           // is for RGB values
    FILE *streamIn;
    streamIn = fopen(filepath, "r");
    if (streamIn == (FILE *)0) {
      printf("File opening error ocurred. Exiting program.\n");
      exit(0);
    }

    int byte;
    int count = 0;
    for (int i = 0; i < 54; i++) byte = getc(streamIn);  // strip out BMP header

    for (int i = 0; i < 230400; i++) {  // foreach pixel
      image[i][2] = getc(streamIn);     // use BMP 24bit with no alpha channel
      image[i][1] =
          getc(streamIn);  // BMP uses BGR but we want RGB, grab byte-by-byte
      image[i][0] =
          getc(streamIn);  // reverse-order array indexing fixes RGB issue...
      // printf("pixel %d :
      // [%d,%d,%d]\n",i+1,image[i][0],image[i][1],image[i][2]);
    }
    fclose(streamIn);

    int fbfd = 0;
    struct fb_var_screeninfo vinfo;
    struct fb_fix_screeninfo finfo;
    long int screensize = 0;
    char *fbp = 0;
    int x = 0, y = 0;
    long int location = 0;

    // Open the framebuffer for reading and writing
    fbfd = open("/dev/fb0", O_RDWR);
    if (fbfd == -1) {
      perror("Error: cannot open framebuffer device");
      exit(1);
    }
    // printf("The framebuffer device was opened successfully.\n");

    // Get fixed screen information
    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
      perror("Error reading fixed information");
      exit(2);
    }

    // Get variable screen information
    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
      perror("Error reading variable information");
      exit(3);
    }

    // printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

    // Figure out the size of the screen in bytes
    screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

    // Map the device to memory
    fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd,
                       0);
    //    if ((int)fbp == -1) {
    //        perror("Error: failed to map framebuffer device to memory");
    //        exit(4);
    //    }
    // printf("The framebuffer device was mapped to memory successfully.\n");

    int countpixel = 0;

    for (y = pos_y + 360; y > pos_y; y--)
      for (x = pos_x; x < pos_x + 640; x++) {
        location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
                   (y + vinfo.yoffset) * finfo.line_length;
        *(fbp + location) = image[countpixel][2];
        *(fbp + location + 1) = image[countpixel][1];
        *(fbp + location + 2) = image[countpixel][0];
        *(fbp + location + 3) = 0;
        countpixel++;
      }
    munmap(fbp, screensize);
    close(fbfd);
  }

 private:
#ifdef USE_CIMG
  typedef cimg<COLS, ROWS, CHANNELS, T> IOBackend;
#else
  typedef opencv<COLS, ROWS, CHANNELS, T> IOBackend;
#endif
  IOBackend backend;
};

}  // namespace utils
}  // namespace visioncpp
