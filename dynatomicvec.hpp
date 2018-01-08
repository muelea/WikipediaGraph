#ifndef DYNATOMICVEC_H
#define DYNATOMICVEC_H

#include <cassert>

template <typename T>
class DynAtomicArray {
  std::size_t _n;
  std::atomic<T> * _data;
  public:
   DynAtomicArray(std::size_t n) : _n(n), _data(new std::atomic<T>[n]) {}
    ~DynAtomicArray() {
      if (_data) {
        delete[] _data;
        _data = nullptr;
      }
    }
    DynAtomicArray(DynAtomicArray const&) = delete;
    DynAtomicArray & operator=(DynAtomicArray const&) = delete;
    DynAtomicArray(DynAtomicArray &&) = delete;
    DynAtomicArray & operator=(DynAtomicArray &&) = delete;

    std::atomic<T> * data() noexcept {
      return _data;
    }

    std::size_t size() const noexcept {
      return _n;
    }

    void setTo(int val) noexcept {
     for (std::size_t i = 0; i < _n; ++i)
       _data[i] = val;
    }

   std::atomic<T> & operator[](std::size_t pos) noexcept {
     assert(pos < _n);
     return _data[pos];
   }
};


#endif