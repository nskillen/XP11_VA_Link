# X-Plane 11 and VoiceAttack Link plugins

These are a pair of plugins that will allow X-Plane 11 and VoiceAttack to work together.

I have no idea if there already was something free that could do this, but I didn't find anything in my 30 minutes of searching, so I decided to write something.

So far it is capable of getting and setting datarefs in X-Plane, and not crashing too much.

Because VA and XP11 are entirely separate processes, I had to implement an IPC mechanism that would allow me to transfer data between the two plugins. My solution (since I only run these products on Windows), was to use named pipes. Seems to work well enough so far.

## Building

This project is built in VS2019 on Windows. I make no guarantees about the ability of this project to build on any othe configuration.

Simply open the solution in Visual Studio, and select `Build -> Build Solution` or hit F6 on your keyboard.

## Projects

### VoiceAttack

This is the plugin for VoiceAttack. It's written in C#, and is very likely windows-only at this time, since I didn't worry at all about limiting myself to .Net Core.

### XPlane11

This is the plugin for X-Plane 11. It communicates with the VoiceAttack plugin to allow the reading and setting of datarefs and commands within X-Plane 11.
