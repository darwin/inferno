// cgcw.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <lmerr.h>
#include <direct.h>

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

void DisplayErrorText(DWORD dwLastError)
{
  HMODULE hModule = NULL; // default to system source
  LPSTR MessageBuffer;
  DWORD dwBufferLength;

  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_IGNORE_INSERTS |
    FORMAT_MESSAGE_FROM_SYSTEM ;

  //
  // If dwLastError is in the network range, 
  //  load the message source.
  //

  if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
    hModule = LoadLibraryEx(
      TEXT("netmsg.dll"),
      NULL,
      LOAD_LIBRARY_AS_DATAFILE
      );

    if(hModule != NULL)
      dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }

  //
  // Call FormatMessage() to allow for message 
  //  text to be acquired from the system 
  //  or from the supplied module handle.
  //

  if(dwBufferLength = FormatMessageA(
    dwFormatFlags,
    hModule, // module to get message from (NULL == system)
    dwLastError,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
    (LPSTR) &MessageBuffer,
    0,
    NULL
    ))
  {
    DWORD dwBytesWritten;

    //
    // Output message string on stderr.
    //
    WriteFile(
      GetStdHandle(STD_ERROR_HANDLE),
      MessageBuffer,
      dwBufferLength,
      &dwBytesWritten,
      NULL
      );

    OutputDebugString(MessageBuffer);
    //
    // Free the buffer allocated by the system.
    //
    LocalFree(MessageBuffer);
  }

  //
  // If we loaded a message source, unload it.
  //
  if(hModule != NULL)
    FreeLibrary(hModule);
}

int _tmain(int argc, _TCHAR* argv[])
{
  //char* p = getcwd(NULL, 0);
  //printf("dir %s", p);
  int i;
  INI::registry ini("cgcw.ini", true, false);

  i = 1;
  int state = 0;
  char* input = NULL;
  while (i<argc)
  {
    if (state==1)
    {
      state = 0;
    }

    if (state==2)
    {
      state = 1;
    }

    if (state==0)
    {
      if (argv[i][0] == '-')
      {
        state = 2;
      }
      else
      {
        input = argv[i];
        break;
      }
    }
    i++;
  }

  i = 1;
  char* output = NULL;
  int output_pos = -1;
  while (i<argc)
  {
    if (strcmpi(argv[i], "-o")==0)
    {
      output = argv[i+1];
      output_pos = i+1;
    }
    i++;
  }

  BOOL bRet = FALSE;
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  // Make child process use this app's standard files.
  si.cb = sizeof(si);
  si.dwFlags    = STARTF_USESTDHANDLES;
  si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
  si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  si.hStdError  = GetStdHandle(STD_ERROR_HANDLE);

  char buf[10*1024] = "cgc.exe";

  std::string sCGCPath = ini["settings"]["cgc path"];
  if (sCGCPath.size())
    _snprintf(buf, 10*1024, "%s", sCGCPath.c_str());
  i = 1;
  while (i<argc)
  {
    if (i==output_pos)
    {
      char* s = strdup(buf);
      _snprintf(buf, 10*1024, "%s %s.cgc", s, argv[i]);
      free(s);
    }
    else
    {
      char* s = strdup(buf);
      _snprintf(buf, 10*1024, "%s %s", s, argv[i]);
      free(s);
    }
    i++;
  }

  //printf("executing:%s", buf);
  bRet = CreateProcess(NULL, buf,
    NULL, NULL,
    TRUE, 0,
    NULL, NULL,
    &si, &pi
    );

  if (bRet)
  {
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // process "delete empty"
    bool bDeleted = false;
    bool bDE = ini["settings"]["delete empty"];
    if (output && bDE)
    {
      _snprintf(buf, 10*1024, "%s.cgc", output);
      FILE* f = fopen(buf, "rb");
      fseek(f, 0, SEEK_END);
      long s = ftell(f);
      fclose(f);
      
      if (s==0) // empty file
      {
        // delete output file
        unlink(buf);
        bDeleted = true;
      }
    }

    // process "filter"
    // TODO:

    // copy from output do dest and process "exclude"
    bool bExclude = false;
    INI::section& sec = ini["exclude"];
    INI::section::iterator si = sec.begin();
    while (si!=sec.end())
    {
      printf(((string)(*si).second).c_str());
      printf(" -> %s\n", input);
      if (stricmp(((string)(*si).second).c_str(), input)==0)
      {
        bExclude = true;
        break;
      }
      si++;
    }
    
    if (!bDeleted)    
    {
      if (!bExclude)
      {
        printf("%s: including!\n", input);
        _snprintf(buf, 10*1024, "%s.cgc", output);
        FILE* fi = fopen(buf, "rb");
        FILE* fo = fopen(output, "wb");
        unsigned char d;
        while (fread(&d, 1, 1, fi))
        {
          fwrite(&d, 1, 1, fo);
        }
        fclose(fo);
        fclose(fi);
      }
      else
      {
        printf("%s: excluding!\n", input);
        // touch original file
        FILE* f = fopen(output, "r+b");
        fseek(f, 0, SEEK_SET);
        int c = fgetc(f);
        fseek(f, 0, SEEK_SET);
        fputc(c, f);
        fclose(f);
      }

      if (ini["settings"]["delete temporaries"])
      {
        _snprintf(buf, 10*1024, "%s.cgc", output);
        // delete output file
        unlink(buf);
      }
    }
  }
  else
  {
    DisplayErrorText(GetLastError());
  }
  return 0;
}