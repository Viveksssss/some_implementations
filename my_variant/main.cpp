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

public:
    ~Variant() noexcept
    {
        get_variant_destructors()[m_index](m_union);
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

    Variant(Variant const&) = delete;
    Variant& operator=(Variant const&) = delete;

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
    std::cout << var.get<std::string>() << std::endl;
    Variant<int, double, std::string> var1 = 5.555;
    std::cout << var1.get<double>() << std::endl;
    Variant<int, double, std::string> var2 = 232;
    std::cout << var2.get<int>() << std::endl;

    std::cout << var.index() << "\t" << var1.index() << "\t" << var2.index() << std::endl;
}