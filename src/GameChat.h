#ifndef WUMTTS_GAMECHAT_H
#define WUMTTS_GAMECHAT_H

#include <string>
#include <vector>

typedef unsigned long DWORD;

enum class GameChatType
{
	Normal,
	Team,
	Action,
	Anonymous,
	WhisperTo,
	WhisperFrom,
	System,
};

class GameChat
{
public:
	typedef void(*GameChatCallback)(GameChatType type, const std::string & name, const std::string & text);
	typedef void(*CommandCallback)(const std::string & args);
	typedef std::tuple<std::string, CommandCallback> CommandAndCallback;

	static void install();
	static void print(const std::string & msg);
	static bool isInGame();

	static void registerCommandCallback(std::string command, CommandCallback callback);
	static void registerChatCallback(GameChatCallback callback);

private:
	static void __fastcall hookChatHandler(void *This, int EDX, void *MsgObject);
	static int __stdcall hookOnChatInput(void * a1, char *a2, int a3);

	static inline std::vector<CommandAndCallback> commandCallbacks;
	static inline std::vector<GameChatCallback> chatCallbacks;
};

#endif //WUMTTS_GAMECHAT_H
