# SIM: Speak Instead of Me

SIM is a small program that enables any text to be spoken using any TTS in the OS and output to any audio device.

SIM aims to replace large programs such as [Balabolka](https://www.cross-plus-a.com/balabolka.htm) when you want to speak in TeamTalk or Discord with a SAPI voice, for example, due to microphone issues.

## Features

Features are marked with checkboxes to indicate if a feature is already implemented or not.

- [x] Select any SAPI voice from the list;
- [x] Select any audio device to output speech to it;
- [x] Tune TTS rate and volume (volume is controlled by audio system, not the SAPI engine);
- [ ] Exclude voices which are known to not work with the program (for example Hungarian Profivox or some older SAPI synthesizers);
- [ ] Add UI labels;
- [ ] Keep history of spoken phrases;
- [ ] Clear input text field on enter press and successful speech;
- [ ] Accept command line arguments (to create a personalized shortcut with predefined settings);
- [ ] Display more friendly and understandable SAPI voice names;
- [ ] Modify audio system logic to avoid resampling artifacts;

## Building

### Tools

- CMake 3.25 or newer;
- A working C/C++ compiler (for now tested only with msvc and clang);

### Dependencies

- [Miniaudio](https://github.com/mackron/miniaudio) for audio output (fetched by CMake automatically);
- [SRAL](https://github.com/m1maker/sral) for TTS (fetched by CMake automatically);
- [SPDLog](https://github.com/gabime/spdlog) for logging (fetched by CMake automatically);
- [WXWidgets](https://github.com/wxWidgets/wxWidgets) for user interface (fetched by CMake automatically);

## Development notes

The program aims to be always only one executable file with no extra DLLs.
When contributing, please note that and do not use libraries, which have to be DLL.
