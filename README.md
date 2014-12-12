Jzon is a JSON parser for C++ with focus on a nice and easy to use interface.

No dependencies, except the standard library.

#### Code
```c
Jzon::Node node = Jzon::object();
node.add("name", "value");
node.add("number", 20);
node.add("anothernumber", 15.3);
node.add("bool", true);

{
  Jzon::Node array = Jzon::array();
  array.add(1);
  array.add("asdf");
  
  {
    Jzon::Node array_node = Jzon::object();
    array_node.add("key1", "val1");
    array_node.add("key2", "val2");
    array.add(array_node);
  }

  node.add("array", array);
}

{
  Jzon::Node subnode = Jzon::object();
  subnode.add("key1", "val1");
  subnode.add("key2", "val2");
  node.add("subnode", subnode);
}

Jzon::Writer writer;
writer.writeStream(node, cout);
```

#### Result
```json
{
  "name": "value",
  "number": 20,
  "anothernumber": 15.3,
  "bool": true,
  "array": [
    1,
    "asdf",
    {
      "key1": "val1",
      "key2": "val2"
    }
  ],
  "subnode": {
    "key1": "val1",
    "key2": "val2"
  }
}
```
