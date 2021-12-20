#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;

void WriteToCurrentPosition(HANDLE hFile, string result);

bool CountSymThread(string command)
{
	bool isOneOrZeroInput = true;	//Проверка что символ перенаправления потока ввода встречается не больше 1 раза
	bool isOneOrZeroOutput = true;	//Проверка что символ перенаправления потока вывода встречается не больше 1 раза
	int countIn = 0;	//Кол-во >
	int countOut = 0;	//Кол-во <
	int countQuotes = 0;
	//Проверка на одинарность < >
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
	bool isWhiteSpace = true;	//Последний просматриваемый символ - пробел
	int begin = 0;	//Индекс начала для разделения параметров
	int end = 0;	//Индекс начала для разделения параметров
	//Разбиваю строку на слова
	for (int i = 0; i < command.length(); i++)
	{
		if (command[i] == ' ')
		{
			if (!isWhiteSpace)	//Если раньше шли символы, а теперь закончились
			{
				end = i - 1;
				result.push_back(command.substr(begin, end - begin + 1));
				isWhiteSpace = true;
			}
		}
		else
		{
			if (isWhiteSpace)	//Если раньше шли пробелы, а теперь начались символы
			{
				begin = i;
				isWhiteSpace = false;
			}
			if (i == command.length() - 1)	//Последний символ строки, если он не пробел, должен стать конечным в последнем аргументе
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
	//Проверка чередования
	for (int i = 0; i < processArgs.size() - 1; i++)
	{
		if ((processArgs[i].compare("<") == 0 && processArgs[i + 1].compare(">") == 0) ||
			(processArgs[i].compare(">") == 0 && processArgs[i + 1].compare("<") == 0))
		{
			cout << "Недопустимо чередование символов  '<' '>'" << endl;
			return false;
		}
	}
	//Символы не должны идти без названия файла
	if ((processArgs[processArgs.size() - 1].compare("<") == 0 || processArgs[processArgs.size() - 1].compare(">") == 0))
	{
		cout << "Не указан файл, в который перенаправляется поток" << endl;
		return false;
	}
	//Первый аргумент - название программы
	if ((processArgs[0].compare("<") == 0 || processArgs[0].compare(">") == 0))
	{
		cout << "Символ перенаправления потока не может идти первым аргументом." << endl;
		return false;
	}
	return true;
}

int SearchFileForThreads(string* inThread, string* outThread, vector<string>* processArgs)
{

	//Если есть, то ищем файлы для перенаправления потоков
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
		printf_s("ОШИБКА! ВВЕДЕНО НЕВЕРНОЕ КОЛ-ВО АРГУМЕНТОВ!\n");
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
				printf_s("ЗАПИСАНА СТРОКА: ");
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
		//Проверка выхода
		if (command.compare("exit") == 0)
		{
			return 0;
		}
		//Проверка длины введенной строки
		if (command.length() > 0)
		{
			//Для удобства слева и справа от <> ставятся пробелы
			if (CountSymThread(command))	//Если с символами перенаправления все нормально (кол-во)
			{
				AddingWhiteSpaces(&command);	//Отделяю пробелами

				vector<string> processArgs = ParsingString(command);	//Тут хранятся аргументы				

				if (IsNormArgString(processArgs))	//Если все нормально
				{
					string outThread = "";
					string inThread = "";
					SearchFileForThreads(&inThread, &outThread, &processArgs);	//Ищу файлы для перенаправления

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
						//Перенаправление потока вывода, если указано
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
						//Перенаправление потока ввода, если указано
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
						int index = PathForCreatingProcess(processArgs, &argsString);	//Проверяю директорию

						if (index != -1)
						{
							// если есть кавычки - убираем у директории
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
							cout << "Процесс не запустился" << endl;
						}
						//
						//Закрытие хэндлов
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
				cout << "Символы перенаправления потоков встречаются больше 1 раза" << endl;
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