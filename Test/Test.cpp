#include <iostream>
#include <Parser.h>
#include <StringReader.h>
#include <FileReader.h>
#include <MemoryMappedReader.h>
#include <assert.h>
#include <cmath>
#include <fstream>

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
    jacc::JSONObject o1("Bugs Bunny");

    assert(o1.isString());
    assert(o1.string() == "Bugs Bunny");
}

void test_move() {
    std::map<std::string, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject("Bugs Bunny"));

    jacc::JSONObject o1(map1);
    jacc::JSONObject o2(std::move(o1));

    assert(o1.isUndefined());
    assert(o2.isObject());
    assert(o2.object()["customer_name"].string() == "Bugs Bunny");

    jacc::JSONObject o3;

    assert(o3.isUndefined());

    o3 = std::move(o2);

    assert(o2.isUndefined());
    assert(o3.isObject());
    assert(o3.object()["customer_name"].string() == "Bugs Bunny");
}

void test_map_ctor() {
    std::map<std::string, jacc::JSONObject> map1;

    map1.emplace("customer_id", jacc::JSONObject(1001.));
    map1.emplace("customer_name", jacc::JSONObject("Bugs Bunny"));

    assert(map1.size() == 2);

    jacc::JSONObject o1(map1);

    assert(o1.isObject());
    assert(o1.object().size() == 2);
    assert(map1.size() == 0);

    std::map<std::string, jacc::JSONObject> map2;

    map2.emplace("customer", std::move(o1));

    jacc::JSONObject o2(map2);

    assert(o2.isObject());
    assert(o2.object()["customer"].isObject());
    assert(o2.object()["customer"].object()["customer_id"].isNumber());
    assert(o2.object()["customer"].object()["customer_name"].string() == "Bugs Bunny");
}

void test_array_ctor() {
    std::map<std::string, jacc::JSONObject> map;

    map.emplace("customer_id", jacc::JSONObject(1001.));
    map.emplace("customer_name", jacc::JSONObject("Bugs Bunny"));

    jacc::JSONObject o1(map);

    map.emplace("customer_id", jacc::JSONObject(1002.));
    map.emplace("customer_name", jacc::JSONObject("Daffy Duck"));

    jacc::JSONObject o2(map);

    std::vector<jacc::JSONObject> list;

    list.push_back(std::move(o1));
    list.push_back(std::move(o2));

    jacc::JSONObject o3(list);

    assert(o3.isArray());
    assert(o3.array().size() == 2);
    assert(o3.array()[1].object()["customer_name"].string() == "Daffy Duck");
}

void test_num_array() {
    const char* json = "[1.11, 3, -10, 4.44]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array().size() == 4);
    assert(root.array()[3].isNumber());
    assert(number_equals(4.44, root.array()[3].number()));
    assert(number_equals(-10.0, root.array()[2].number()));
}

void test_bool_array() {
    const char* json = "[true, false, true, false, false]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array().size() == 5);
    assert(root.array()[3].isBoolean());
    assert(root.array()[3].boolean() == false);
    assert(root.array()[2].boolean() == true);
}

void test_null_array() {
    const char* json = "[true, null, true, false, null]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array().size() == 5);
    assert(root.array()[1].isNull());
    assert(root.array()[4].isNull());
}

void test_str_array() {
    const char* json = "[\"Hello\", \"Wonderful\", \"World\\nMoon\"]";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array().size() == 3);
    assert(root.array()[1].isString());
    assert(root.array()[1].string() == "Wonderful");
    assert(root.array()[2].string() == "World\nMoon");
}

void test_unicode() {
    const char* json = R"(
["Omega \u03A9", "Japanese \u8A9E", "Pair \uD834\uDD1E"]
)";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array().size() == 3);
    assert(root.array()[1].isString());

    assert(root.array()[0].string() == "Omega \u03A9");
    assert(root.array()[1].string() == "Japanese \u8A9E");
    assert(root.array()[2].string() == "Pair \U0001D11E");
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

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isObject());
    assert(root.object()["name"].isString());
    assert(root.object()["age"].isNumber());
    assert(root.object()["manager"].isObject());
    assert(root.object()["manager"].object()["name"].string() == "Daffy Duck");
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

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root.array()[2].object()["name"].isString());
    assert(root.array()[2].object()["name"].string() == "Roger Rabbit");
}

void test_file_reader() {
    const char* json = R"(
[
  "Hello",
  true,
  {
    "name": "Roger Rabbit"
  }
]
)";
    const char* file_name = "__test.json";
 
    {
        std::ofstream test_file(file_name);

        test_file << json;
    } //Closes file

    {
        jacc::FileReader reader(file_name);
        jacc::Parser p(reader);

        auto root = p.parse();

        assert(p.error_code == jacc::ERROR_NONE);
        assert(root.isArray());
        assert(root.array()[2].object()["name"].isString());
        assert(root.array()[2].object()["name"].string() == "Roger Rabbit");
    } //Closes file

    std::remove(file_name);
}


void test_memory_map_reader() {
    const char* json = R"(
[
  "Hello",
  true,
  {
    "name": "Roger Rabbit"
  }
]
)";
    const char* file_name = "__test.json";

    {
        std::ofstream test_file(file_name);

        test_file << json;
    } //Closes file

    {
        jacc::MemoryMappedReader reader(file_name);

        assert(reader.data.size() > 0);
        assert(reader.data.data() != nullptr);

        jacc::Parser p(reader);

        auto root = p.parse();

        assert(p.error_code == jacc::ERROR_NONE);
        assert(root.isArray());
        assert(root.array()[2].object()["name"].isString());
        assert(root.array()[2].object()["name"].string() == "Roger Rabbit");
    } //Closes file

    std::remove(file_name);
}

void test_read_codepoint() {
    const char* str = "8A9E";
    jacc::StringReader reader(str);
    jacc::Parser p(reader);
    
    unsigned long cp = p.read_codepoint();
    
    assert(p.error_code == jacc::ERROR_NONE);
    assert(cp == 0x8A9E);
}

void test_utf16_decode() {
    jacc::StringReader reader("");
    jacc::Parser p(reader);
    
    unsigned long cp = p.decode_utf16(0xD834, 0xDD1E);
    
    assert(p.error_code == jacc::ERROR_NONE);
    assert(cp == 0x1D11E);
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
    test_unicode();
    test_object();
    test_mixed_array();
    test_file_reader();
    test_memory_map_reader();
    test_read_codepoint();
    test_utf16_decode();
}
