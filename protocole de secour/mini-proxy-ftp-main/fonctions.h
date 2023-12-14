#include <stdbool.h>

bool loginServer(int *sockClient, int *sockServer, char *hostname);
int mainLoop(int *sockClient, int *sockServer, char *hostname);
