// Luaponte library

// Copyright (c) 2012 Peter Colberg

// Luaponte is based on Luabind, a library, inspired by and similar to
// Boost.Python, that helps you create bindings between C++ and Lua,
// Copyright (c) 2003-2010 Daniel Wallin and Arvid Norberg.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUAPONTE_DETAIL_CLASS_REP_HPP
#define LUAPONTE_DETAIL_CLASS_REP_HPP

#include <boost/limits.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>

#include <string>
#include <utility>
#include <vector>

#include <luaponte/config.hpp>
#include <luaponte/lua_include.hpp>
#include <luaponte/detail/garbage_collector.hpp>
#include <luaponte/detail/operator_id.hpp>
#include <luaponte/detail/class_registry.hpp>
#include <luaponte/error.hpp>
#include <luaponte/handle.hpp>
#include <luaponte/detail/primitives.hpp>
#include <luaponte/typeid.hpp>
#include <luaponte/detail/ref.hpp>

namespace luaponte {
namespace detail {

    LUAPONTE_API std::string stack_content_by_name(lua_State* L, int start_index);

    struct class_registration;

    struct conversion_storage;

    // This function is used as a tag to identify "properties".
    LUAPONTE_API int property_tag(lua_State*);

    // this is class-specific information, poor man's vtable
    // this is allocated statically (removed by the compiler)
    // a pointer to this structure is stored in the lua tables'
    // metatable with the name __classrep
    // it is used when matching parameters to function calls
    // to determine possible implicit casts
    // it is also used when finding the best match for overloaded
    // methods

    class cast_graph;
    class class_id_map;

    class LUAPONTE_API class_rep
    {
    friend struct class_registration;
    friend int super_callback(lua_State*);
//TODO: avoid the lua-prefix
    friend int lua_class_gettable(lua_State*);
    friend int lua_class_settable(lua_State*);
    friend int static_class_gettable(lua_State*);
    public:

        enum class_type
        {
            cpp_class = 0,
            lua_class = 1
        };

        // EXPECTS THE TOP VALUE ON THE LUA STACK TO
        // BE THE USER DATA WHERE THIS CLASS IS BEING
        // INSTANTIATED!
        class_rep(type_id const& type
            , const char* name
            , lua_State* L
        );

        // used when creating a lua class
        // EXPECTS THE TOP VALUE ON THE LUA STACK TO
        // BE THE USER DATA WHERE THIS CLASS IS BEING
        // INSTANTIATED!
        class_rep(lua_State* L, const char* name);

        ~class_rep();

        std::pair<void*,void*> allocate(lua_State* L) const;

        // this is called as metamethod __call on the class_rep.
        static int constructor_dispatcher(lua_State* L);

        struct base_info
        {
            int pointer_offset; // the offset added to the pointer to obtain a basepointer (due to multiple-inheritance)
            class_rep* base;
        };

        void add_base_class(const base_info& binfo);

        const std::vector<base_info>& bases() const throw() { return m_bases; }

        void set_type(type_id const& t) { m_type = t; }
        type_id const& type() const throw() { return m_type; }

        const char* name() const throw() { return m_name; }

        // the lua reference to the metatable for this class' instances
        int metatable_ref() const throw() { return m_instance_metatable; }

        void get_table(lua_State* L) const { m_table.push(L); }
        void get_default_table(lua_State* L) const { m_default_table.push(L); }

        class_type get_class_type() const { return m_class_type; }

        void add_static_constant(const char* name, int val);

        static int super_callback(lua_State* L);

        static int lua_settable_dispatcher(lua_State* L);

        // called from the metamethod for __index
        // obj is the object pointer
        static int static_class_gettable(lua_State* L);

        bool has_operator_in_lua(lua_State*, int id);

        cast_graph const& casts() const
        {
            return *m_casts;
        }

        class_id_map const& classes() const
        {
            return *m_classes;
        }

    private:

        void cache_operators(lua_State*);

        // this is a pointer to the type_info structure for
        // this type
        // warning: this may be a problem when using dll:s, since
        // typeid() may actually return different pointers for the same
        // type.
        type_id m_type;

        // a list of info for every class this class derives from
        // the information stored here is sufficient to do
        // type casts to the base classes
        std::vector<base_info> m_bases;

        // the class' name (as given when registered to lua with class_)
        const char* m_name;

        // a reference to this structure itself. Since this struct
        // is kept inside lua (to let lua collect it when lua_close()
        // is called) we need to lock it to prevent collection.
        // the actual reference is not currently used.
        detail::lua_reference m_self_ref;

        // this should always be used when accessing
        // members in instances of a class.
        // this table contains c closures for all
        // member functions in this class, they
        // may point to both static and virtual functions
        handle m_table;

        // this table contains default implementations of the
        // virtual functions in m_table.
        handle m_default_table;

        // the type of this class.. determines if it's written in c++ or lua
        class_type m_class_type;

        // this is a lua reference that points to the lua table
        // that is to be used as meta table for all instances
        // of this class.
        int m_instance_metatable;

        std::map<const char*, int, ltstr> m_static_constants;

        // the first time an operator is invoked
        // we check the associated lua table
        // and cache the result
        int m_operator_cache;

        cast_graph* m_casts;
        class_id_map* m_classes;
    };

    bool is_class_rep(lua_State* L, int index);

} // namespace detail
} // namespace luaponte

#endif // LUAPONTE_DETAIL_CLASS_REP_HPP
