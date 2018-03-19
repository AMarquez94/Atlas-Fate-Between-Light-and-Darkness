#include "mcv_platform.h"
#include "variant.h"

CVariant::CVariant()
: _type(EType::BOOL)
, _bValue(false)
{
}

void CVariant::setName(const std::string& name)
{
  _name = name;
}

void CVariant::setBool(bool value) {
  _bValue = value; 
  _type = EType::BOOL;
}

void CVariant::setInt(int value) {
  _iValue = value;
  _type = EType::INT;
}

void CVariant::setFloat(float value) {
  _fValue = value;
  _type = EType::FLOAT;
}

void CVariant::setHandle(CHandle value) {
  _hValue = value;
  _type = EType::HANDLE;
}

void CVariant::setString(const std::string& value)
{
  _sValue = value;
  _type = EType::STRING;
}

CHandle CVariant::getHandle() const
{
  if (_type == EType::STRING)
  {
    return getEntityByName(_sValue);
  }
  else
  {
    return _hValue;
  }
}

void CVariant::load(const json& jData)
{
  const std::string& name = jData["name"];
  const std::string& type = jData["type"];

  setName(name);
  if (type == "bool")
  {
    setBool(jData.value("value", false));
  }
  else if (type == "int")
  {
    setInt(jData.value("value", 0));
  }
  else if (type == "float")
  {
    setFloat(jData.value("value", 0.f));
  }
  else if (type == "handle" || type == "string")
  {
    setString(jData.value("value", ""));
  }
}

std::string CVariant::toString() const
{
  switch (_type)
  {
  case EType::BOOL: return _bValue ? "true" : "false";
  case EType::INT: return std::to_string(_iValue);
  case EType::FLOAT: return std::to_string(_fValue);
  case EType::HANDLE: {
    CEntity* entity = _hValue;
    if (entity)
    {
      return entity->getName();
    }
    else {
      return _hValue.isValid() ? "valid" : "...";
    }
  }
  case EType::STRING: return _sValue;
  default: return "";
  }
}

void MVariants::clear()
{
  _variants.clear();
}

void MVariants::setVariant(const std::string& name, bool value)
{
  _variants[name].setBool(value);
}

void MVariants::setVariant(const std::string& name, int value)
{
  _variants[name].setInt(value);
}

void MVariants::setVariant(const std::string& name, float value)
{
  _variants[name].setFloat(value);
}

void MVariants::setVariant(const std::string& name, CHandle value)
{
  _variants[name].setHandle(value);
}

void MVariants::setVariant(const std::string& name, const std::string& value)
{
  _variants[name].setString(value);
}

void MVariants::setVariant(const std::string& name, const CVariant& value)
{
  _variants[name] = value;
}

void MVariants::setVariant(const json& jData)
{
  CVariant var;
  var.load(jData);
  _variants[var.getName()] = var;
}

CVariant* MVariants::getVariant(const std::string& name)
{
  return const_cast<CVariant*>(const_cast<const MVariants*>(this)->getVariant(name));
}

const CVariant* MVariants::getVariant(const std::string& name) const
{
  for (auto& var : _variants)
  {
    if (var.first == name)
    {
      return &var.second;
    }
  }
  return nullptr;
}

bool MVariants::getBool(const std::string& name, bool defaultValue) const
{
  const CVariant* var = getVariant(name);
  if (var)
  {
    return var->getBool();
  }
  return defaultValue;
}

int MVariants::getInt(const std::string& name, int defaultValue) const
{
  const CVariant* var = getVariant(name);
  if (var)
  {
    return var->getInt();
  }
  return defaultValue;
}

float MVariants::getFloat(const std::string& name, float defaultValue) const
{
  const CVariant* var = getVariant(name);
  if (var)
  {
    return var->getFloat();
  }
  return defaultValue;
}

CHandle MVariants::getHandle(const std::string& name, CHandle defaultValue) const
{
  const CVariant* var = getVariant(name);
  if (var)
  {
    return var->getHandle();
  }
  return defaultValue;
}
