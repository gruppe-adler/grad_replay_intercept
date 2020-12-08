#pragma once

#include <intercept.hpp>

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

#include "Position.h"

using namespace intercept;
namespace nl = nlohmann;

namespace grad {
    namespace replay {
        class Record {
        public:
            std::string icon;
            int color;
            Position position;
            float direction;
            std::string name;
            std::string group;
            std::optional<Position> target;

            std::shared_ptr<Record> prevPtr;
            const std::map<int, std::array<float_t, 4>>& colorMap;

            static inline std::vector<std::string> iconTypes {
                "Land_DataTerminal_01_F", "StaticMortar", "StaticWeapon", "ParachuteBase", "Tank", "Motorcycle", "Helicopter", "Plane", "Ship", "Truck_F", "Static", "Car", "Man"
            };
            
            Record(std::string icon, int color, Position position, float direction, std::string name, std::string group, std::optional<Position> target, const std::map<int, std::array<float_t, 4>>& colorMap);
            Record(types::auto_array<types::game_value> record, std::shared_ptr<Record> prevPtr, const std::map<int, std::array<float_t, 4>>& colorMap);
        };
        void to_json(nl::json& j, const Record& r);
        void from_json(const nl::json& j, Record& r);
    };
};