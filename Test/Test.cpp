#include <iostream>
#include <Parser.h>
#include <StringReader.h>
#include <assert.h>
#include <cmath>

#include "Test.h"

bool number_equals(double n1, double n2) {
    return fabs(n1 - n2) < 0.0001;
}

void test_string_reader()
{
    auto src = "{    \"id:\", 10}";
    jacc::StringReader p(src);

    assert(p.peek() == '{');

    p.putback();

    assert(p.pop() == '{');
    p.putback();
    assert(p.pop() == '{');
}

void test_str_ctor() {
    jacc::JSONObject o1(std::string("Bugs Bunny"));

    assert(o1.type == jacc::JSON_STRING);
    assert(o1.str == "Bugs Bunny");
}

void test_move() {
    std::map<std::string, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject(std::string("Bugs Bunny")));

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
    std::map<std::string, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject(std::string("Bugs Bunny")));

    assert(map1.size() == 2);

    jacc::JSONObject o1(map1);

    assert(o1.type == jacc::JSON_OBJECT);
    assert(o1.object.size() == 2);
    assert(map1.size() == 0);

    std::map<std::string, jacc::JSONObject> map2;

    map2.emplace("customer", std::move(o1));

    jacc::JSONObject o2(map2);

    assert(o2.type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].type == jacc::JSON_OBJECT);
    assert(o2.object["customer"].object["customer_id"].type = jacc::JSON_NUMBER);
    assert(o2.object["customer"].object["customer_name"].str == "Bugs Bunny");
}

void test_array_ctor() {
    std::map<std::string, jacc::JSONObject> map;

    map.emplace("customer_id", jacc::JSONObject(1001.));
    map.emplace("customer_name", jacc::JSONObject(std::string("Bugs Bunny")));

    jacc::JSONObject o1(map);

    map.emplace("customer_id", jacc::JSONObject(1002.));
    map.emplace("customer_name", jacc::JSONObject(std::string("Daffy Duck")));

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
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array.size() == 4);
    assert(p.root.array[3].type == jacc::JSON_NUMBER);
    assert(number_equals(4.44, p.root.array[3].number));
    assert(number_equals(-10.0, p.root.array[2].number));
}

void test_bool_array() {
    const char* json = "[true, false, true, false, false]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array.size() == 5);
    assert(p.root.array[3].type == jacc::JSON_BOOLEAN);
    assert(p.root.array[3].booleanValue == false);
    assert(p.root.array[2].booleanValue == true);
}

void test_null_array() {
    const char* json = "[true, null, true, false, null]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array.size() == 5);
    assert(p.root.array[1].type == jacc::JSON_NULL);
    assert(p.root.array[4].type == jacc::JSON_NULL);
}

void test_str_array() {
    const char* json = "[\"Hello\", \"Wonderful\", \"World\\nMoon\"]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array.size() == 3);
    assert(p.root.array[1].type == jacc::JSON_STRING);
    assert(p.root.array[1].str == "Wonderful");
    assert(p.root.array[2].str == "World\nMoon");
}

void test_object() {
    const char* json = R"(
{
  "name": "Bugs Bunny",
  "age": 10,
  "manager": {
    "name": "Daffy Duck"
  }
}
)";

    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_OBJECT);
    assert(p.root.object["name"].type == jacc::JSON_STRING);
    assert(p.root.object["age"].type == jacc::JSON_NUMBER);
    assert(p.root.object["manager"].type == jacc::JSON_OBJECT);
    assert(p.root.object["manager"].object["name"].str == "Daffy Duck");
}

void test_mixed_array() {
    const char* json = R"(
[
  "Hello",
  true,
  {
    "name": "Roger Rabbit"
  }
]
)";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(p.root.type == jacc::JSON_ARRAY);
    assert(p.root.array[2].object["name"].type == jacc::JSON_STRING);
    assert(p.root.array[2].object["name"].str == "Roger Rabbit");
}

int main()
{
    test_str_ctor();
    test_map_ctor();
    test_array_ctor();
    test_move();
    test_string_reader();
    test_num_array();
    test_bool_array();
    test_null_array();
    test_str_array();
    test_object();
    test_mixed_array();
}
