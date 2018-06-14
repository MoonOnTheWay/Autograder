#include "syscall.h"

// This file tests both Exec and Join;

// Note: this test should be called from code/vm, because the filename
// is dependent on that;

int buffer[1500];

int
main()
{
    int id,id1,id2;
    id = Exec("/tmp/test/testvm1");
    id2 = Exec("/tmp/test/testvm1");
    Join(id);
    Join(id2);
    Write("Done\n",5,ConsoleOutput);
    Exit(0);
}

