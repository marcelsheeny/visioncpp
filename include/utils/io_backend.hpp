
#include <iostream>
#include <string>

/* Implemented interface. */
class AbstractInterface {
 public:
  virtual void someFunctionality() = 0;
};

/* Interface for internal implementation that Bridge uses. */
class IOHandler {
 public:
  virtual void InitMemory() = 0;
  virtual void save() = 0;
  virtual void display() = 0;
};

/* The Bridge */
class Bridge : public AbstractInterface {
 protected:
  IOHandler* implementation;

 public:
  Bridge(IOHandler* backend) { implementation = backend; }
};

/* Different special cases of the interface. */

class UseCase1 : public Bridge {
 public:
  UseCase1(IOHandler* backend) : Bridge(backend) {}

  void someFunctionality() {
    std::cout << "UseCase1 on ";
    implementation->anotherFunctionality();
  }
};

/* Different background implementations. */

class opencv_backend : public IOHandler {
 public:
  void anotherFunctionality() { std::cout << "opencv :-!" << std::endl; }
};

class cimg_backend : public IOHandler {
 public:
  void anotherFunctionality() { std::cout << "cimg! :-)" << std::endl; }
};

int main() {
  AbstractInterface* useCase = 0;
#ifdef USE_CIMG
  ImplementationInterface* backend = new opencv_backend;
#elif USE_OPENCV
  ImplementationInterface* backend = new cimg_backend;
#else
  ImplementationInterface* backend = new opencv_backend;

  /* First case */
  useCase = new UseCase1(backend);
  useCase->someFunctionality();

  return 0;
}
