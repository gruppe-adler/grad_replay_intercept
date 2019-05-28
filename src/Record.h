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

            Record* prevPtr;
            
            static inline std::map<int, std::string> colorMap {
                {0,"rgba(0,76,153,1)"},     // 0: WEST
                {1,"rgba(127,0,0,1)"},      // 1: EAST
                {2,"rgba(0,127,0,1)"},      // 2: INDEPENDENT
                {3,"rgba(102,0,127,1)"},    // 3: CIVILIAN
                {4,"rgba(178,153,0,1)"},    // 4: SIDEEMPTY
                {5,"rgba(0,76,153,127)"},   // 5: WEST unconscious
                {6,"rgba(127,0,0,127)"},    // 6: EAST unconscious
                {7,"rgba(0,127,0,127)"},    // 7: INDEPENDENT unconscious
                {8,"rgba(102,0,127,127)"},  // 8: CIVILIAN unconscious
                {9,"rgba(178,153,0,127)"},  // 9: SIDEEMPTY unconscious
                {10,"rgba(51,51,51,127)"},  // 10: dead unit
                {11,"rgba(1,0,0,1)"}        // 11: funkwagen-red when sending, speciality for "breaking contact"
            };

            static inline std::vector<std::string> iconTypes {
                "StaticMortar", "StaticWeapon", "ParachuteBase", "Tank", "Motorcycle", "Helicopter", "Plane", "Ship", "Truck_F", "Static", "Car", "Man"
            };
            
            Record() {};
            Record(std::string icon, int color, Position position, float direction, std::string name, std::string group, std::optional<Position> target);
            Record(types::auto_array<types::game_value> record, std::optional<Record*> prevPtr);
        };
        void to_json(nl::json& j, const Record& r);
        void from_json(const nl::json& j, Record& r);
    };
};