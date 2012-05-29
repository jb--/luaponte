// Luaponte library

// Copyright (c) 2012 Peter Colberg

// Luaponte is based on Luabind, a library, inspired by and similar to
// Boost.Python, that helps you create bindings between C++ and Lua,
// Copyright (c) 2003-2010 Daniel Wallin and Arvid Norberg.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUAPONTE_OPEN_HPP
#define LUAPONTE_OPEN_HPP

#include <luaponte/config.hpp>

namespace luaponte {

LUAPONTE_API void open(lua_State* L);

} // namespace luaponte

#endif // LUAPONTE_OPEN_HPP
