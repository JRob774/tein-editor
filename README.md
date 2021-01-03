# TEIN Editor

An unofficial editor for the game [The End is Nigh](https://store.steampowered.com/app/583470/The_End_Is_Nigh/),
by Edmund McMillen and Tyler Glaiel. This tool is designed to help in the creation
of mods and custom content for The End is Nigh; providing facilities to pack/unpack
the game's data, create new levels, and place them within the world.

## Building

Simply run the `msvc/build.bat` file in order to build the application.
The script supports Visual Studio. By commenting out certain lines in the file
`msvc/config.bat` you can control which type of build to compile (Debug/Release x86/x64).
C++17 is required and all of the external dependencies are provided with the repo.
