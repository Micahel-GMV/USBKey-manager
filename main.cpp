// USBKEY.cpp: определяет точку входа для консольного приложения.
//

#include <stdio.h>
#include <stdlib.h>//Metsys
#include <string.h>
#include <fstream>
#include <iostream>
#include <termios.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

//#define _CRT_SECURE_NO_WARNINGS = true

const int KeyLength = 32;
const char * logFileName = "log.txt";//CONSOLE - to screen, NOLOG - no loging
typedef uint8_t KeyArrayType[KeyLength];
const int MAXSTRLEN = 2048;
typedef char FilePathType[MAXSTRLEN];

bool StringAssignChecked(FilePathType target, char* source)
{
    bool result = false;
    if (strlen(source)<=MAXSTRLEN)
    {
        target = source;
        result = true;
    }
    return result;
}

void WriteLog(char * strEvent)
{
    if (logFileName!="NOLOG")
        if (logFileName == "CONSOLE")
        {
            printf("LOG: ");
            printf(strEvent);
            printf("\n");
        }
        else
        {
            FILE * file;
            char timestring[1024];
            file = fopen(logFileName,"a");
            time_t seconds = time(NULL);
            tm* timeinfo = localtime(&seconds);
            strftime(timestring, 80,"%y/%m/%d %H:%M:%S : ", timeinfo);
            fprintf(file, timestring);
            fprintf(file, strEvent);
            fprintf(file, "\n");
        }
}

// Считывание ключа с консоли *******************************************************************************
static struct termios termiosOld, termiosNew;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &termiosOld); /* grab old terminal i/o settings */
  termiosNew = termiosOld; /* make new settings same as old settings */
  termiosNew.c_lflag &= ~ICANON; /* disable buffered i/o */
  termiosNew.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &termiosNew); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &termiosOld);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch()
{
  return getch_(0);
}

/* Read 1 character with echo */
char getche()
{
  return getch_(1);
}

int Get4BitCon(bool consoleOut)
{
	int i = -1;
	char c;
	while (i<0)
	{
		c = getch();
		switch (c)
		{
		case '0': i = 0;
			break;
		case '1': i = 1;
			break;
		case '2': i = 2;
			break;
		case '3': i = 3;
			break;
		case '4': i = 4;
			break;
		case '5': i = 5;
			break;
		case '6': i = 6;
			break;
		case '7': i = 7;
			break;
		case '8': i = 8;
			break;
		case '9': i = 9;
			break;
		case 'a': i = 10;
			break;
		case 'b': i = 11;
			break;
		case 'c': i = 12;
			break;
		case 'd': i = 13;
			break;
		case 'e': i = 14;
			break;
		case 'f': i = 15;
			break;
		case 'A': i = 10;
			break;
		case 'B': i = 11;
			break;
		case 'C': i = 12;
			break;
		case 'D': i = 13;
			break;
		case 'E': i = 14;
			break;
		case 'F': i = 15;
			break;
		default: if(consoleOut) putchar('\a');

		}

	}

	if (consoleOut) putchar(c);
	return i;
}

int Get8BitCon(bool consoleOut)
{
	int i = 0;
	i = (Get4BitCon(consoleOut) << 4) + Get4BitCon(consoleOut);
	return i;
}

int FillArrayConsole(KeyArrayType arr)
{
	printf("\n Please enter key byte-by-byte in HEX.");
	for (int i = 0; i < KeyLength; i++)
	{
		printf("\n Please enter byte number %d ", i);
		arr[i] = Get8BitCon(true);
	}
	return 1;
}

// Считывание ключа с USB *******************************************************************************
bool FileOk(FilePathType fname)
{
    WriteLog("In FileOk.");
    bool result = false;
    int KeySize = KeyLength * sizeof(uint8_t);
    FILE * fptr;
    fptr = fopen(fname,"r");
    int i = 0;
    while((fgetc(fptr) != EOF)||(i<=KeySize))
        { i++; }
    if (i==(KeySize+1)) result = true;
    char text[2048];
    sprintf (text, "Size of file is %i",i);
    WriteLog(text);
    return result;
}

int ReadArrayFile(KeyArrayType arr, FilePathType filenameFull)
{
    WriteLog("In ReadArrayFile.");

	int result = -1;

	FILE *file;
	file = fopen(filenameFull, "rb");

	if (file == NULL)
	{
        WriteLog("Error reading key file:");
        WriteLog(filenameFull);
	}
	else
	{
        if (!FileOk(filenameFull)) WriteLog("Key file size is wrong.");
        else
        {
            WriteLog("File opened Ok.");
            fread(arr,sizeof(uint8_t),KeyLength,file);
            fclose(file);
            result = 0;
        }
	}

	return result;
}

