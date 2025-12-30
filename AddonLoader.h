#pragma once
#include <string>
#include <vector>
struct AddonMeta{ std::string id; std::string title; std::string developer; std::string thumbnail; };
class AddonLoader{
public:
  std::vector<AddonMeta> scanAddons();
};
