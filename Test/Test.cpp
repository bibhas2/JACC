#include <iostream>
#include <Parser.h>
#include <assert.h>

int main()
{
    auto src = "{    \"id:\", 10}";
    jacc::Parser p;

    p.parse(src);

    assert(p.peek() == '{');

    p.putback();

    assert(p.pop() == '{');
    p.putback();
    assert(p.pop() == '{');
    p.eat_space();
    assert(p.pop() == '\"');
}
