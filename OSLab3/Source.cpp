#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;

void WriteToCurrentPosition(HANDLE hFile, string result);

bool CountSymThread(string command)
{
	bool isOneOrZeroInput = true;	//�������� ��� ������ ��������������� ������ ����� ����������� �� ������ 1 ����
	bool isOneOrZeroOutput = true;	//�������� ��� ������ ��������������� ������ ������ ����������� �� ������ 1 ����
	int countIn = 0;	//���-�� >
	int countOut = 0;	//���-�� <
	int countQuotes = 0;
	//�������� �� ����������� < >
	for (int i = 0; i < command.length(); i++)
	{
		if (command[i] == '>')
		{
			countOut++;
			if (countOut > 1)
			{
				isOneOrZeroOutput = false;
				break;
			}
		}
		else if (command[i] == '<')
		{
			countIn++;
			if (countIn > 1)
			{
				isOneOrZeroInput = false;
				break;
			}
		}
		else if (countQuotes % 2 == 1)
		{
			return false;
		}
	}
	return (isOneOrZeroInput && isOneOrZeroOutput);
}

vector<string> ParsingString(string command)
{
	vector<string> result;
	bool isWhiteSpace = true;	//��������� ��������������� ������ - ������
	int begin = 0;	//������ ������ ��� ���������� ����������
	int end = 0;	//������ ������ ��� ���������� ����������
	//�������� ������ �� �����
	for (int i = 0; i < command.length(); i++)
	{
		if (command[i] == ' ')
		{
			if (!isWhiteSpace)	//���� ������ ��� �������, � ������ �����������
			{
				end = i - 1;
				result.push_back(command.substr(begin, end - begin + 1));
				isWhiteSpace = true;
			}
		}
		else
		{
			if (isWhiteSpace)	//���� ������ ��� �������, � ������ �������� �������
			{
				begin = i;
				isWhiteSpace = false;
			}
			if (i == command.length() - 1)	//��������� ������ ������, ���� �� �� ������, ������ ����� �������� � ��������� ���������
			{
				end = i;
				result.push_back(command.substr(begin, end - begin + 1));
			}
		}
	}
	return result;
}

void AddingWhiteSpaces(string* command)
{
	for (int i = 0; i < (*command).length(); i++)
	{
		if ((*command)[i] == '<' || (*command)[i] == '>')
		{
			(*command).insert(i, 1, ' ');
			(*command).insert(i + 2, 1, ' ');
			i++;
		}
	}
}

bool IsNormArgString(vector<string> processArgs)
{
	//�������� �����������
	for (int i = 0; i < processArgs.size() - 1; i++)
	{
		if ((processArgs[i].compare("<") == 0 && processArgs[i + 1].compare(">") == 0) ||
			(processArgs[i].compare(">") == 0 && processArgs[i + 1].compare("<") == 0))
		{
			cout << "����������� ����������� ��������  '<' '>'" << endl;
			return false;
		}
	}
	//������� �� ������ ���� ��� �������� �����
	if ((processArgs[processArgs.size() - 1].compare("<") == 0 || processArgs[processArgs.size() - 1].compare(">") == 0))
	{
		cout << "�� ������ ����, � ������� ���������������� �����" << endl;
		return false;
	}
	//������ �������� - �������� ���������
	if ((processArgs[0].compare("<") == 0 || processArgs[0].compare(">") == 0))
	{
		cout << "������ ��������������� ������ �� ����� ���� ������ ����������." << endl;
		return false;
	}
	return true;
}

int SearchFileForThreads(string* inThread, string* outThread, vector<string>* processArgs)
{

	//���� ����, �� ���� ����� ��� ��������������� �������
	for (int i = 0; i < (*processArgs).size(); i++)
	{
		if ((*processArgs)[i].compare(">") == 0)
		{
			*outThread = (*processArgs)[i + 1];
			(*processArgs).erase((*processArgs).begin() + i, (*processArgs).begin() + i + 2);
			i--;
		}
		if ((*processArgs)[i].compare("<") == 0)
		{
			*inThread = (*processArgs)[i + 1];
			(*processArgs).erase((*processArgs).begin() + i, (*processArgs).begin() + i + 2);
			i--;
		}
	}
	return 0;
}

int PathForCreatingProcess(vector<string> processArgs, string* argsString)
{
	int index = -1;
	for (int i = 0; i < processArgs.size(); i++)
	{
		*argsString += processArgs[i] + " ";
	}


	if ((*argsString)[0] == '"')
	{
		int iter = 0;
		do
		{
			if ((*argsString)[iter] == '\\')
			{
				index = iter;
			}
			iter++;
		} while ((*argsString)[iter] != '"');

	}
	else
	{
		index = processArgs[0].rfind("\\");
	}
	return index;
}

