#include <cstdio>
#include <iostream>
#include <type_traits>

template <class F, class... Args>
auto invoke(F f, Args&&... args)
{
    printf("begin\n");
    if constexpr (std::is_same_v<std::decay_t<decltype(f(std::forward<Args>(args)...))>, void>) {
        f(std::forward<Args>(args)...);
    } else {
        auto p = f(std::forward<Args>(args)...);
        return p;
    }
    printf("end\n");
}

template <class F, class... Args>
auto invokes(F f, Args&&... args)
{
    printf("begin\n");

    f(std::forward<Args>(args)...);

    printf("end\n");
}

template <class F, class... Args>
auto invokes(F f, Args&&... args)
    requires(!std::is_void_v<std::invoke_result_t<F, Args...>>)
{
    printf("begin\n");
    auto p = f(std::forward<Args>(args)...);
    printf("end\n");
    return p;
}

template <typename T, class = void>
struct has_dismentle {
    static constexpr bool value = false;
};
template <typename T>
struct has_dismentle<T, std::void_t<decltype(std::declval<T>().dismentle())>> {
    static constexpr bool value = true;
};

template <typename F>
auto func(F f)
{
    if constexpr (requires { {f.func()}->std::same_as<void>; }) {
        f.func();
    }
}

class myfunc {
public:
    void func()
    {
        std::cout << "7es" << std::endl;
    }
};

template <typename F, typename std::enable_if_t<std::is_same_v<decltype(std::declval<F>().func()), int>, int> = 0>
auto func(F f)
{
}

int main(int, char**)
{
    myfunc m;
    func(m);
}
