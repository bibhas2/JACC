## Overview
JACC is a JSON parser written in C++. It uses standard data structures like std::string, std::vector and std::map.

Basic example.

```c
void test_parsing() {
    const char* json = R"(
[
  "Hello",
  true,
  {
    "name": "Roger Rabbit",
    "id": 10023,
    "manager": {
        "name": "Bugs Bunny",
        "likes": ["Carrot", "Singing"]
    }
  }
]
)";
    jacc::StringReader reader(json);
    jacc::Parser p(reader);

    auto root = p.parse();

    assert(p.error_code == jacc::ERROR_NONE);
    assert(root.isArray());
    assert(root[2]["name"].isString());
    assert(root[2]["name"].string() == "Roger Rabbit");
    assert(root[2]["manager"]["name"].string() == "Bugs Bunny");
    assert(root[2]["manager"]["likes"][1].string() == "Singing");
}
```
