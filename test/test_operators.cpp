// Luaponte library

// Copyright (c) 2012 Peter Colberg

// Luaponte is based on Luabind, a library, inspired by and similar to
// Boost.Python, that helps you create bindings between C++ and Lua,
// Copyright (c) 2003-2010 Daniel Wallin and Arvid Norberg.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test.hpp"
#include <luaponte/luaponte.hpp>
#include <luaponte/operator.hpp>
#include <iosfwd>

struct operator_tester : counted_type<operator_tester>
{
    int operator+(int a) const
    {
        return 1 + a;
    }

    int operator-() const
    {
        return 46;
    }

    float operator()() const
    {
        return 3.5f;
    }

    float operator()(int a) const
    {
        return 3.5f + a;
    }

    float operator()(int a)
    {
        return 2.5f + a;
    }
};

float operator*(operator_tester const& lhs, operator_tester const& rhs)
{
    return 35.f;
}

std::string operator*(operator_tester const&, int v)
{
    return "(operator_tester, int) overload";
}

int operator+(int a, const operator_tester&)
{
    return 2 + a;
}

struct operator_tester2 : counted_type<operator_tester2>
{
};

int operator+(const operator_tester&, const operator_tester2&)
{
    return 73;
}

struct operator_tester3: operator_tester, counted_type<operator_tester3> {};

std::ostream& operator<<(std::ostream& os, const operator_tester&)
{
    os << "operator_tester"; return os;
}

struct op_test1
{
    bool operator==(op_test1 const& rhs) const { return true; } 
};

struct op_test2 : public op_test1
{
    bool operator==(op_test2 const& rhs) const { return true; } 
};

COUNTER_GUARD(operator_tester);
COUNTER_GUARD(operator_tester2);
COUNTER_GUARD(operator_tester3);

struct len_tester
{
    len_tester(int len)
      : len_(len)
    {}

    int len() const
    {
        return len_;
    }

    int len_;
};

void test_main(lua_State* L)
{
    using namespace luaponte;

    module(L)
    [
        class_<operator_tester>("operator_tester")
            .def(constructor<>())
            .def(tostring(const_self))
            .def(self + int())
            .def(other<int>() + self)
            .def(-self)
            .def(self + other<operator_tester2&>())
            .def(self())
            .def(const_self(int()))
            .def(self(int()))
//          .def(const_self * other<operator_tester const&>())
            .def(const_self * const_self)
            .def(const_self * other<int>()),
//          .def("clone", &clone, adopt(return_value)),

        class_<operator_tester2>("operator_tester2")
            .def(constructor<>())
            .def(other<const operator_tester&>() + self),

        class_<operator_tester3, operator_tester>("operator_tester3")
            .def(constructor<>()),

        class_<op_test1>("op_test1")
            .def(constructor<>())
            .def(const_self == const_self),

        class_<op_test2, op_test1>("op_test2")
            .def(constructor<>())
            .def(self == self),

        class_<len_tester>("len_tester")
            .def(constructor<int>())
            .def("__len", &len_tester::len)
    ];
    
    DOSTRING(L, "test = operator_tester()");
    DOSTRING(L, "test2 = operator_tester2()");
    DOSTRING(L, "test3 = operator_tester3()");

    DOSTRING(L, "assert(tostring(test) == 'operator_tester')");
    
    DOSTRING(L, "assert(test() == 3.5)");
    DOSTRING(L, "assert(test(5) == 2.5 + 5)");

    DOSTRING(L, "assert(-test == 46)");
    DOSTRING(L, "assert(test * test == 35)");
    DOSTRING(L, "assert(test * 3 == '(operator_tester, int) overload')")
    DOSTRING(L, "assert(test + test2 == 73)");
    DOSTRING(L, "assert(2 + test == 2 + 2)");
    DOSTRING(L, "assert(test + 2 == 1 + 2)");
    DOSTRING(L, "assert(test3 + 6 == 1 + 6)");
    DOSTRING(L, "assert(test3 + test2 == 73)");
//  DOSTRING(L, "assert(tostring(test) == 'operator_tester')");

    const char* prog =
        "class 'my_class'\n"
        "function my_class:__add(lhs)\n"
        "   return my_class(self.val + lhs.val)\n"
        "end\n"
        "function my_class:__init(a)\n"
        "   self.val = a\n"
        "end\n"
        "function my_class:__sub(v)\n"
        "   if (type(self) == 'number') then\n"
        "       return my_class(self - v.val)\n"
        "   elseif (type(v) == 'number') then\n"
        "       return my_class(self.val - v)\n"
        "   else\n"
        "       return my_class(self.val - v.val)\n"
        "   end\n"
        "end\n"
        "a = my_class(3)\n"
        "b = my_class(7)\n"
        "c = a + b\n"
        "d = a - 2\n"
        "d = 10 - d\n"
        "d = d - b\n";

    DOSTRING(L, prog);
    DOSTRING(L, "assert(c.val == 10)");
    DOSTRING(L, "assert(d.val == 2)");

    DOSTRING(L,
        "a = op_test1()\n"
        "b = op_test2()\n"
        "assert(a == b)");

    DOSTRING(L,
        "x = len_tester(0)\n");

    DOSTRING(L,
        "x = len_tester(3)\n"
        "assert(#x == 3)");
}
