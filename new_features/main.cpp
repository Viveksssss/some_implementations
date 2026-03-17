#include <chrono>
#include <cstdio>
#include <iostream>
#include <list>
#include <memory_resource>
#include <utility>

/* 变参 -17 */
template <typename... Ts>
auto func_add(Ts... ts)
{
    return (0 + ... + ts);
}

template <typename... Ts>
auto func_print_1(Ts... ts)
{
    ((std::cout << ts << " "), ...) << std::endl;
}

template <typename T0, typename... Ts>
auto func_print_2(T0 t, Ts... ts)
{
    std::cout << t << "";
    ((std::cout << "," << ts), ...) << std::endl;
}

auto func_args()
{
}
template <typename T0, typename... Args>
auto func_args(T0 t, Args... args)
{
    std::cout << t << " ";
    func_args(args...);
}

template <typename... Args>
void f(Args... args)
{
    int a[] = { args... };
    for (int i = 0; i < sizeof(a) / sizeof(a[0]); ++i) {
        std::cout << a[i] << " ";
    }
}

/* common_type */
struct animal { };

struct cat : animal { };
struct dog : animal { };

template <typename T1, typename T2>
struct common_type_two {
    using type = typename std::decay<decltype(true ? std::declval<T1>() : std::declval<T2>())>::type;
};

template <typename... Ts>
struct common_type { };

template <typename T0>
struct common_type<T0> {
    using type = T0;
};

template <typename T0, typename T1, typename... Ts>
struct common_type<T0, T1, Ts...> {
    using type = typename common_type_two<T0, typename common_type<T1, Ts...>::type>::type;
};

template <typename T0, typename... Ts>
constexpr auto get_common_type(T0 t0, Ts... ts)
{
    if constexpr (sizeof...(ts) == 0) {
        return t0;
    } else {
        return 0 ? t0 : get_common_type(ts...);
    }
}

/* memory */

static char buffer[65536 * 161];

struct my_memory_resource {
    char* buf;
    std::size_t watermark = 0;

    char* allocate(size_t n, size_t align)
    {

        watermark = (watermark + align - 1) / align * align;
        char* p = buf + watermark;
        if (watermark + n > sizeof(buffer)) {
            throw std::bad_alloc();
        }
        watermark += n;
        return p;
    }
    void release() noexcept
    {
        watermark = 0;
    }
};

template <class T>
struct my_allocator {
    using value_type = T;
    std::shared_ptr<my_memory_resource> m_resource = nullptr;

    my_allocator() = default;
    my_allocator(std::shared_ptr<my_memory_resource> resouece)
        : m_resource(resouece)
    {
    }

    T* allocate(size_t n)
    {
        char* p = m_resource->allocate(n * sizeof(T), alignof(T));
        return (T*)(p);
    }
    void deallocate(T* p, size_t n)
    {
        // do nothing
    }

    void release() noexcept
    {
        m_resource->release();
    }

    template <class U>
    constexpr my_allocator(const my_allocator<U>& other) noexcept
        : m_resource(other.m_resource) // 必须复制 resource！
    {
    }
    constexpr bool operator==(const my_allocator& other) noexcept
    {
        return m_resource == other.m_resource;
    }
};
#include <format>

struct memory_resource_inspector : std::pmr::memory_resource {
public:
    explicit memory_resource_inspector(std::pmr::memory_resource* upstream)
        : m_upstream(upstream)
    {
    }

private:
    void* do_allocate(size_t bytes, size_t alignment) override
    {
        void* p = m_upstream->allocate(bytes, alignment);
        auto s = std::format("allocate    {}  {}  {}\n", p, bytes, alignment);
        std::cout << s;
        return p;
    }

    bool do_is_equal(std::pmr::memory_resource const& other) const noexcept override
    {
        return other.is_equal(*m_upstream);
    }

    void do_deallocate(void* p, size_t bytes, size_t alignment) override
    {
        auto s = std::format("deallocate  {}  {}  {}\n", p, bytes, alignment);
        std::cout << s;
        return m_upstream->deallocate(p, bytes, alignment);
    }

    std::pmr::memory_resource* m_upstream;
};

int main(int, char**)
{

    memory_resource_inspector mem { std::pmr::new_delete_resource() };

    std::pmr::vector<int> s { &mem };
    for (int i = 0; i < 4096; i++) {
        s.push_back(i);
    }
}
