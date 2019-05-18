class CfgPatches {
    class grad_replay_intercept {
        name = "Gruppe Adler Replay Intercept Client";
        units[] = {};
        weapons[] = {};
        requiredVersion = 1.82;
        requiredAddons[] = {"intercept_core"};
        author = "Willard";
        authors[] = {"Willard"};
        url = "https://github.com/gruppe-adler";
        version = "1.0";
        versionStr = "1.0";
        versionAr[] = {1,0};
    };
};
class Intercept {
    class Willard {
        class grad_replay_intercept {
            pluginName = "grad_replay_intercept";
        };
    };
};
