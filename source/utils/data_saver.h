#ifndef INC_DATA_SAVER_H_
#define INC_DATA_SAVER_H_

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <cassert>

class CDataSaver {
public:
  virtual bool isValid() const = 0;
  virtual void writeBytes(const void* addr, uint32_t num_bytes) = 0;
  virtual const char* getName() const = 0;

  template< typename POD >
  void write(const POD& pod) {
    writeBytes(&pod, sizeof(POD));
  }
};

class CFileDataSaver : public CDataSaver {
  FILE* f = nullptr;
  std::string my_filename;
public:
  CFileDataSaver(const char* filename) {
    my_filename = filename;
    f = fopen(filename, "wb");
  }
  ~CFileDataSaver() {
    if (f)
      fclose(f), f = nullptr;
  }
  bool isValid() const override {
    return f != nullptr;
  }
  void writeBytes(const void* addr, uint32_t num_bytes_to_write) override {
    auto num_bytes_written = fwrite(addr, 1, num_bytes_to_write, f);
    assert(num_bytes_written == num_bytes_to_write);
  }
  const char* getName() const override {
    return my_filename.c_str();
  }

};


class CMemoryDataSaver : public CDataSaver {
public:
  std::vector< uint8_t > buffer;
  bool isValid() const override {
    return true;
  }
  void writeBytes(const void* addr, uint32_t num_bytes_to_write) override {
    size_t old_size = buffer.size();
    buffer.resize(old_size + num_bytes_to_write);
    memcpy(buffer.data() + old_size, addr, num_bytes_to_write);
  }
  const char* getName() const override {
    return "buffer";
  }

};

#endif

