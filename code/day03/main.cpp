#include <iostream>
#include "BPTree.h"
#include <stdlib.h>
#include <time.h>

using namespace std;
int main(int argc, char const *argv[])
{
    BPTree<int, int> *tree = new BPTree<int, int>();
    tree->insert(1, 1);
    tree->scan();
    return 0;
}
