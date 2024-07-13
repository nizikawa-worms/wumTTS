#include <Windows.h>
#include <exception>
#include <string>
#include <vector>
#include <array>
#include <map>
#include <tinyformat.h>

#include "Speaker.h"
#include "GameChat.h"
#include "Debug.h"

bool useDebugPrint = false;
bool readPlayerName = false;
bool readPlayerChat = false;
bool readSystemMessages = false;
bool speaking = true;
std::vector<Speaker *> speakers;
std::thread loaderThread;

unsigned short crc16(const unsigned char *data_p, unsigned char length) {
	// From https://stackoverflow.com/a/23726131
	unsigned char x;
	unsigned short crc = 0xFFFF;

	while (length--) {
		x = crc >> 8 ^ *data_p++;
		x ^= x >> 4;
		crc = (crc << 8) ^ ((unsigned short) (x << 12)) ^ ((unsigned short) (x << 5)) ^ ((unsigned short) x);
	}
	return crc;
}

void printChat(const std::string &msg) {
	debugf("%s\n", msg.c_str());
	// GameChat::print(msg);
}

void speak(const std::string &name, const std::string &text) {
	if (!speaking) {
		return;
	}
	if (loaderThread.joinable()) {
		loaderThread.join();
	}
	Speaker *chosen = nullptr;
	for (Speaker *speaker: speakers) {
		if (speaker->name == name) {
			chosen = speaker;
			break;
		} else if (!chosen && !speaker->isBusy()) {
			chosen = speaker;
		}
	}
	if (chosen) {
		int voiceIndex = 0;
		if (!name.empty()) {
			// Select a voice based on the name
			const unsigned short hash = crc16((const unsigned char *) (name.c_str()), name.length());
			voiceIndex = hash % Speaker::getNumVoices();
		}
		chosen->name = name;
		chosen->say(text, voiceIndex);
	}
}

std::map<std::string, std::string> playermap;

void speakGameChatMessage(GameChatType type, const std::string &name, const std::string &text) {
	if(playermap.contains(name)) {
		speak(playermap[name], readPlayerName ? tfm::format("%s says: %s", name, text) : text);
	} else {
		speak(name, readPlayerName ? tfm::format("%s says: %s", name, text) : text);
	}

}

void shutUp(const std::string &args) {
	for (Speaker *speaker: speakers) {
		speaker->shutUp();
	}
}

void setVolume(const std::string &args) {
	try {
		const int volume = std::clamp(std::stoi(args), 0, 100);
		for (Speaker *speaker: speakers) {
			speaker->setVolume(volume / 100.f);
		}
		printChat(tfm::format("Text to speech volume set to %d%%.", volume));
	} catch (std::exception) {
		speaking = !speaking;
		if (speaking) {
			printChat("Text to speech enabled.");
		} else {
			for (Speaker *speaker: speakers) {
				speaker->shutUp();
			}
			printChat("Text to speech disabled.");
		}
	}
}

std::map<std::string, std::string> readPlayermapSection(const std::string &filePath) {
	// credit: chatgpt
	std::map<std::string, std::string> playermap;
	const std::string section = "Playermap";
	const DWORD bufferSize = 4096;
	char keyBuffer[bufferSize] = {0};
	char valueBuffer[bufferSize] = {0};

	// Get all keys in the section
	DWORD keysRead = GetPrivateProfileStringA(section.c_str(), nullptr, "", keyBuffer, bufferSize, filePath.c_str());
	if (keysRead == 0) {
		debugf("Failed to read keys from section: %s\n", section.c_str());
		return playermap;
	}

	char *keyPtr = keyBuffer;
	while (*keyPtr) {
		std::string key = keyPtr;

		// Get the value for the key
		DWORD valueRead = GetPrivateProfileStringA(section.c_str(), key.c_str(), "", valueBuffer, bufferSize, filePath.c_str());
		if (valueRead == 0) {
			debugf("Failed to read value for key: : %s\n", key.c_str());
			keyPtr += (key.length() + 1);
			continue;
		}

		std::string value = valueBuffer;
		playermap[key] = value;

		// Move to the next key
		keyPtr += (key.length() + 1);
	}

	return playermap;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			try {
				auto start = std::chrono::high_resolution_clock::now();
				decltype(start) finish;

				const char iniPath[] = ".\\wumTTS.ini";
				const int moduleEnabled = GetPrivateProfileIntA("General", "EnableModule", 1, iniPath);
				if (!moduleEnabled) {
					return TRUE;
				}
				useDebugPrint = GetPrivateProfileIntA("General", "UseDebugPrint", 0, iniPath);
				speaking = GetPrivateProfileIntA("TTS", "StartEnabled", 0, iniPath);
				readPlayerName = GetPrivateProfileIntA("TTS", "ReadPlayerName", 0, iniPath);
				readPlayerChat = GetPrivateProfileIntA("TTS", "ReadPlayerChat", 1, iniPath);
				readSystemMessages = GetPrivateProfileIntA("TTS", "ReadSystemMessages", 0, iniPath);

				playermap = readPlayermapSection(iniPath);

				GameChat::install();
				GameChat::registerChatCallback(&speakGameChatMessage);
				GameChat::registerCommandCallback("shutup", &shutUp);
				GameChat::registerCommandCallback("tts", &setVolume);

				loaderThread = std::thread([=]() {
					try {
						const int maxVoices = 50;
						char voicePath[MAX_PATH];
						for (int i = 0; i < maxVoices; i++) {
							GetPrivateProfileStringA("TTS", tfm::format("Voice%d", i).c_str(), "", (LPSTR) &voicePath, sizeof(voicePath), iniPath);
							if (strlen(voicePath)) {
								Speaker::loadVoice(voicePath);
							}
						}

						const float volume = GetPrivateProfileIntA("TTS", "Volume", 100, iniPath) / 100.f;
						const int maxSpeakers = 8;
						for (int i = 0; i < maxSpeakers; i++) {
							speakers.push_back(new Speaker(volume));
						}
					} catch (std::exception &e) {
						MessageBoxA(0, e.what(), PROJECT_NAME " " PROJECT_VERSION " (" __DATE__ ")", MB_ICONERROR);
					}
				});
				loaderThread.detach();

				finish = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed = finish - start;
				debugf("wumTTS startup took %lf seconds\n", elapsed.count());
			} catch (const std::exception &e) {
				MessageBoxA(0, e.what(), PROJECT_NAME " " PROJECT_VERSION " (" __DATE__ ")", MB_ICONERROR);
			}
			break;
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		default:
			break;
	}
	return TRUE;
}
