#include "target.h"

#include <vector>
#include <string>
#include <algorithm>

std::vector<Target> TargetManager::targets;

bool TargetManager::set(const std::string &name, Player *data)
{
    const auto it = find(name);
    if (targets.end() == it) {
        return false;
    }

    it->data = data;
    return true;
}

void TargetManager::remove(const std::string &name)
{
    const auto it = find(name);
    if (targets.end() != it) {
        targets.erase(it);
    }
}

std::vector<Target>::iterator TargetManager::find(const std::string &name)
{
    return std::find_if(targets.begin(), targets.end(), [&name](const Target &t) {
        return name == t.name;
    });
}