int sel(const struct dirent * d)
{ return 1; // всегда подтверждаем
}


bool FileExists(FilePathType fname)
{
    WriteLog("In FileExists.");
    bool result = false;
    int fileDescr = open(fname, O_RDONLY);
    if (fileDescr>0)
    {
        result = true;
        close(fileDescr);

    }
    return result;
}

int GetFullFileName(char *fileName, char *volumename, FilePathType& fileNameFull)//
{
    WriteLog("In GetFullFileName.");
    FilePathType finalpath;
	int result = 0;
	int i, n;
    struct dirent **entry;
    FilePathType startdir = "/media";
    n = scandir(startdir, &entry, sel, alphasort);
    if (n < 0)
    {
        WriteLog("Error reading directory.\n");
        return 1;
    }
    else
    {

        for (i = 0; i < n; i++)
        {
            FilePathType singledot = ".";
            FilePathType currentdir = "";
            strcpy(currentdir , entry[i]->d_name);
            if ((strcmp(currentdir, singledot) != 0 )&&( (strcmp(currentdir,"..")) !=0 ))
            {
                strcpy(finalpath,"");
                strcpy(finalpath, startdir);
                strcat(finalpath, "/");
                strcat(finalpath, currentdir);
                strcat(finalpath, "/");
                strcat(finalpath, volumename);
                strcat(finalpath, "/");
                strcat(finalpath, fileName);
                if (FileExists(finalpath))
                {
                    if (result == 0)
                    {
                        result = 1;
                        WriteLog("Key file found!");
                        strcpy(fileNameFull, finalpath);
                        WriteLog(finalpath);
                    }
                    else
                    {
                        WriteLog("Number of key files more than one!");
                        result = -1;
                    }
                } else WriteLog("Key file not found! \n");
            }
        }
    }
	return result;
}

int FillArrayUSB(KeyArrayType arr, char *fileName, char *volumeName)
{
    WriteLog("In FillArrayUSB.");
	int result = -1;
	FilePathType fileNameFull;
	if (GetFullFileName(fileName, volumeName, fileNameFull) == 1)
        result = ReadArrayFile(arr, fileNameFull);
	return result;
}

// Общий кусок ***************************************************************
int DeliverKey(KeyArrayType KeyArray, char* volumename, char* filename)		//В массив KeyArray считывает 32 байта ключа, сама функция возвращает -1 - ошибка, 1 - успех
																				//filename - имя файла, содержащего ключ, volumename - имя устройства, содержащего ключ "console" и там, и там - вводим
																				//ключ с клавиатуры байт за байтом в десятичном формате

{
    WriteLog("In DeliverKey.");
	int result = 0;
	if ((filename == "console")&&(volumename == "console")) result = FillArrayConsole(KeyArray);
	else result = FillArrayUSB(KeyArray, filename, volumename);
	return result;
}
// Создаем файл ключа *********************************************************
int CreateKeyFile(KeyArrayType KeyArray, FilePathType filename)
{
    WriteLog("In CreateKeyFile.");
    int result = -1;
    FILE * fout;
    fout = fopen(filename, "wb");
    if (fout!=NULL)
    {
        fwrite(KeyArray,sizeof(uint8_t),KeyLength,fout);
        fclose(fout);
        result = 1;
    }
    else WriteLog("ERROR! Can`t open file for write.");
    return result;
}


int main()//работа с файлами
{
    WriteLog("Program started.");
	KeyArrayType TestArray;
	int result;
	result = DeliverKey(TestArray, "USBKEY_STORAGE", "key.bin");
	//result = DeliverKey(TestArray, "console", "console");
	char text[1024];
	sprintf(text,"Result of reading attempt:%i .", result);
	WriteLog(text);

	printf("Key in DEC:");
	for (unsigned int i = 0; i<KeyLength; i++) printf("%02u ", TestArray[i]);
	printf("\n");

	printf("Key in HEX:");
	for (unsigned int i = 0; i<KeyLength; i++) printf("%02X ", TestArray[i]);
	printf("\n");

	/*if (result > 0)
	{
        result = CreateKeyFile(TestArray,"key.bin");
        printf("Result of write attempt:%i\n", result);
    }*/
    WriteLog("Program finished.");
    return result;
}
