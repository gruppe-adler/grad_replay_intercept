#pragma once

#include <intercept.hpp>

#include <nlohmann/json.hpp>

#include <vector>
#include <string>
#include <sstream>

#include "Record.h"

using namespace intercept;

namespace nl = nlohmann;

namespace grad {
    namespace replay {
        class ReplayPart {
        public:
            std::vector<std::optional<std::shared_ptr<Record>>> records;
            std::string time;
            std::shared_ptr<ReplayPart> prevReplayPart;

            ReplayPart(types::auto_array<types::game_value> replay, std::shared_ptr<ReplayPart> replayPart, const std::map<int, std::string>& colorMap);
            static std::string convertDaytimeToString(float daytime);
        };

        void to_json(nl::json& j, const ReplayPart& rp);
        void from_json(const nl::json& j, ReplayPart& rp);
    };
};