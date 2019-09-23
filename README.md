# X-Plane 11 and VoiceAttack Link plugins

These are a pair of plugins that will allow X-Plane 11 and VoiceAttack to work together.

I have no idea if there already was something free that could do this, but I didn't find anything in my 30 minutes of searching, so I decided to write something.

So far it is capable of getting and setting datarefs in X-Plane, and not crashing too much.

Because VA and XP11 are entirely separate processes, I had to implement an IPC mechanism that would allow me to transfer data between the two plugins. My solution (since I only run these products on Windows), was to use named pipes. Seems to work well enough so far.

## VoiceAttackPlugin

This is the plugin for VoiceAttack. It's written in C#, and is very likely windows-only at this time, since I didn't worry at all about limiting myself to .Net Core.

When the plugin is called, it uses the `vaProxy.Context` variable to determine what action to take. The main requests will be to `GetDataref` and `SetDataref`. The former allows you to read the state of a dataref in X-Plane, while the latter allows you to modify it.

Here is what a command that gets your current landing gear state might look like:

```
When I say: "[Landing;]gear status"
When this command executes, do the following sequence:
    Set Text [datarefName] to 'sim/cockpit/switches/gear_handle_status'
    Execute external plugin, 'X-Plane 11 / VoiceAttack Link'
        (Plugin Context: GetDataref)
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

## X-Plane 11 Plugin

This plugin is written in C++, and attempts to be fairly simple.

It is a multi-threaded named pipe server, with one thread for each pipe, plus a connection thread.

The connection thread simply creates a new instance of the pipe server, and then blocks until a client connects. When that happens, a new thread is created for handling the pipe read/write operations, and the connection thread creates another instance of the pipe server and blocks until the next connection.

In each read/write thread, the pipe immediately goes into a blocking read, which will sit there indefinitely until the client sends it some data. Once the data is received, it is parsed and the requested operation is run. Once the requested dataref has been get/set, the result is written back to the pipe, and the thread loops around to the blocking read operation again.

The main plugin thread has a list of all pipes and threads that have been created, so that things can be shut down cleanly when the plugin is terminated.

_In theory_, it should be possible to modify this to work on non-windows systems, so long as it is possible to implement the `Pipe` interface on a non-windows system. Note that the classes implementing `Pipe` don't have to worry about threading at all, that is all handled in the `Link` class that requests `Pipe` instances.

### Pipe syntax

The syntax for passing messages into the X-Plane 11 plugin is quite simple:

    operation;datarefName[;datarefType;datarefValue]

You may notice a certain correspondence here between this syntax and the required VoiceAttack variables.

`operation` is either `get` or `set`. No other values are supported, and will result in an unrecognized command.
`datarefName` is the full name of the dataref you are requesting, eg `sim/cockpit/switches/gear_handle_status`
`datarefType` is only required for a `set` operation, and specifies the type of the dataref to be set
`datarefValue` is only required for a `set` operation, and specifies the new value of the dataref

If the dataref is one of the array types, then the values should be comma-separated.

For `get` operations, either the datarefName is valid or it is not.

If `datarefName` was valid, then the value written to the pipe will be:

    datarefName;datarefType;datarefValue

Otherwise, the value written to the pipe will be:

    {invalid_dataref}

For `set` operations, the dataref must be writable, the type in `datarefType` must match the actual dataref type in X-Plane, and the value must be of the correct type as well.

If the dataref does not exist, then the set command will write `{invalid_dataref}` to the pipe.

If the dataref is not writable, then the set command will write `{dataref_not_writable}` to the pipe.

If there is a mismatch between the dataref types, then the set command will write `{dataref_type_mismatch}` to the pipe.

For both `get` and `set` operations, if some other otherwise unhandled error occurs, then the plugin will write either `{get_failed}` or `{set_failed}` back to the pipe, as appropriate.

If the command sent is niether `get` nor `set`, then the plugin will write `{invalid_command_<command>}` back to the pipe (eg. If you send `purple;foo;bar`, then you will get back `{invalid_command_purple}`).