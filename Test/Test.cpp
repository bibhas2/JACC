#include <iostream>
#include <Parser.h>
#include <assert.h>
#include "Test.h"

void test_peek()
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

void test_str_ctor() {
    jacc::JSONObject o1(std::string_view("Bugs Bunny"));

    assert(o1.type == jacc::JSON_STRING);
}

void test_move() {

}

void test_map_ctor() {
    std::map<std::string_view, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject(std::string_view("Bugs Bunny")));

    assert(map1.size() == 2);

    jacc::JSONObject o1(map1);

    assert(o1.type == jacc::JSON_OBJECT);
    assert(o1.object.size() == 2);
    assert(map1.size() == 0);

    std::map<std::string_view, jacc::JSONObject> map2;

    map2.emplace("customer", std::move(jacc::JSONObject(o1)));

    jacc::JSONObject o2(map2);

    assert(o2.type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].object["customer_id"].type = jacc::JSON_NUMBER);
    assert(o2.object["customer"].object["customer_name"].str == "Bugs Bunny");
}

int main()
{
    test_str_ctor();
    test_map_ctor();
    test_peek();
}
