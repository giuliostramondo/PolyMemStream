#ifndef TESTS_H
#define TESTS_H

#include <stdio.h>
#include <stdlib.h>
#include "Maxfiles.h"
#include "MaxSLiCInterface.h"
#include "schedule_utils.h"

struct testsuite{
    int (*function)();
    char *name;
};

typedef struct testsuite Testsuite;

int run_testsuite(Testsuite* tests, int testnb);

int test_STREAM();
#endif
