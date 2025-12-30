#include "AddonLoader.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp> // optional, include single header nlohmann json in third_party
using json = nlohmann::json;
std::vector<AddonMeta> AddonLoader::scanAddons(){
  std::vector<AddonMeta> out;
  std::filesystem::path base = std::filesystem::path(std::getenv("HOME"))/".dash-duck"/"addons";
  if(!std::filesystem::exists(base)) return out;
  for(auto &d : std::filesystem::directory_iterator(base)){
    if(!d.is_directory()) continue;
    auto metaFile = d.path()/"addon.json";
    if(!std::filesystem::exists(metaFile)) continue;
    std::ifstream f(metaFile);
    try{
      json j; f >> j;
      AddonMeta m;
      m.id = j.value("id", "");
      m.title = j.value("title", "(no title)");
      m.developer = j.value("developer", "");
      m.thumbnail = (d.path()/j.value("thumbnail", "thumbnail.png")).string();
      out.push_back(m);
    }catch(...){ /* fehlerhafte addon.json überspringen */ }
  }
  return out;
}
