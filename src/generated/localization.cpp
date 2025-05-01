#include "localization.hpp"

std::unique_ptr<std::unordered_map<uint64_t, std::string>> current_localization_map;

std::string get_localized_message(uint64_t code)
{
    if (current_locale_map != nullptr) {
        auto msg_it = current_locale_map->find(code);
        if (msg_it != current_locale_map->end()) {
            return msg_it->second;
        }
    }
    return "Unknown localization code";
}
