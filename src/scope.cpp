// Luaponte library

// Copyright (c) 2011-2012 Peter Colberg

// Luaponte is based on Luabind, a library, inspired by and similar to
// Boost.Python, that helps you create bindings between C++ and Lua,
// Copyright (c) 2003-2010 Daniel Wallin and Arvid Norberg.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define LUAPONTE_BUILDING

#include <luaponte/lua_include.hpp>

#include <luaponte/scope.hpp>
#include <luaponte/detail/debug.hpp>
#include <luaponte/detail/stack_utils.hpp>
#include <cassert>

#if LUA_VERSION_NUM < 502
# define lua_pushglobaltable(L) lua_pushvalue(L, LUA_GLOBALSINDEX)
#endif

namespace luaponte {
namespace detail {

registration::registration()
    : m_next(0)
{
}

registration::~registration()
{
    delete m_next;
}

} // namespace detail

scope::scope()
    : m_chain(0)
{
}

scope::scope(std::auto_ptr<detail::registration> reg)
    : m_chain(reg.release())
{
}

scope::scope(scope const& other)
    : m_chain(other.m_chain)
{
    const_cast<scope&>(other).m_chain = 0;
}

scope& scope::operator=(scope const& other_)
{
    delete m_chain;
    m_chain = other_.m_chain;
    const_cast<scope&>(other_).m_chain = 0;
    return *this;
}

scope::~scope()
{
    delete m_chain;
}

scope& scope::operator,(scope s)
{
    if (!m_chain)
    {
        m_chain = s.m_chain;
        s.m_chain = 0;
        return *this;
    }

    for (detail::registration* c = m_chain;; c = c->m_next)
    {
        if (!c->m_next)
        {
            c->m_next = s.m_chain;
            s.m_chain = 0;
            break;
        }
    }

    return *this;
}

void scope::register_(lua_State* L) const
{
    for (detail::registration* r = m_chain; r != 0; r = r->m_next)
    {
        LUAPONTE_CHECK_STACK(L);
        r->register_(L);
    }
}

namespace {

struct lua_pop_stack
{
    lua_pop_stack(lua_State* L)
        : m_state(L)
    {
    }

    ~lua_pop_stack()
    {
        lua_pop(m_state, 1);
    }

    lua_State* m_state;
};

} // namespace

module_::module_(lua_State* L, char const* name = 0)
    : m_state(L)
    , m_name(name)
{
}

void module_::operator[](scope s)
{
    if (m_name)
    {
        lua_getglobal(m_state, m_name);

        if (!lua_istable(m_state, -1))
        {
            lua_pop(m_state, 1);

            lua_newtable(m_state);
            lua_pushvalue(m_state, -1);
            lua_setglobal(m_state, m_name);
        }
    }
    else
    {
        lua_pushglobaltable(m_state);
    }

    lua_pop_stack guard(m_state);

    s.register_(m_state);
}

struct namespace_::registration_ : detail::registration
{
    registration_(char const* name)
        : m_name(name)
    {
    }

    void register_(lua_State* L) const
    {
        LUAPONTE_CHECK_STACK(L);
        assert(lua_gettop(L) >= 1);

        lua_pushstring(L, m_name);
        lua_gettable(L, -2);

        detail::stack_pop p(L, 1); // pops the table on exit

        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);

            lua_newtable(L);
            lua_pushstring(L, m_name);
            lua_pushvalue(L, -2);
            lua_settable(L, -4);
        }

        m_scope.register_(L);
    }

    char const* m_name;
    scope m_scope;
};

namespace_::namespace_(char const* name)
    : scope(std::auto_ptr<detail::registration>(
          m_registration = new registration_(name)))
{
}

namespace_& namespace_::operator[](scope s)
{
    m_registration->m_scope.operator,(s);
    return *this;
}

} // namespace luaponte
