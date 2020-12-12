#include "script_component.hpp"

class CfgPatches {
    class grad_replay_intercept_main {
        name = "Gruppe Adler Replay Intercept Client";
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.92;
        requiredAddons[] = {"intercept_core"};
        authors[] = { "Willard" };
        url = "https://github.com/gruppe-adler/grad_replay_intercept";
        VERSION_CONFIG;
    };
};
class Intercept {
    class grad {
        class grad_replay_intercept {
            pluginName = "grad_replay_intercept";
        };
    };
};
