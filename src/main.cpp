#include <intercept.hpp>

#include "ReplayPart.h"

#include <nlohmann/json.hpp>

#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/JSON/Object.h>

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

namespace nl = nlohmann;

namespace p = Poco;
namespace pn = Poco::Net;

using namespace intercept;
using namespace grad::replay;

using SQFPar = game_value_parameter;

#define REPLAY_URL "https://replay.gruppe-adler.de/"

std::string token = "";
std::chrono::system_clock::time_point missionStart;

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
    intercept::sqf::diag_log("The Intercept template plugin is running!");

    // load token from config
    boost::property_tree::ptree pt;
    auto path = std::filesystem::path("grad_replay_interept_config.ini").string();
    try {
        boost::property_tree::ini_parser::read_ini(path, pt);
        token = pt.get<std::string>("Config.BearerToken");
        sqf::diag_log(token);
    }
    catch (boost::property_tree::ini_parser_error ex) {
        sqf::diag_log("grad_replay_intercept couldn't parse @grad_replay_intercept/config.ini, writing a new one");
        pt.add<std::string>("Config.BearerToken", "InsertYourBearerTokenHere");
        boost::property_tree::ini_parser::write_ini(path, pt);
    }
}

nl::json constructData(types::auto_array<types::game_value> rootArray) {
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

    std::vector<ReplayPart> replayParts;
    replayParts.reserve(rootArray.size());

    for (int i = 0; i < rootArray.size(); i++) {
        std::vector<Record*> prevElVec;
        if (replayParts.size() > 0) {
            auto records = &replayParts[i - 1].records;
            for (int j = 0; j < records->size(); j++) {
                auto prevPtr = &records->at(j);
                if (prevPtr == nullptr) {                    
                    prevElVec.push_back(std::make_shared<Record>("", -1, Position(0, 0), -1.0f, "", "", std::nullopt).get());
                } else {
                    prevElVec.push_back(prevPtr);
                }
            }
        }
        replayParts.push_back(ReplayPart(rootArray[i].to_array(), prevElVec));

    }

    auto result = nl::json();
    result["replay"] = replayParts;
    result["config"] = { {"precision", precision }, {"trackedSides", trackedSides}, {"stepsPerTick", stepsPerTick}, {"trackedVehicles", trackedVehicles},
        {"trackedAI", trackedAI}, {"sendingChunkSize", sendingChunkSize}, {"trackShots", trackShots} };

    return result;
}

game_value sendReplay(game_state &gs, SQFPar right_arg) {

    // Yes, I'm scared
#ifndef _DEBUG
    try {
#endif // !_DEBUG

        // Construct JSON Object
        auto obj = nl::json();
        obj["missionName"] = sqf::mission_name();
        obj["date"] = timePointToString(missionStart);

        auto now = std::chrono::system_clock::now();
        obj["duration"] = std::chrono::duration_cast<std::chrono::seconds>(now - missionStart).count();

        obj["worldName"] = sqf::world_name();
        obj["data"] = constructData(right_arg.to_array());

        // Needed only on Windows/NOP on everything else
        pn::initializeNetwork();

        try
        {
            p::URI uri(REPLAY_URL);
            std::string path(uri.getPathAndQuery());

            const pn::Context::Ptr context(new pn::Context(pn::Context::CLIENT_USE, "rootcert.pem"));
            pn::HTTPSClientSession session(uri.getHost(), uri.getPort(), context);

            pn::HTTPRequest request(pn::HTTPRequest::HTTP_POST, path, pn::HTTPMessage::HTTP_1_1);
            pn::HTTPResponse response;

            std::stringstream ss;
            ss << obj;

            request.setKeepAlive(false);
            request.setContentLength(ss.str().size());
            request.setContentType("application/json");

            auto bearerToken = std::string("Bearer ").append(token);

            request.set("Authorization", bearerToken);
            request.set("User-Agent", "grad_replay_intercept/0.1");

            std::ostream& o = session.sendRequest(request);
            o << obj;

            session.receiveResponse(response);
            /*
            std::istream& is =
            std::stringstream responseSStream;
            p::StreamCopier::copyStream(is, responseSStream);
            sqf::diag_log(responseSStream.str());
            */

            std::stringstream responseLog;
            responseLog << "[GRAD] (replay_intercept) INFO: POST Request Status " << response.getStatus() << " Reason " << response.getReason();
            sqf::diag_log(responseLog.str());

            if (response.getStatus() != pn::HTTPResponse::HTTPStatus::HTTP_CREATED) {
                return false;
            }
        }
        catch (p::Exception & ex)
        {
            sqf::diag_log(ex.displayText());
            return false;
        }
        return true;
#ifndef _DEBUG
    }
    catch (std::exception& ex) {
        std::stringstream responseLog;
        responseLog << "[GRAD] (replay_intercept) CRASH: " << ex.what();
        sqf::diag_log(responseLog.str());
        return false;
    }
#endif // !_DEBUG

}

game_value startRecord() {
    missionStart = std::chrono::system_clock::now();
    sqf::diag_log(timePointToString(missionStart));
    return true;
}


void intercept::pre_start() {
    static auto grad_replay_intercept_replay_start = 
        client::host::register_sqf_command("gradReplayInterceptStartRecord", "Called when Recording is started", userFunctionWrapper<startRecord>, game_data_type::BOOL);

    static auto grad_replay_intercept_replay_send =
        client::host::register_sqf_command("gradReplayInterceptSendReplay", "Sends the replay", sendReplay, game_data_type::BOOL, game_data_type::ARRAY);
}

void intercept::post_init() {
    if ((bool)sqf::get_number(sqf::config_entry(sqf::mission_config_file()) >> ("GRAD_replay") >> ("upload"))) {
#ifdef _WIN32
        __SQF(["CBA_loadingScreenDone", { gradReplayInterceptStartRecord }] call CBA_fnc_addEventHandler);
        __SQF(["GRAD_replay_stopped", { gradReplayInterceptSendReplay GRAD_REPLAY_DATABASE }] call CBA_fnc_addEventHandler);
#else
#warning "__SQF() macros skipped, call the commands from inside the mission!"
#endif
    }
}

