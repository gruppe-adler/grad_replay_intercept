#include "ReplayPart.h"

using namespace grad::replay;

ReplayPart::ReplayPart(types::auto_array<types::game_value> replay, std::shared_ptr<ReplayPart> prevReplayPart, const std::map<int, std::string>& colorMap) {
    this->prevReplayPart = prevReplayPart;

    for (int i = 0; i < replay.size(); i++) {
        if(replay[i].is_null()) { // <null>
            this->records.push_back(std::nullopt);
        }
        else if (replay[i].type_enum() == game_data_type::ARRAY) { // [...]
            if (this->prevReplayPart != nullptr && this->prevReplayPart->records.size() > 0 && i < this->prevReplayPart->records.size()) {
                auto record = this->prevReplayPart->records[i];

                if (record) {
                    this->records.push_back(std::make_shared<Record>(replay[i].to_array(), record.value(), colorMap));
                }
                else { // prev was <null> or not present
                    auto prevPrevReplayPart = this->prevReplayPart->prevReplayPart;
                    
                    while (prevPrevReplayPart != nullptr) { // Iterate back until a record is found
                        if (prevPrevReplayPart->records.size() > 0 && i < prevPrevReplayPart->records.size()) {
                            auto record = prevPrevReplayPart->records[i];

                            if (record) {
                                this->records.push_back(std::make_shared<Record>(replay[i].to_array(), record.value(), colorMap));
                                prevPrevReplayPart = nullptr;
                            }
                        }
                        if (prevPrevReplayPart != nullptr) {
                            prevPrevReplayPart = prevPrevReplayPart->prevReplayPart;
                        }
                    }
                }
            }
            else {
                this->records.push_back(std::make_shared<Record>(replay[i].to_array(), std::make_shared<Record>("", -1, Position(0, 0), -1.0f, "", "", std::nullopt, colorMap), colorMap));
            }
        }
        else { // daytime, for example 12.0066
            this->time = ReplayPart::convertDaytimeToString(replay[i]);
        }
    }
    if (prevReplayPart && this->records.size() != prevReplayPart->records.size()) {
        while (this->records.size() < prevReplayPart->records.size())
        {
            this->records.push_back(std::nullopt);
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
            j["data"].push_back(*record.value());
        } 
        /* DEBUG
        else {
            j["data"].push_back(nl::json::array());
        }
        */
    }
    j["time"] = rp.time;
}

// TODO: should be possible
void grad::replay::from_json(const nl::json& j, ReplayPart& rp)
{
    //j.at("record").get_to(rp.records);
    j.at("time").get_to(rp.time);
}

