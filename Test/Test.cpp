#include <iostream>
#include <Parser.h>
#include <assert.h>
#include <cmath>

#include "Test.h"

bool number_equals(double n1, double n2) {
    return fabs(n1 - n2) < 0.0001;
}

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
    assert(o1.str == "Bugs Bunny");
}

void test_move() {
    std::map<std::string_view, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject(std::string_view("Bugs Bunny")));

    jacc::JSONObject o1(map1);
    jacc::JSONObject o2(std::move(o1));

    assert(o1.type == jacc::JSON_UNDEFINED);
    assert(o2.type == jacc::JSON_OBJECT);
    assert(o2.object["customer_name"].str == "Bugs Bunny");

    jacc::JSONObject o3;

    assert(o3.type == jacc::JSON_UNDEFINED);

    o3 = std::move(o2);

    assert(o2.type == jacc::JSON_UNDEFINED);
    assert(o3.type == jacc::JSON_OBJECT);
    assert(o3.object["customer_name"].str == "Bugs Bunny");
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

    map2.emplace("customer", std::move(o1));

    jacc::JSONObject o2(map2);

    assert(o2.type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].object["customer_id"].type = jacc::JSON_NUMBER);
    assert(o2.object["customer"].object["customer_name"].str == "Bugs Bunny");
}

void test_array_ctor() {
    std::map<std::string_view, jacc::JSONObject> map;

    map.emplace("customer_id", jacc::JSONObject(1001.));
    map.emplace("customer_name", jacc::JSONObject(std::string_view("Bugs Bunny")));

    jacc::JSONObject o1(map);

    map.emplace("customer_id", jacc::JSONObject(1002.));
    map.emplace("customer_name", jacc::JSONObject(std::string_view("Daffy Duck")));

    jacc::JSONObject o2(map);

    std::vector<jacc::JSONObject> list;

    list.push_back(std::move(o1));
    list.push_back(std::move(o2));

    jacc::JSONObject o3(list);

    assert(o3.type == jacc::JSON_ARRAY);
    assert(o3.array.size() == 2);
    assert(o3.array[1].object["customer_name"].str == "Daffy Duck");
}

void test_num_array() {
    const char* json = "[1.11, 3, -10, 4.44]";
    jacc::Parser p;

    p.parse(json);

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array.size() == 4);
    assert(p.root.array[3].type == jacc::JSON_NUMBER);
    assert(number_equals(4.44, p.root.array[3].number));
}

int main()
{
    test_str_ctor();
    test_map_ctor();
    test_array_ctor();
    test_move();
    test_peek();
    test_num_array();
}
