#include "json.hpp"
#include <iostream>
#include <json/json.h>
#include <string>

struct Student {
    int age;
    std::string name;
    REFLECT(age, name);
};

// REFLECT_TYPE(Student, age, name);

template <typename T>
std::string serialize(T const &t) {
    Json::Value root;
    T::template foreach_members_ptr<T>(
        [&](char const *key, auto const &value) { root[key] = t.*value; });
    return root.toStyledString();
}

int main(int, char **) {
    Student student = {
        .age = 10,
        .name = "Tom",
    };
    std::string s = serialize(student);
    std::cout << s << std::endl;
}
