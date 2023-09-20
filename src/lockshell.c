// Lockshell Source Code - Copyright Cyuria 2023 (c)
// See LICENSE.txt for more information

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "sha256.h"


/*
 * `kill <pid>` gives an error status code of 130
 * `kill -KILL <pid>` gives an error status code of 137
 * This program will only exit with a code of zero if it successfully completed
 * Otherwise you can suspect that some foul play occured.
 */

#define STDDATA "~/.lockfile/"
#define PWDBIN STDDATA"pwd.bin"

// OS Specific code

static int initTerminal(void);
static char waitch(void);
static void restoreTerm(void);

#if defined(__unix) || defined(__APPLE__)
#define UNIX
#endif
#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#include <windows.h>

static DWORD oldDwMode;
static HANDLE hOut;

static int initTerminal(void)
{
  hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
    return GetLastError();

  DWORD dwMode = 0;
  if (!GetConsoleMode(hOut, &dwMode))
    return GetLastError();

  oldDwMode = dwMode;
  dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

  if (!SetConsoleMode(hOut, dwMode))
    return GetLastError();

  atexit(restoreTerm);
  return 0;
}

static void restoreTerm(void)
{
  SetConsoleMode(hOut, oldDwMode);
}

static char waitch(void)
{
  return _getch();
}

#endif
#ifdef UNIX

#include <termios.h>
#include <unistd.h>

static struct termios old_terminal = {0};

static void restoreTerm(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &old_terminal);
}

static int initTerminal(void)
{
  tcgetattr(STDIN_FILENO, &old_terminal);
  struct termios new_terminal = old_terminal;
  new_terminal.c_lflag &= ~( ICANON | ECHO );
  tcsetattr(STDIN_FILENO, TCSANOW, &new_terminal);

  atexit(restore_terminal);
  return 0;
}

static char waitch(void)
{
  return getchar();
}

#endif


void lockshell(void);

void initTerminalAnsi(void);
void resetTerminalAnsi(void);

void registerSignalIgnores(void);

void printlock(void);


void makeDataDir(void);

void loadPasswordHash(void);
void setPasswordHash(void);

void resizePassword(void);

int checkPassword(void);


static const char lockascii[];
static const int width;

static char* password = NULL;
static int passwordlen = 0;

static unsigned char pwdhash[32];

void initTerminalAnsi(void)
{
  // Register the terminal reset for exit
  atexit(resetTerminalAnsi);
  // Use alternate screen buffer
  fputs("\033[?1049h", stdout);
  // Set the cursor to a blinking vertical bar
  fputs("\033[5SPq", stdout);
}
void resetTerminalAnsi(void)
{
  // Reset the cursor
  fputs("\033[0SPq", stdout);
  // Reset back to original terminal
  fputs("\033[?1049l", stdout);
}

void registerSignalIgnores(void)
{
  // Ignore every signal except SIGHUP
  for (int i = 1; i < 32; ++i)
    signal(i, SIG_IGN);
}

int dataFileExists(void)
{
  struct stat st = { 0 };
  return !stat(PWDBIN, &st);
}
void makeDataDir(void)
{
  struct stat st = { 0 };

  if (stat(STDDATA, &st) == -1)
  {
#ifdef _WIN32
    _mkdir(STDDATA);
#endif
#ifdef UNIX
    mkdir(STDDATA, 0700);
#endif
  }
}

void resizePassword(void)
{
  password = realloc(password, passwordlen);
}

void loadPasswordHash(void)
{
  FILE* hashfile;
  const int err = fopen_s(&hashfile, PWDBIN, "rb");
  if (err)
  {
    printf("Error opening file: %s\n", PWDBIN);
    exit(-1);
  }
  fread(pwdhash, 32, 1, hashfile);
  fclose(hashfile);
}
void setPasswordHash()
{
  FILE* hashfile;
  fopen_s(&hashfile, PWDBIN, "wb");
  fwrite(pwdhash, 32, 1, hashfile);
  fclose(hashfile);
}

int checkPassword(void)
{
  char currenthash[32];
  calc_sha_256((uint8_t*) currenthash, password, passwordlen);
  for (int i = 0; i < 32; ++i)
    if (pwdhash[i] != currenthash[i]) return 0;
  return 1;
}

void lockshell(void)
{
  resizePassword();
  loadPasswordHash();
  // Enable ANSI escape sequences on windows
  {
    const int err = initTerminal();
    if (err) exit(err);
  }
  // Start ignoring signals
  registerSignalIgnores();
  initTerminalAnsi();

  // Print the lock, along with the relevant ansi escape sequnces to clear the
  // screen and move back up to the password row
  fputs(lockascii, stdout);

  // Password handling
  while (1)
  {
    fprintf(stdout, "\033[2K\033[%dG", width - passwordlen);
    for (int i = 0; i < passwordlen; ++i)
      fputs(" *", stdout);
    const char c = waitch();
    if (c == 10 || c == 13) break;
    if (c == 12 || c == 8)
    {
      if (--passwordlen < 0) passwordlen = 0;
      resizePassword();
      continue;
    }
    if (++passwordlen >= width) passwordlen = width - 1;
    resizePassword();
    password[passwordlen-1] = c;
  }


  password = realloc(password, passwordlen + 1);
  password[passwordlen] = '\0';
  printf("\n\n\n\r\nYour password is: %s\n", password);
  if (checkPassword())
    puts("This matches!");
  else
    puts("This does not match");
  waitch();
}

static const int width = 16;
static const char lockascii[] = 
  "\033[H\033[1J"
  "         :+*@@@@@%+=\n"
  "       =@@@#+====*@@@*.\n"
  "    .*@@@.          +@@=\n"
  "    %@@.              %@@.\n"
  "    @@%               =@@-\n"
  "    @@@@@@@@@@@@@@@@@@@@@-\n"
  "    @@@#+%@@@@@@@@@@@@@@@-\n"
  "    @@@@=  `\\%@@@@@@@@@@@-\n"
  "    @@@@@@@>  >@@@@@@@@@@-\n"
  "    @@@%=  ./%@@@@@@@@@@@-\n"
  "    @@@#+@@@@@@________%@-\n"
  "    @@@@@@@@@@@@@@@@@@@@@-\n"
  "    #####################:\n"
  "\n\n\n"
  " |__________________________|"
  "\033M";


int main(int argc, char* argv[])
{
  if (argc > 1)
  {

  }
  lockshell();
}

