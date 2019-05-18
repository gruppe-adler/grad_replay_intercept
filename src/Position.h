#pragma once

#include <nlohmann/json.hpp>

namespace nl = nlohmann;

namespace grad {
    namespace replay {
        class Position {
        public:
            float x, y;
            Position() : x(0), y(0) {};
            Position(float x, float y) : x(x), y(y) {};
        };

        void inline to_json(nl::json& j, const Position& p) {
            j = nl::json{ p.x,p.y };
        };
        void inline from_json(const nl::json& j, Position& p) {
            j[0].get_to(p.x);
            j[1].get_to(p.y);
        };
    };
};