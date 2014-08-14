#include "Design.h"
#include "main.h"

void printWelcome();
void printUsage();

int
main(int argc, char* argv[])
{
  printWelcome();

  Design* LC = new Design();
  if (false == LC->parseParameters(argc, argv))
  {
    printUsage();
    exit(1);
  }
  LC->readAll();
  //LC->outputAscii();
  LC->outputTcl();
} 


void
printWelcome()
{
  printf("\n");
  printf("==================   ascii2tcl - Version 1.1  ====================\n");
  printf("   Author      :  Bei Yu  (UT Austin)                             \n");
  printf("   Last Update :  07/2014                                         \n");
  printf("   Function    :  read ascii, dump out tcl                        \n");
  printf("==================================================================\n");
}


void
printUsage()
{
  printf("\n");
  printf("==================   Command-Line Usage   ====================\n");
  printf("\"-in\"         : followed by input library (default: \"input.ascii\"). \n");
  printf("\"-out\"        : followed by input library (default: \"output.tcl\"). \n");
	printf("\n");
}
