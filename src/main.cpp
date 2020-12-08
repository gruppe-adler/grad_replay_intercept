#include <intercept.hpp>

#include "ReplayPart.h"

#include <nlohmann/json.hpp>

#include <cpr/cpr.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <filesystem>
#include <memory>
#include <algorithm>

#define GRAD_REPLAY_USER_AGENT "grad_replay_intercept/0.3"

namespace nl = nlohmann;

namespace fs = std::filesystem;

using namespace intercept;
using namespace grad::replay;

using SQFPar = game_value_parameter;

std::string url = "";
std::string token = "";
std::chrono::system_clock::time_point missionStart;
fs::path basePath;

static inline std::map<int, std::string> defaultColorMap {
    {0,"rgba(0, 0.3, 0.6, 1)"},     // 0: WEST
    {1,"rgba(0.5, 0, 0, 1)"},       // 1: EAST
    {2,"rgba(0, 0.5, 0, 1)"},       // 2: INDEPENDENT
    {3,"rgba(0.4, 0, 0.5, 1)"},     // 3: CIVILIAN
    {4,"rgba(0.7, 0.6, 0, 1)"},     // 4: SIDEEMPTY
    {5,"rgba(0, 0.3, 0.6, 0.5)"},   // 5: WEST unconscious
    {6,"rgba(0.5, 0, 0, 0.5)"},     // 6: EAST unconscious
    {7,"rgba(0, 0.5, 0, 0.5)"},     // 7: INDEPENDENT unconscious
    {8,"rgba(0.4, 0, 0.5, 0.5)"},   // 8: CIVILIAN unconscious
    {9,"rgba(0.7, 0.6, 0, 0.5)"},   // 9: SIDEEMPTY unconscious
    {10,"rgba(0.2, 0.2, 0.2, 0.5)"},// 10: dead unit
    {11,"rgba(1, 0, 0, 1)"}         // 11: funkwagen-red when sending, speciality for "breaking contact"
};

std::string timePointToString(std::chrono::system_clock::time_point timePoint) {
    std::time_t missionStartInTimeT = std::chrono::system_clock::to_time_t(timePoint);
    std::stringstream timeStream;
    timeStream << std::put_time(std::localtime(&missionStartInTimeT), "%F %T");
    return timeStream.str();
}

int intercept::api_version() {
    return INTERCEPT_SDK_API_VERSION;
}

void intercept::register_interfaces() {}

void intercept::pre_init() {
    intercept::sqf::diag_log(sqf::text("[GRAD] (replay_intercept) INFO: Running"));
}

std::map<int, std::string> constructColorMap(types::auto_array<types::game_value> colorArray) {
    // not sure why this is a map
    std::map<int, std::string> colorMap;
    for (size_t i = 0; i < colorArray.size(); i++)
    {
        auto rgba = colorArray[i].to_array();
        std::stringstream rgbaStrStream;
        rgbaStrStream << "rgba("
            << (float_t)rgba[0] << ", "
            << (float_t)rgba[1] << ", "
            << (float_t)rgba[2] << ", "
            << (float_t)rgba[3] << ")";
        colorMap.insert({ i, rgbaStrStream.str() });
    }
    return colorMap;
}

nl::json constructData(types::auto_array<types::game_value> parameters) {

    if (parameters.size() == 2) {

        std::map<int, std::string> colorMap;
        if (parameters[1].is_nil()) {
            client::invoker_lock thread_lock;
            sqf::diag_log(sqf::text("[GRAD] (replay_intercept) WARNING: GRAD_REPLAY_COLORS is nil, using default colors!"));
            colorMap = defaultColorMap;
        }
        else {
            colorMap = constructColorMap(parameters[1].to_array());
        }

        types::auto_array rootArray = parameters[0].to_array();

        std::vector<std::shared_ptr<ReplayPart>> replayParts;
        replayParts.reserve(rootArray.size());

        std::shared_ptr<ReplayPart> prevReplayPart;

        for (int i = 0; i < rootArray.size(); i++) {
            auto replayPart = std::make_shared<ReplayPart>(rootArray[i].to_array(), prevReplayPart, colorMap);
            replayParts.push_back(replayPart);
            prevReplayPart = replayPart;
        }

        auto result = nl::json();

        for (auto& replayPart : replayParts) {
            result.push_back(*replayPart);
        }

        return result;
    }
    client::invoker_lock thread_lock;
    sqf::diag_log(sqf::text("[GRAD] (replay_intercept) ERROR: Old syntax is no longer supported!"));
    return nl::json();
}

void dumpReplayAsJson(const std::chrono::system_clock::time_point& now, const nlohmann::json& obj) {
    auto path = std::string(timePointToString(now)).append(".json");
    std::replace(path.begin(), path.end(), ':', '-');
    std::ofstream o(basePath / path);
    o << std::setw(4) << obj << std::endl;
}

