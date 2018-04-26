#pragma once

/*
  Mesh
  Texture
  RenderTechnique
  Sound
  ...
  RenderToTexture
*/

class IResource;

// A type of resource.
struct CResourceClass {
  std::string class_name;   // Textures
  std::vector< std::string > extensions;    // .dds
  virtual IResource* create(const std::string& name) const = 0;
};

// A forward declaration of a function which should return a const CResourceClass*
// The user must specify which class to use it like: getResourceClassOf<CTexture>()
template<typename MyClass>
const CResourceClass* getResourceClassOf();

// Base class for all resources
class IResource {

protected:
  std::string     name;
  const CResourceClass* resource_class = nullptr;

public:

  virtual ~IResource() { }
  virtual void destroy() { }
  virtual void debugInMenu() { }
  virtual void onFileChanged(const std::string& filename) { }

  const std::string& getName() const { return name; }
  const CResourceClass* getClass() const {
    return resource_class;
  }

  // This method should be called only by the ResourceManager. All other classes
  // should have const IResource pointers which can't call this method
  virtual void setNameAndClass(const std::string& new_name, const CResourceClass* new_class) {
    name = new_name;
    resource_class = new_class;
  }

  template< typename TDerivedResource >
  const TDerivedResource* as() const {
    assert(this != nullptr);
    const CResourceClass* requested_class = getResourceClassOf<TDerivedResource>();

    assert(resource_class == requested_class 
      || 
      fatal( "You are trying to cast a resource of class %s to class %s\n", resource_class->class_name.c_str(), requested_class->class_name.c_str() )
      );
    return static_cast<const TDerivedResource*>(this);
  }

};


