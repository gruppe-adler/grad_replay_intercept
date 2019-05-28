#include "ReplayPart.h"

using namespace grad::replay;

ReplayPart::ReplayPart(types::auto_array<types::game_value> replay, std::vector<std::optional<Record*>> prevRecordPtrVec) {
    for (int i = 0; i < replay.size(); i++) {
        if(replay[i].is_null()) {
            this->records.push_back(std::nullopt);
        }
        else if (replay[i].type_enum() == game_data_type::ARRAY) {
            this->records.push_back(Record(replay[i].to_array(), prevRecordPtrVec.size() > 0 && i < prevRecordPtrVec.size() ? prevRecordPtrVec[i].value : 
                std::make_shared<Record>("", -1, Position(0, 0), -1.0f, "", "", std::nullopt).get()));
        }
        else {
            this->time = ReplayPart::convertDaytimeToString(replay[i]);
        }
    }    
};

std::string ReplayPart::convertDaytimeToString(float daytime) {
    std::stringstream daytimeSStream;
    auto hour = std::floor(daytime);
    auto minute = std::floor((daytime - hour) * 60);
    auto second = std::floor((((daytime - hour) * 60) - minute) * 60);

    if (hour < 10)
        daytimeSStream << "0";
    daytimeSStream << hour << ":";
    
    if (minute < 10)
        daytimeSStream << "0";
    daytimeSStream << minute << ":";

    if (second < 10)
        daytimeSStream << "0";
    daytimeSStream << second;

    return daytimeSStream.str();
}

void grad::replay::to_json(nl::json& j, const ReplayPart& rp)
{
    j = nl::json();
    for (auto& record : rp.records) {
        if(record) {
            j["data"].push_back(record);
        } else {
            // TODO: Remove after Debug
            j["data"].push_back(json::array());
        }
    }
    j["time"] = rp.time;
}

// TODO
void grad::replay::from_json(const nl::json& j, ReplayPart& rp)
{
    j.at("record").get_to(rp.records);
    j.at("time").get_to(rp.time);
}

