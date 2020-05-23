# Gruppe Adler Replay Intercept

![Build Status](https://github.com/gruppe-adler/grad_replay_intercept/workflows/C/C++%20CI/badge.svg?branch=master)

---

Gruppe Adler Replay Intercept (grad_replay_intercept) is an Arma 3 Modification built on [intercept](https://github.com/intercept/intercept), which sends replay data recorded by [grad-replay](https://github.com/gruppe-adler/grad-replay) to [replay.gruppe-adler.de](https://github.com/gruppe-adler/replay.gruppe-adler.de).

## Installation

Download the latest version of the mod from our [releases page](https://github.com/gruppe-adler/grad_replay_intercept/releases). The only dependency, **which has to be loaded as well** is [intercept](https://steamcommunity.com/sharedfiles/filedetails/?id=1645973522). Additionally, [grad-replay](https://github.com/gruppe-adler/grad-replay) is required in order to record the replay data.

## Configuration

### Config File

The config file `grad_replay_intercept_config.ini` is located in the Arma 3 root directory, if it is not present on startup, it will be automatically generated. It consists of two entries:

* `Config.ReplayUrl` which holds the URL to the [replay.gruppe-adler.de](https://github.com/gruppe-adler/replay.gruppe-adler.de) API
* `Config.BearerToken` which holds the authorization token
  
Example:

```ini
[Config]
ReplayUrl=https://replay.gruppe-adler.de/
BearerToken=YourBearerTokenHere
```

### Mission Configuration

In order to activate the mod for a mission, `upload` has to be added to the `GRAD_Replay` class in the `description.ext`:

```cpp
class GRAD_Replay {
    [...]
    upload = 1;         // defines if replay data will be uploaded to replay.gruppe-adler.de (0/1)
};
```

## Data Structure

grad_replay_intercept will assemble and serialize the original replay data. Additionally, it will add the config from `grad-replay` as well as the date, duration, mission name and world name.

Original replay data:

```jsonc
[
    [
        ["B_Soldier_F",0,[14701,16740.6],52,"Dylan Lee"," (Alpha 1-1)"],
        12.0022
    ],
    [
        [nil,nil,[14716.6,16756.1],50,nil,nil],
        12.0025
    ],
    [
        [nil,nil,[14733.7,16773.6],42,nil,nil],
        ["B_Soldier_F",0,[14679.1,16692.2],71,"Oscar Carter"," (Alpha 1-2)"],
        12.003
    ],
    [
        [nil,nil,[14751.5,16791.5],44,nil,nil],
        [nil,nil,[14700.3,16692.2],83,nil,nil],
        ["B_Soldier_F",0,[14660.1,16688.9],134,"Oliver White"," (Alpha 1-3)"],
        12.0038
    ],
    [
        [nil,nil,[14769.4,16809.4],45,nil,nil],
        [nil,nil,[14722.8,16692.4],79,nil,nil],
        [nil,nil,[14677.8,16670.9],135,nil,nil],
        12.0044
    ]
];
```

Resulting JSON from grad_replay_intercept:

```jsonc
{
    "config": {
        "precision": 1, 
        "sendingChunkSize": 10, 
        "stepsPerTick": 1, 
        "trackShots": true, 
        "trackedAI": true, 
        "trackedSides": [
            "west", 
            "east", 
            "civilian"
        ],
        "trackedVehicles": true
    }, 
    "date": "2019-05-23 15:07:30", 
    "duration": 119, 
    "missionName": "replayTest", 
    "data": [
        {
            "data": [
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 52.0, 
                    "group": " (Alpha 1-1)", 
                    "icon": "iconMan", 
                    "name": "Dylan Lee", 
                    "position": [
                        14701.0, 
                        16740.599609375
                    ]
                }
            ], 
            "time": "12:00:07"
        }, 
        {
            "data": [
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 50.0, 
                    "group": " (Alpha 1-1)", 
                    "icon": "iconMan", 
                    "name": "Dylan Lee", 
                    "position": [
                        14716.599609375, 
                        16756.099609375
                    ]
                }
            ], 
            "time": "12:00:08"
        }, 
        {
            "data": [
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 42.0, 
                    "group": " (Alpha 1-1)", 
                    "icon": "iconMan", 
                    "name": "Dylan Lee", 
                    "position": [
                        14733.7001953125, 
                        16773.599609375
                    ]
                }, 
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 71.0, 
                    "group": " (Alpha 1-2)", 
                    "icon": "iconMan", 
                    "name": "Oscar Carter", 
                    "position": [
                        14679.099609375, 
                        16692.19921875
                    ]
                }
            ], 
            "time": "12:00:10"
        }, 
        {
            "data": [
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 44.0, 
                    "group": " (Alpha 1-1)", 
                    "icon": "iconMan", 
                    "name": "Dylan Lee", 
                    "position": [
                        14751.5, 
                        16791.5
                    ]
                }, 
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 83.0, 
                    "group": " (Alpha 1-2)", 
                    "icon": "iconMan", 
                    "name": "Oscar Carter", 
                    "position": [
                        14700.2998046875, 
                        16692.19921875
                    ]
                }, 
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 134.0, 
                    "group": " (Alpha 1-3)", 
                    "icon": "iconMan", 
                    "name": "Oliver White", 
                    "position": [
                        14660.099609375, 
                        16688.900390625
                    ]
                }
            ], 
            "time": "12:00:13"
        }, 
        {
            "data": [
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 45.0, 
                    "group": " (Alpha 1-1)", 
                    "icon": "iconMan", 
                    "name": "Dylan Lee", 
                    "position": [
                        14769.400390625, 
                        16809.400390625
                    ]
                }, 
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 79.0, 
                    "group": " (Alpha 1-2)", 
                    "icon": "iconMan", 
                    "name": "Oscar Carter", 
                    "position": [
                        14722.7998046875, 
                        16692.400390625
                    ]
                }, 
                {
                    "color": "rgba(0,76,153,1)", 
                    "direction": 135.0, 
                    "group": " (Alpha 1-3)", 
                    "icon": "iconMan", 
                    "name": "Oliver White", 
                    "position": [
                        14677.7998046875, 
                        16670.900390625
                    ]
                }
            ], 
            "time": "12:00:15"
        }
    ]
    "worldName": "stratis"
}
```
To find out more about the replay data structure check out [grad-replay#20](https://github.com/gruppe-adler/grad-replay/pull/20)
