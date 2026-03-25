#pragma once

#include <system_error>

namespace co_async{
    int check_error(auto res){
        if (res == -1)[[unlikely]]{
            throw std::system_error(errno,std::system_category());
        }
        return res;
    }
};