int OwnFunction(int argc, char* argv[])
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	HANDLE hIn, hOut;
	int lineCount;
	int* lineNumbers;
	char* inputFileName, * outputFileName;

	if (argc != 7) {
		printf_s("������! ������� �������� ���-�� ����������!\n");
		return 1;
	}

	lineCount = atoi(argv[1]);
	lineNumbers = (int*)malloc(sizeof(int) * lineCount);

	for (int i = 0; i < 3; i++)
		lineNumbers[i] = atoi(argv[i + 2]);

	inputFileName = argv[5];
	outputFileName = argv[6];

	hIn = CreateFileA(inputFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hOut = CreateFileA(outputFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	char currentByte;
	int currentLineNumber = 1;
	DWORD numberOfBytes;
	string result;

	while (ReadFile(hIn, &currentByte, 1, &numberOfBytes, NULL) > 0 && numberOfBytes > 0) {
		for (int i = 0; i < 3; i++) {
			if (lineNumbers[i] == currentLineNumber) {
				result += currentByte;
				break;
			}
		}

		if (currentByte == 10) {
			currentLineNumber++;

			if (result.size()) {
				WriteToCurrentPosition(hOut, result);
				printf_s("�������� ������: ");
				puts(result.c_str());

				result.clear();
			}
		}
	}

	BOOL res = SetFileAttributesA(outputFileName, FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN);

	CloseHandle(hIn);
	CloseHandle(hOut);

	free(lineNumbers);
	return 0;
}



int main()
{
	setlocale(LC_ALL, "Russian");
	while (true)
	{
		cout << ">>> ";
		string command;
		getline(cin, command);
		//�������� ������
		if (command.compare("exit") == 0)
		{
			return 0;
		}
		//�������� ����� ��������� ������
		if (command.length() > 0)
		{
			//��� �������� ����� � ������ �� <> �������� �������
			if (CountSymThread(command))	//���� � ��������� ��������������� ��� ��������� (���-��)
			{
				AddingWhiteSpaces(&command);	//������� ���������

				vector<string> processArgs = ParsingString(command);	//��� �������� ���������				

				if (IsNormArgString(processArgs))	//���� ��� ���������
				{
					string outThread = "";
					string inThread = "";
					SearchFileForThreads(&inThread, &outThread, &processArgs);	//��� ����� ��� ���������������

					if (processArgs[0].compare("linecopy") == 0)
					{
						vector<char*> cstrings;
						cstrings.reserve(processArgs.size());

						for (size_t i = 0; i < processArgs.size(); ++i)
							cstrings.push_back(const_cast<char*>(processArgs[i].c_str()));
						OwnFunction(processArgs.size(), &cstrings[0]);
					}
					else
					{
						STARTUPINFOA thisProcess;
						PROCESS_INFORMATION ProcessInfo;
						ZeroMemory(&thisProcess, sizeof(thisProcess));
						thisProcess.cb = sizeof(thisProcess);
						ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));

						thisProcess.dwFlags = STARTF_USESTDHANDLES;
						thisProcess.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
						thisProcess.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

						//
						//��������������� ������ ������, ���� �������
						//
						SECURITY_ATTRIBUTES o_attr = { sizeof(o_attr), NULL, TRUE };
						HANDLE outputFile = INVALID_HANDLE_VALUE;

						if (outThread.compare("") != 0)
						{
							outputFile = CreateFileA(outThread.c_str(),
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								&o_attr,
								OPEN_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
							thisProcess.hStdOutput = outputFile;
						}
						//
						//��������������� ������ �����, ���� �������
						//
						SECURITY_ATTRIBUTES i_attr = { sizeof(i_attr), NULL, TRUE };
						HANDLE inputFile = INVALID_HANDLE_VALUE;

						if (inThread.compare("") != 0)
						{
							inputFile = CreateFileA(inThread.c_str(),
								GENERIC_READ | GENERIC_WRITE,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								&i_attr, OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
							thisProcess.hStdInput = inputFile;
						}

						bool processIsStarting;
						string argsString = "";
						string path = "";
						int index = PathForCreatingProcess(processArgs, &argsString);	//�������� ����������

						if (index != -1)
						{
							// ���� ���� ������� - ������� � ����������
							if (argsString[0] != '"') {
								path = argsString.substr(0, index + 1);
							}
							else {
								path = argsString.substr(1, index);
							}
							processIsStarting = CreateProcessA(NULL,
								const_cast<char*>(argsString.c_str()),
								NULL,
								NULL,
								TRUE,
								0,
								NULL,
								path.c_str(),
								&thisProcess,
								&ProcessInfo);
						}
						else
						{
							processIsStarting = CreateProcessA(NULL,
								const_cast<char*>(argsString.c_str()),
								NULL,
								NULL,
								TRUE,
								0,
								NULL,
								NULL,
								&thisProcess,
								&ProcessInfo);
						}

						if (processIsStarting)
						{
							WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
						}
						else
						{
							cout << "������� �� ����������" << endl;
						}
						//
						//�������� �������
						//
						if (inputFile != INVALID_HANDLE_VALUE) CloseHandle(inputFile);
						if (outputFile != INVALID_HANDLE_VALUE) CloseHandle(outputFile);
						CloseHandle(ProcessInfo.hProcess);
						CloseHandle(ProcessInfo.hThread);
					}
				}
			}
			else
			{
				cout << "������� ��������������� ������� ����������� ������ 1 ����" << endl;
			}

		}
	}
	system("pause");
	return 0;
}

void WriteToCurrentPosition(HANDLE hFile, string result) {
	DWORD currentPos = SetFilePointer(hFile, 0, 0, FILE_CURRENT);
	int currentBufferPos = 0;

	string buff = result;

	char currentByte;
	DWORD numberOfBytes;

	while (ReadFile(hFile, &currentByte, 1, &numberOfBytes, NULL) > 0 && numberOfBytes > 0) {
		SetFilePointer(hFile, -1, NULL, FILE_CURRENT);

		WriteFile(hFile, &(buff[currentBufferPos]), 1, NULL, NULL);
		buff[currentBufferPos] = currentByte;

		if (++currentBufferPos == result.size())
			currentBufferPos = 0;
	}

	int endOfFilePos = currentBufferPos;

	for (int i = endOfFilePos; i < result.size(); i++)
		WriteFile(hFile, &(buff[i]), 1, NULL, NULL);
	for (int i = 0; i < endOfFilePos; i++)
		WriteFile(hFile, &(buff[i]), 1, NULL, NULL);

	SetFilePointer(hFile, currentPos + result.size(), NULL, FILE_BEGIN);
}