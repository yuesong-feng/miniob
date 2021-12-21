#include <iostream>
#include "BPTree.h"
#include <stdlib.h>
#include <time.h>

using namespace std;
int main(int argc, char const *argv[])
{
    BPTree<int, const char*> *tree = new BPTree<int, const char*>();
    tree->insert(1, "This is the first data!");
    tree->insert(2, "This is the second data!");
    tree->display();
    return 0;
}
