/*
    SLB - Simple Lua Binder
    Copyright (C) 2007-2011 Jose L. Hidalgo Valiño (PpluX)

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



#ifndef __SLB_LUA__
#define __SLB_LUA__

#include "Export.hpp"

extern "C" {
#ifdef SLB_EXTERNAL_LUA
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
#else
  #if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
    #ifndef SLB_STATIC_LIBRARY
      #define LUA_BUILD_AS_DLL
      #ifdef SLB_LIBRARY
        #define LUA_LIB
        #define LUA_CORE
      #endif
    #endif // SLB_STATIC_LIBRARY
  #endif // on windows...

  // Local (static) lua (v 5.2.0-beta-rc3)
  #include "lua/lua.h"
  #include "lua/lauxlib.h"
  #include "lua/lualib.h"

#endif
}

#endif
