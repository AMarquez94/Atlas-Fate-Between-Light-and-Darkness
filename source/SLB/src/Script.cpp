/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2010 Jose L. Hidalgo Valiño (PpluX)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  Jose L. Hidalgo (www.pplux.com)
  pplux@pplux.com
*/



#include<SLB/Script.hpp>
#include<SLB/Debug.hpp>
#include<SLB/Error.hpp>
#include<SLB/Allocator.hpp>
#include<sstream>

namespace SLB {

#if SLB_DEBUG_LEVEL != 0
  /* Debugging function, see TODO on Script::getState */
  void ScriptHook(lua_State *L, lua_Debug *ar)
  {
    lua_getinfo(L, "Sl",ar);
    __SLB_ADJUST__();
    SLB_DEBUG_FUNC("SLB-X[%p] %s:%d",L,ar->short_src,ar->currentline );
    SLB_DEBUG_FUNC("\n");\
  }
#endif

  Script::Script(Manager *m, bool default_libs) : 
    _manager(m), 
    _L(0),
    _allocator(&Script::allocator),
    _allocator_ud(0),
    _errorHandler(0),
    _loadDefaultLibs(default_libs)
  {
    SLB_DEBUG_CALL;
    
    DefaultErrorHandler *err = 0;
    New_T(&err);
    _errorHandler = err;
  }

  Script::~Script()
  {
    SLB_DEBUG_CALL;
    Free_T(&_errorHandler);
    close();
  }

  void Script::setAllocator(lua_Alloc f, void *ud)
  {
    _allocator = f;
    _allocator_ud = ud;
  }
  
  lua_State* Script::getState()
  {
    SLB_DEBUG_CALL;
    if (!_L)
    {
      SLB_DEBUG(10, "Open default libs = %s", _loadDefaultLibs ? " true": " false");
      _L = lua_newstate(_allocator, _allocator_ud);
      assert("Can not create more lua_states" && (_L != 0L));
      if (_loadDefaultLibs) luaL_openlibs(_L);
      _manager->registerSLB(_L);
    
      //TODO: Promote that functionality to a higher interface to allow proper
      //      debugging
      //
      /* if debug_level > 0 ........ */
      #if SLB_DEBUG_LEVEL != 0
      lua_sethook(_L, ScriptHook, LUA_MASKLINE, 0);
      #endif
      /* end debug */

      onNewState(_L);
    }
    return _L;
  }

  void Script::close()
  {
    SLB_DEBUG_CALL;
    if (_L)
    {
      onCloseState(_L);
      lua_close(_L);
      _L = 0;
    }
  }

  void Script::callGC()
  {
    SLB_DEBUG_CALL;
    if (_L)
    {
      onGC(_L);
      lua_gc(_L, LUA_GCCOLLECT, 0);
    }
  }
  
  size_t Script::memUsage()
  {
    SLB_DEBUG_CALL;
    size_t result  = 0;
    if (_L)
    {
      int r = lua_gc(_L, LUA_GCCOUNT, 0);
      result = r;
    }
    return result;
  }

  void Script::doFile(const std::string &filename) throw (std::exception)
  {
    SLB_DEBUG_CALL;
    lua_State *L = getState();
    int top = lua_gettop(L);
    SLB_DEBUG(10, "filename %s = ", filename.c_str());

    switch(luaL_loadfile(L,filename.c_str()))
    {
      case LUA_ERRFILE:
      case LUA_ERRSYNTAX: throw std::runtime_error(lua_tostring(_L, -1)); break;
      case LUA_ERRMEM:    throw std::runtime_error("Error allocating memory"); break;
    }

    // otherwise...
    if( _errorHandler->call(_L, 0, 0))
    {
      const char *s = lua_tostring(L,-1);
      throw std::runtime_error( s );
    }

    lua_settop(L,top);
  }

  void Script::doString(const std::string &o_code, const std::string &hint) throw (std::exception)
  {
    SLB_DEBUG_CALL;
    lua_State *L = getState();
    int top = lua_gettop(L);
    SLB_DEBUG(10, "code = %10s, hint = %s", o_code.c_str(), hint.c_str()); 
    std::stringstream code;
    code << "--" << hint << std::endl << o_code;

    if(luaL_loadstring(L,code.str().c_str()) || _errorHandler->call(_L, 0, 0))
    {
      const char *s = lua_tostring(L,-1);
      throw std::runtime_error( s );
    }

    lua_settop(L,top);
  }

  void Script::setErrorHandler( ErrorHandler *e )
  {
    Free_T(&_errorHandler);
    _errorHandler = e;
  }

  // Added by MCV
  bool Script::exists(const char *name) {
    lua_State* L = getState();
    lua_getglobal(L, name);
    bool rc = lua_isfunction(L, lua_gettop(L));
    // Confirm if this is required
    lua_pop(L, 1);
    return rc;
  }

  void *Script::allocator(void * /*ud*/, void *ptr, size_t osize, size_t nsize)
  {
    if (nsize == 0)
    {
      SLB::Free(ptr);
      return 0;
    }
    else
    {
      void *newpos = SLB::Malloc(nsize);
      size_t count = osize;
      if (nsize < osize) count = nsize;
      if (ptr) memcpy(newpos, ptr, count);
      SLB::Free(ptr);
      return newpos;
    }
  }

} /* SLB */
