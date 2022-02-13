#include "module.h"
#include <iostream>

MudoleTest::MudoleTest(/* args */)
{
    std::cout << "hello world";
    std::cout << "MudoleTest()" << this->n << std::endl;
}

MudoleTest::~MudoleTest()
{
    std::cout << "~MudoleTest()" << this->n << std::endl;
}