#pragma once

namespace reflect {

template <typename T>
struct reflect_traits { };

#define REFLECT__PP_FOREACH_1(f, _1) f(_1)
#define REFLECT__PP_FOREACH_2(f, _1, _2) f(_1) f(_2)
#define REFLECT__PP_FOREACH_3(f, _1, _2, _3) f(_1) f(_2) f(_3)
#define REFLECT__PP_FOREACH_4(f, _1, _2, _3, _4) f(_1) f(_2) f(_3) f(_4)
#define REFLECT__PP_FOREACH_5(f, _1, _2, _3, _4, _5)                           \
    f(_1) f(_2) f(_3) f(_4) f(_5)
#define REFLECT__PP_FOREACH_6(f, _1, _2, _3, _4, _5, _6)                       \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6)
#define REFLECT__PP_FOREACH_7(f, _1, _2, _3, _4, _5, _6, _7)                   \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7)
#define REFLECT__PP_FOREACH_8(f, _1, _2, _3, _4, _5, _6, _7, _8)               \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8)
#define REFLECT__PP_FOREACH_9(f, _1, _2, _3, _4, _5, _6, _7, _8, _9)           \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9)
#define REFLECT__PP_FOREACH_10(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)     \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10)
#define REFLECT__PP_FOREACH_11(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)                           \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)
#define REFLECT__PP_FOREACH_12(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)                      \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12)
#define REFLECT__PP_FOREACH_13(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)                 \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13)
#define REFLECT__PP_FOREACH_14(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)            \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14)
#define REFLECT__PP_FOREACH_15(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)       \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15)
#define REFLECT__PP_FOREACH_16(                                                \
    f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)  \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15) f(_16)
#define REFLECT__PP_FOREACH_17(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
    _11, _12, _13, _14, _15, _16, _17)                                         \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15) f(_16) f(_17)
#define REFLECT__PP_FOREACH_18(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
    _11, _12, _13, _14, _15, _16, _17, _18)                                    \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18)
#define REFLECT__PP_FOREACH_19(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
    _11, _12, _13, _14, _15, _16, _17, _18, _19)                               \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19)
#define REFLECT__PP_FOREACH_20(f, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,     \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20)                          \
    \f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)       \
        f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20)

/*
    当有5个参数的时候，REFLECT__PP_NARGS（n1,n2...n5）->
   REFLECT__PP_NARGS_IMPL(n1,n2...n5,20,19,18,17...1)
    对于REFLECT__PP_NARGS_IMPL，如果没有参数，那么前二十个参数正好对抵了20,19,18,17...1，N为空
    但如果__VA_ARGS__有5个，那么传入REFLECT__PP_NARGS_IMPL的那20,19,18...1就会被顶到右侧5个单位
    那么这样的话，5就正好顶到了N的位置，我们就能得知参数的个数为5.
    知道参数的话，我们根据共同的宏前缀REFLECT__PP_FOREACH_再连接个数，就是对应个数参数的宏处理函数
    下面之所以分为CONCAT_1
   CONCAT_2多次连接，是因为##会阻止宏展开，多次嵌套可以先展开参数最后连接

    // 错误：一次CONCAT
    #define CONCAT(x, y) x##y
    #define CONCAT_TEST CONCAT(REF_, FLECT)
    一开始CONCAT(REF,FLECT)的时候，REF和FLECT并没有展开，而是直接被##连接在一起

    // 正确：两次CONCAT
    #define CONCAT_2(x, y) x##y
    #define CONCAT(x, y) CONCAT_2(x, y)
    #define CONCAT_TEST CONCAT(REF_, FLECT)
    CONCAT(REF_, FLECT) -> CONCAT_2(REF_, FLECT) -> REF_FLECT
*/
#define REFLECT__PP_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11,   \
    _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...)                       \
    N

#define REFLECT__PP_NARGS(...)                                                 \
    REFLECT__PP_NARGS_IMPL(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12,    \
        11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define REFLECT__PP_CONCAT_2(x, y) x##y
#define REFLECT__PP_CONCAT(x, y) REFLECT__PP_CONCAT_2(x, y)
#define REFLECT__PP_EXPAND_2(...) __VA_ARGS__
#define REFLECT__PP_EXPAND(...) REFLECT__PP_EXPAND_2(__VA_ARGS__)
#define REFLECT__PP_FOREACH(f, ...)                                            \
    REFLECT__PP_EXPAND(REFLECT__PP_CONCAT(                                     \
        REFLECT__PP_FOREACH_, REFLECT__PP_NARGS(__VA_ARGS__))(f, __VA_ARGS__))

#define REFLECT__PER_MEMBER_PTR(x) func(#x, &This::x);
#define REFLECT__TYPE_PER_MEMBER_PTR(x) func(#x, &This::x);

#define REFLECT_TYPE(Type, ...)                                                \
    template <>                                                                \
    struct reflect::reflect_traits<Type> {                                     \
        using This = Type;                                                     \
        static constexpr bool has_member() {                                   \
            return true;                                                       \
        }                                                                      \
        template <typename Func>                                               \
        static constexpr void foreach_members_ptr(Func &&func) {               \
            REFLECT__PP_FOREACH(REFLECT__TYPE_PER_MEMBER_PTR, __VA_ARGS__)     \
        }                                                                      \
    };

#define REFLECT(...)                                                           \
    template <class This, class Func>                                          \
    static constexpr void foreach_members_ptr(Func &&func) {                   \
        REFLECT__PP_FOREACH(REFLECT__PER_MEMBER_PTR, __VA_ARGS__)              \
    };

}; // namespace reflect
