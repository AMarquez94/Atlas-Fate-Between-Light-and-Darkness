#pragma once

#include <vector>
#include <map>

class CVariant
{
public:
  enum class EType { BOOL = 0, INT, FLOAT, HANDLE, STRING };

  CVariant();

  const std::string& getName() const { return _name; }
  EType getType() const { return _type; }

  bool getBool() const { return _bValue; }
  int getInt() const { return _iValue; }
  float getFloat() const { return _fValue; }
  CHandle getHandle() const;
  const std::string& getString() const {return _sValue; }

  void setName(const std::string& name);
  void setBool(bool value);
  void setInt(int value);
  void setFloat(float value);
  void setHandle(CHandle value);
  void setString(const std::string& value);

private:
  std::string _name;
  EType _type = EType::BOOL;
  union
  {
    bool _bValue;
    int _iValue;
    float _fValue;
    CHandle _hValue;
  };
  std::string _sValue;
};

class MVariants
{
public:
  void addVariant(const std::string& name, bool value);
  void addVariant(const std::string& name, int value);
  void addVariant(const std::string& name, float value);
  void addVariant(const std::string& name, CHandle value);
  void addVariant(const json& jData);

  CVariant* getVariant(const std::string& name) const;

  bool getBool(const std::string& name, bool defaultValue = false) const;
  int getInt(const std::string& name, int defaultValue = 0) const;
  float getFloat(const std::string& name, float defaultValue = 0.f) const;
  CHandle getHandle(const std::string& name, CHandle defaultValue = CHandle()) const;

private:
  std::map<std::string, CVariant> _variants;
};
