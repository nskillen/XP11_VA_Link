This is the plugin for X-Plane 11. It communicates with the VoiceAttack plugin to allow the reading and setting of datarefs and commands within X-Plane 11.

This plugin is written in C++, and attempts to be fairly simple.

It is a multi-threaded named pipe server, with one thread for each pipe, plus a connection thread.

The connection thread simply creates a new instance of the pipe server, and then blocks until a client connects. When that happens, a new thread is created for handling the pipe read/write operations, and the connection thread creates another instance of the pipe server and blocks until the next connection.

In each read/write thread, the pipe immediately goes into a blocking read, which will sit there indefinitely until the client sends it some data. Once the data is received, it is parsed and the requested operation is run. Once the requested dataref has been get/set, the result is written back to the pipe, and the thread loops around to the blocking read operation again.

The main plugin thread has a list of all pipes and threads that have been created, so that things can be shut down cleanly when the plugin is terminated.

_In theory_, it should be possible to modify this to work on non-windows systems, so long as it is possible to implement the `Pipe` interface on a non-windows system. Note that the classes implementing `Pipe` don't have to worry about threading at all, that is all handled in the `Link` class that requests `Pipe` instances.

## Pipe syntax

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