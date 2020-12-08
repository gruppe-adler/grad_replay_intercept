#include "Record.h"

using namespace grad::replay;

// ["iconMan",0,[1887.71,5696.76],0,"Willard"," (Alpha 1-1)",[123,456]]
// TODO: handle case when prevPtr == nullptr

Record::Record(std::string icon, int color, Position position, float direction, std::string name, std::string group, std::optional<Position> target, const std::map<int, std::string>& colorMap) :
    icon(icon), color(color), position(position), direction(direction), name(name), group(group), target(target), colorMap(colorMap) {}

Record::Record(types::auto_array<types::game_value> record, std::shared_ptr<Record> prevPtr, const std::map<int, std::string>& colorMap) : prevPtr(prevPtr), colorMap(colorMap) {
    this->icon = record[0].is_nil() ? this->prevPtr->icon : std::string(record[0]);
    this->color = record[1].is_nil() ? this->prevPtr->color : (int)record[1];
    
    if (record[2].is_nil()) {
        this->position = this->prevPtr->position;
    }
    else {
        auto posArray = record[2].to_array();
        this->position = Position(posArray[0], posArray[1]);
    }
    
    this->direction = record[3].is_nil() ? this->prevPtr->direction : (float)record[3];
    this->name = record[4].is_nil() ? this->prevPtr->name : std::string(record[4]);
    this->group = record[5].is_nil() ? this->prevPtr->group : std::string(record[5]);

    if (record.size() < 7 || record[6].is_nil() || record[6].to_array().size() == 0) {
        this->target = std::nullopt;
    }
    else {
        auto targetPos = record[6].to_array();
        this->target = Position(targetPos[0], targetPos[1]);
    }
}

void grad::replay::to_json(nl::json& j, const Record& r)
{

    std::string icon = "unknown";
    auto findResult = std::find_if(Record::iconTypes.begin(), Record::iconTypes.end(), [r](std::string & type) {
        return sqf::is_kind_of(r.icon, type);
    });

    if (findResult != Record::iconTypes.end()) {
        icon = *findResult;
        if (icon == "StaticWeapon")
            icon = "Static";
        else if (icon == "Truck_F")
            icon = "Truck";
        else if (icon == "Man")
            icon = "iconMan";
        else if (icon == "ParachuteBase")
            icon = "Parachute";
        else if (icon == "Land_DataTerminal_01_F")
            icon = "radiounit";
    }
    std::string color;
    if (r.colorMap.find(r.color) != r.colorMap.end()) {
        color = r.colorMap.at(r.color);
    }
    else {
        color = "rgba(1, 1, 1, 1)";
    }
    j = nl::json{ {"icon", icon} , {"color", color }, { "position", r.position },
        { "direction", r.direction }, { "name", r.name }, { "group", r.group }};
    if (r.target) {
        j["target"] = r.target.value();
    }
}


void grad::replay::from_json(const nl::json& j, Record& r)
{
    j.at("icon").get_to(r.icon);
    j.at("color").get_to(r.color);
    j.at("position").get_to(r.position);
    j.at("direction").get_to(r.direction);
    j.at("name").get_to(r.name);
    j.at("group").get_to(r.group);
    if (j.find("target") == j.end()) {
        r.target = std::nullopt;
    }
    else {
        r.target = std::make_optional(j.at("target"));
    }
}