game_value sendReplay(game_state& gs, SQFPar right_arg) {

    try {
        // Construct JSON Object
        auto obj = nl::json();
        obj["missionName"] = sqf::briefing_name();
        obj["date"] = timePointToString(missionStart);

        auto now = std::chrono::system_clock::now();
        obj["duration"] = std::chrono::duration_cast<std::chrono::seconds>(now - missionStart).count();

        //auto worldName = std::string(sqf::world_name());
        //std::transform(worldName.begin(), worldName.end(), worldName.begin(), ::tolower);
        obj["worldName"] = sqf::world_name(); // worldName;

        // Config
        auto gradReplayConfig = sqf::config_entry(sqf::mission_config_file()) >> ("GRAD_replay");

        auto precision = (int)sqf::get_number(gradReplayConfig >> ("precision"));

        auto trackedSidesArr = sqf::get_array(gradReplayConfig >> ("trackedSides")).to_array();
        auto trackedSides = std::vector<std::string>();
        for (auto& trackedSide : trackedSidesArr) {
            trackedSides.push_back(trackedSide);
        }

        auto stepsPerTick = (int)sqf::get_number(gradReplayConfig >> ("stepsPerTick"));
        auto trackedVehicles = (bool)sqf::get_number(gradReplayConfig >> ("trackedVehicles"));
        auto trackedAI = (bool)sqf::get_number(gradReplayConfig >> ("trackedAI"));
        auto sendingChunkSize = (int)sqf::get_number(gradReplayConfig >> ("sendingChunkSize"));
        auto trackShots = (bool)sqf::get_number(gradReplayConfig >> ("trackShots"));

        obj["config"] = { 
            {"precision", precision }, 
            {"trackedSides", trackedSides}, 
            {"stepsPerTick", stepsPerTick}, 
            {"trackedVehicles", trackedVehicles},
            {"trackedAI", trackedAI}, 
            {"sendingChunkSize", sendingChunkSize}, 
            {"trackShots", trackShots} 
        };

        // Replay
        obj["data"] = constructData(right_arg.to_array());

        std::thread sendReplayThread([obj, now]() {
            try
            {
                std::stringstream ss;
                ss << obj;

                auto header = cpr::Header{
                    {"Content-Type","application/json"},
                    {"content-length", std::to_string(ss.str().size())},
                    {"Connection", "close"},
                    {"Authorization", std::string("Bearer ").append(token)},
                    {"User-Agent", GRAD_REPLAY_USER_AGENT}
                };

                cpr::Response response = cpr::Post(
                    cpr::Url{ url },
                    header,
                    cpr::Body(ss.str()),
                    cpr::Timeout(60000)
                );

                if (response.status_code != 201) {
                    dumpReplayAsJson(now, obj);
                }

                std::stringstream responseLog;
                responseLog << "[GRAD] (replay_intercept) INFO: POST Request Status Code: " << response.status_code << " Request time: " << response.elapsed;
                client::invoker_lock thread_lock;
                sqf::diag_log(sqf::text(responseLog.str()));
            }
            catch (std::exception& ex)
            {
                dumpReplayAsJson(now, obj);

                std::stringstream exceptionLog;
                exceptionLog << "[GRAD] (replay_intercept) INFO: Exception during POST Request " << ex.what();
                client::invoker_lock thread_lock;
                sqf::diag_log(sqf::text(exceptionLog.str()));
            }
        });
        sendReplayThread.detach();
    }
    catch (std::exception& ex) {
        std::stringstream responseLog;
        responseLog << "[GRAD] (replay_intercept) CRASH: " << ex.what();
        sqf::diag_log(sqf::text(responseLog.str()));
    }
    return true;
}

game_value startRecord() {
    missionStart = std::chrono::system_clock::now();
    return true;
}

void intercept::pre_start() {
    
    // load token from config
    boost::property_tree::ptree pt;
    auto path = std::filesystem::path("grad_replay_intercept_config.ini").string();
    try {
        boost::property_tree::ini_parser::read_ini(path, pt);
        url = pt.get<std::string>("Config.ReplayUrl");
        token = pt.get<std::string>("Config.BearerToken");
    }
    catch (boost::property_tree::ini_parser_error ex) {
        sqf::diag_log(sqf::text("[GRAD] (replay_intercept) WARNING: Couldn't parse grad_replay_intercept_config.ini, writing a new one"));
        pt.add<std::string>("Config.ReplayUrl", "https://replay.gruppe-adler.de/");
        pt.add<std::string>("Config.BearerToken", "InsertYourBearerTokenHere");
        boost::property_tree::ini_parser::write_ini(path, pt);
    }

    basePath = "grad_replay_intercept";

    if (!fs::exists(basePath)) {
        fs::create_directories(basePath);
    }
    
    static auto grad_replay_intercept_replay_send =
        client::host::register_sqf_command("gradReplayInterceptSendReplay", "Sends the replay", sendReplay, game_data_type::BOOL, game_data_type::ARRAY);

}

void intercept::post_init() {
    if ((bool)sqf::get_number(sqf::config_entry(sqf::mission_config_file()) >> ("GRAD_replay") >> ("upload"))) {
        startRecord();
        sqf::call(sqf::compile("['GRAD_replay_stopped', { gradReplayInterceptSendReplay [GRAD_REPLAY_DATABASE, GRAD_REPLAY_COLORS] }] call CBA_fnc_addEventHandler"));
    }
}

