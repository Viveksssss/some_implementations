#include <iostream>
#include <string>
#include <type_traits>

struct BadVariantAccess : std::exception {
    BadVariantAccess() = default;
    virtual ~BadVariantAccess() = default;

    const char* what() const noexcept override
    {
        return "BadVariantAccess";
    }
};

template <std::size_t I>
struct InPlcaeIndex {
    InPlcaeIndex() = default;
};

template <typename, typename>
struct VariantIndex { };
template <typename, size_t>
struct VariantAlternative { };

template <typename... Ts>
struct Variant {
private:
    static constexpr size_t max_size()
    {
        size_t max = 0;
        // 使用折叠表达式（C++17）
        ((max = (sizeof(Ts) > max ? sizeof(Ts) : max)), ...);
        return max;
    }

    alignas(max_size()) char m_union[max_size()];
    std::size_t m_index;

    static void (**get_variant_destructors() noexcept)(char*) noexcept
    {
        static void (*destructors[max_size()])(char*) noexcept = {
            [](char* m_union) noexcept {
                reinterpret_cast<Ts*>(m_union)->~Ts();
            }...
        };
        return destructors;
    }

    static void (**move_constructors() noexcept)(char*, char*) noexcept
    {
        static void (*move_constructors[max_size()])(char*, char*) noexcept = {
            [](char* union_dst, char* union_src) noexcept {
                new (union_dst) Ts(std::move(*reinterpret_cast<Ts*>(union_src)));
            }...
        };
        return move_constructors;
    }
    static void (**move_assignment_constructors() noexcept)(char*, char*) noexcept
    {
        static void (*move_assignment_constructors[max_size()])(char*) noexcept = {
            [](char* union_dst, char* union_src) noexcept {
                *reinterpret_cast<Ts*>(union_dst) = std::move(*reinterpret_cast<Ts*>(union_src));
            }...
        };
        return move_assignment_constructors;
    }

    static void (**copy_constructors() noexcept)(char*, const char*) noexcept
    {
        static void (*copy_constructors[max_size()])(char*, const char*) noexcept = {
            [](char* union_dst, const char* union_src) noexcept {
                new (union_dst) Ts(*reinterpret_cast<Ts const*>(union_src));
            }...
        };
        return copy_constructors;
    }
    static void (**copy_assignment_constructors() noexcept)(char*, const char*) noexcept
    {
        static void (*copy_assignment_constructors[max_size()])(char*, const char*) noexcept = {
            [](char* union_dst, const char* union_src) noexcept {
                *reinterpret_cast<Ts*>(union_dst) = *reinterpret_cast<Ts const*>(union_src);
            }...
        };
        return copy_assignment_constructors;
    }

    template <typename Lambda>
    static typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type (**visitable_c() noexcept)(const char*, Lambda) noexcept
    {
        static void (*visitable[max_size()])(const char*, Lambda) noexcept = {
            [](const char* m_union, Lambda lambda) noexcept -> typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type {
                return lambda(*reinterpret_cast<const Ts*>(m_union));
            }...
        };
        return visitable;
    }

    template <typename Lambda>
    static typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type (**visitable() noexcept)(char*, Lambda) noexcept
    {
        using visit_return_type = typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type;

        static visit_return_type (*visitable[max_size()])(char*, Lambda) noexcept = {
            [](char* m_union, Lambda lambda) noexcept -> visit_return_type {
                return lambda(*reinterpret_cast<Ts*>(m_union));
            }...
        };
        return visitable;
    }

public:
    ~Variant() noexcept
    {
        get_variant_destructors()[m_index](m_union);
    }

    Variant(Variant&& other)
        : m_index(other.m_index)
    {
        move_constructors()[m_index](m_union, other.m_union);
    }
    Variant& operator=(Variant&& other)

    {
        m_index = other.m_index;
        move_assignment_constructors()[m_index](m_union, other.m_union);
    }

    Variant(Variant const& other)
        : m_index(other.m_index)
    {
        copy_constructors()[m_index](m_union, other.m_union);
    }

    Variant& operator=(Variant const& other)
    {
        m_index = other.m_index;
        copy_assignment_constructors()[index()](m_union, other.m_union);
    }

    template <std::size_t I, typename... Args>
    Variant(InPlcaeIndex<I>, Args&&... value_types)
        : m_index(I)
    {
        new (m_union) typename VariantAlternative<Variant, I>::type(std::forward<Args>(value_types)...);
    }

#if __cplusplus >= 202002L
    template <typename T>
        requires(std::is_same_v<T, Ts> || ...)
    Variant(T value)
        : m_index(VariantIndex<Variant, T>::value)
    {
        T* p = reinterpret_cast<T*>(m_union);
        new (p) T(value);
    }
#else
    template <typename T, typename std::enable_if<std::disjunction<std::is_same<T, Ts>...>::value, int>::value = 0>
    Variant(T value)
        : m_index(VariantIndex<Variant, T>::value)
    {
        T* p = reinterpret_cast<T*>(m_union);
        new (p) T(value);
    }
#endif

    constexpr size_t index() const
    {
        return m_index;
    }

    template <typename T>
    constexpr bool holds_alternative() const
    {
        return VariantIndex<Variant, T>::value == m_index;
    }
    template <std::size_t I>
    typename VariantAlternative<Variant, I>::type const& get() const
    {
        static_assert(I < sizeof...(Ts), "out of range");

        if (m_index != I) {
            throw BadVariantAccess();
        }

        using _type = typename VariantAlternative<Variant, I>::type;
        return *reinterpret_cast<_type const*>(m_union);
    }

    template <typename T>
    T const& get() const
    {
        return get<VariantIndex<Variant, T>::value>();
    }

    //
    template <class Lambda>
    typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type visit(Lambda lambda) const
    {
        return visitable_c<Lambda>()[m_index]((m_union), lambda);
    }

    template <class Lambda>
    typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type visit(Lambda lambda)
    {
        using visit_return_type = typename std::common_type<typename std::invoke_result<Lambda, Ts&>::type...>::type;
        return visitable<Lambda>()[m_index]((m_union), lambda);
    }

    template <std::size_t I>
    auto get_if() const -> typename VariantAlternative<Variant, I>::type const*
    {
        static_assert(I < sizeof...(Ts), "I out of range!");
        if (m_index != I) {
            return nullptr;
        }
        return reinterpret_cast<typename VariantAlternative<Variant, I>::type const*>(m_union);
    }

    template <std::size_t I>
    auto get_if() -> typename VariantAlternative<Variant, I>::type*
    {
        static_assert(I < sizeof...(Ts), "I out of range!");
        if (m_index != I) {
            return nullptr;
        }
        return reinterpret_cast<typename VariantAlternative<Variant, I>::type*>(m_union);
    }
};

template <typename T, typename... Ts>
struct VariantIndex<Variant<T, Ts...>, T> {
    static constexpr size_t value = 0;
};

template <typename T0, typename T, typename... Ts>
struct VariantIndex<Variant<T0, Ts...>, T> {
    static constexpr size_t value = VariantIndex<Variant<Ts...>, T>::value + 1;
};

template <typename T, typename... Ts>
struct VariantAlternative<Variant<T, Ts...>, 0> {
    using type = T;
};

template <typename T, typename... Ts, size_t I>
struct VariantAlternative<Variant<T, Ts...>, I> {
    using type = typename VariantAlternative<Variant<Ts...>, I - 1>::type;
};

int main()
{
    Variant<int, double, std::string> var = std::string("SAdsad");
    auto p = var.visit([](auto a) {
        std::cout << a << std::endl;
        return "asd";
    });
    std::cout << p << std::endl;
}