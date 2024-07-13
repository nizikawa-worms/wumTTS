#include "GameChat.h"

#include "Hooks.h"
#include "Debug.h"

void (__fastcall *addrChatHandler)(void *This, int EDX, void *MsgObject);

void (__fastcall *origChatHandler)(void *This, int EDX, void *MsgObject);

const char *getMessage(DWORD This) {
	return *(const char **) (This + 20);
}

const char *getFrom(DWORD This) {
	return *(const char **) (This + 12);
}

void __fastcall GameChat::hookChatHandler(void *This, int EDX, void *MsgObject) {
	GameChatType type = GameChatType::Normal;
	const std::string name = getFrom((DWORD) MsgObject);
	const std::string text = getMessage((DWORD) MsgObject);
	for (const GameChatCallback &callback: chatCallbacks) {
		callback(type, name, text);
	}

	origChatHandler(This, EDX, MsgObject);
}

int (__stdcall *origOnChatInput)(void *a1, char *a2, int a3) = nullptr;

int __stdcall GameChat::hookOnChatInput(void *a1, char *msg, int a3) {
	std::string msgs = msg;
	for (const CommandAndCallback &cb: commandCallbacks) {
		if (msgs.starts_with(std::get<0>(cb))) {
			std::get<1>(cb)(msgs.substr(std::get<0>(cb).length()));
			return 1;
		}
	}
	return origOnChatInput(a1, msg, a3);
}

void GameChat::print(const std::string &msg) {
	debugf("%s", msg.c_str());
}

void GameChat::install() {
	addrChatHandler = (void (__fastcall *)(void *, int, void *)) Hooks::scanPatternAndHook("ChatHandler",
	                                                                                       "\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x83\xEC\x24\x80\x3D\x00\x00\x00\x00\x00\x53\x55\x56\x8B\xD9\x57\x89\x5C\x24\x20\x74\x09\xC6\x05\x00\x00\x00\x00\x00\xEB\x09\x8D\x4C\x24\x18\xE8\x00\x00\x00\x00\xB8\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x66\x39\x05\x00\x00\x00\x00\x75\x11\x68\x00\x00\x00\x00\x6A\x2F\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00",
	                                                                                       "???????xx????xxxx????xxxxx?????xxxxxxxxxxxxxx?????xxxxxxx????x????xxx?????xxx????xxx????xxx????x????",
	                                                                                       (DWORD *) &hookChatHandler,
	                                                                                       (DWORD *) &origChatHandler);
	Hooks::scanPatternAndHook("OnChatInput",
	                          "\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x64\x89\x25\x00\x00\x00\x00\x83\xEC\x08\x56\x8B\x74\x24\x20\x57\x56\x8D\x4C\x24\x10\xE8\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\x8D\x4C\x24\x0C\xE8\x00\x00\x00\x00\x8B\x7C\x24\x20\x56\x8B\xCF\xE8\x00\x00\x00\x00\x8B\xF0\x83\xFE\x06\x0F\x87\x00\x00\x00\x00\x0F\xB6\x86\x00\x00\x00\x00\xFF\x24\x85\x00\x00\x00\x00",
	                          "???????xx????xxxx????xxxxxxxxxxxxxxx????xxx?????xxxxx????xxxxxxxx????xxxxxxx????xxx????xxx????",
	                          (DWORD *) &hookOnChatInput,
	                          (DWORD *) &origOnChatInput);
}

bool GameChat::isInGame() {
	return true;
}

void GameChat::registerCommandCallback(std::string command, CommandCallback callback) {
	commandCallbacks.push_back(std::make_tuple("/" + command, callback));
}

void GameChat::registerChatCallback(GameChatCallback callback) {
	chatCallbacks.push_back(callback);
}
