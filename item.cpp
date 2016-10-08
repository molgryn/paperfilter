#include "item.hpp"
#include "item_value.hpp"
#include <vector>

class Item::Private {
public:
  std::string name;
  std::string attr;
  std::string range;
  std::unique_ptr<ItemValue> value;
  std::vector<std::unique_ptr<Item>> children;
};

Item::Item() : d(new Private()) {}

Item::~Item() {}
