#include <iostream>
#include <thread>

#include "RWLock.h"

int main() {
  RWLock lock;
  int count = 0;
  std::thread b([&] {
    lock.AcquireWLck();
    count = 2;
  });

  std::thread a([&] {
    lock.AcquireRLck();
    std::cout << count << ' ';
  });

  b.join();
  a.join();
  return 0;
}