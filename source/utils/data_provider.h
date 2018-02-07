#pragma once

// ------------------------------------------
class CDataProvider {

protected:
  std::string name;

public:

  virtual ~CDataProvider() { }
  virtual bool isValid() const = 0;
  virtual const std::string& getName() const {
    return name;
  }
  virtual void readBytes(void *out_buffer, size_t num_bytes) = 0;

  // Read the size of the TPOD
  template< class TPOD>
  void read(TPOD& obj) {
    readBytes(&obj, sizeof(TPOD));
  }
};

// ------------------------------------------
class CFileDataProvider : public CDataProvider {
  FILE* f = nullptr;

public:

  CFileDataProvider(const char* infilename) {
    f = fopen(infilename, "rb");
    name = infilename;
    assert(isValid());
  }

  ~CFileDataProvider() {
    if (f)
      fclose(f);
    f = nullptr;
  }

  bool isValid() const override {
    return f != nullptr;
  }

  void readBytes(void *out_buffer, size_t num_bytes) override {
    auto bytes_read = fread(out_buffer, 1, num_bytes, f);
    assert(bytes_read == num_bytes);
  }


};

