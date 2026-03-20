#include "json.hpp"
#include <iostream>
#include <json/json.h>
#include <string>

struct Student {
    int age;
    std::string name;

    void ff() {
        std::cout << "hello world" << std::endl;
    }

    REFLECT(age, name);
};

REFLECT_TYPE(Student, age, name, ff);

template <typename T>
std::string serialize(T const &t) {
    Json::Value root;
    T::template foreach_members_ptr<T>(
        [&](char const *key, auto &&value) { root[key] = t.*value; });
    return root.toStyledString();
}

template <typename T>
std::string serialize2(T const &t) {
    Json::Value root;
    reflect::reflect_traits<T>::foreach_members_ptr(
        [&](char const *key, auto ptr) {
            using PtrType = decltype(ptr);

            if constexpr (std::is_member_function_pointer_v<PtrType>) {
                // 函数，只存函数名
                root[key] = std::string(key);

            } else {
                root[key] = t.*ptr;
            }
        });
    return root.toStyledString();
}

int main(int, char **) {
    Student student = {
        .age = 10,
        .name = "Tom",
    };
    std::string s = serialize(student);
    std::string s2 = serialize2(student);
    std::cout << s << std::endl;
    std::cout << s2 << std::endl;
}
