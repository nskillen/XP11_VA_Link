This is the plugin for VoiceAttack. It's written in C#, and is very likely windows-only at this time, since I didn't worry at all about limiting myself to .Net Core.

## Helper Commands

The file XP11_VA_Link.vap is a profile with a few pre-defined helper (non-voice) commands. They are described in the following table. Any required variables are indicated by an asterisk (`*`).

| Command | Expected Variables | Description |
| ------- | ------------------ | ----------- |
| Begin Command | | |
| End Command | | |
| Execute Command Once | | |
| Get Dataref | | |
| Get Log Level | | |
| Hold Command | | |
| Set Dataref | | |
| Set Log Level | | |

## Defining Voice Commands

Here are some basic examples of how you might define some voice commands for controlling X-Plane. Note that they assume the use of the helper commands detailed above.

Here is what a command that gets your current landing gear state might look like:

```
When I say: "[Landing;]gear status"
When this command executes, do the following sequence:
    Set Text [datarefName] to 'sim/cockpit/switches/gear_handle_status'
    Execute command, 'Get Dataref' (and wait until it completes)
    Begin Integer Compare: [datarefValue] Equals 0
        Say, 'Gear is up'
    Else
        Say, 'Gear is down'
    End Condition
```

You can see here that the only required parameter for getting a dataref is the name of the dataref to fetch.
Based on the dataref type, the appropriate type of VoiceAttack variable will be set.

Here is what a command that lowers your landing gear might look like:

```
When I say: "[Landing;]gear down"
When this command executes, do the following sequence:
    Set integer [datarefType] value to 1
    Set integer [datarefValue] value to 1
    Set Text [datarefName] value to 'sim/cockpit/switches/gear_handle_status'
    Execute external plugin, 'X-Plane 11 / VoiceAttack Link'
        (Plugin Context: SetDataref)
    Say, 'Landing gear down'
```

You can see in this command the three values that must be set when changing a dataref in X-Plane.

`datarefName` - the name of the dataref to be changed
`datarefType` - the type of the dataref in X-Plane
`datarefValue` - the value to set the dataref to.

Note that you do have to specify the type, as VoiceAttack has different getter/setter functions for variables based on the type, and also X-Plane supports both `double` and `float` datarefs, where VoiceAttack only has `decimal`, so the `datarefType` is used for both determining which getter function to call, and for disambiguation when setting the dataref.

Another gotcha to watch out for is that VoiceAttack, by default, doesn't wait for the plugin to finish executing, and simply proceeds on to the next part of the command. This is usually not what you want, so don't forget to theck the box near the bottom of the `Execute external plugin` window labelled `Wait for the plugin function to finish before continuing`
## Technical Details

When the plugin is called, it uses the `vaProxy.Context` variable to determine what action to take. The main requests will be to `GetDataref` and `SetDataref`. The former allows you to read the state of a dataref in X-Plane, while the latter allows you to modify it.

