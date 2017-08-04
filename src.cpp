#include "ParseCommand.h"
#include <string>
#include <Windows.h>
#include "BalanceTree.h"

const char* Prompt = ">>>";
extern void AddAllTokens();

void RunParseCmd()
{
	CParseCommand parseCmd;
	while (!parseCmd.m_bQuit)
	{
		std::string strInput;
		std::cout << Prompt;
		getline(std::cin, strInput);
		if (!parseCmd.ParseInput(strInput.c_str()))
			std::cout << parseCmd.GetErrorMsg() << std::endl;
	}
}

int main()
{
	std::cout << "Enter [help] for a list of commands." << std::endl;
	AddAllTokens();
	RunParseCmd();
}