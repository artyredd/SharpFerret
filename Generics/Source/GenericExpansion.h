#pragma once

#include "sourceParser.h"
#include "Compiler.h"

array(MethodInfo) ExtractMethod(string data, array(location) locations);

void ExpandGenerics(string data, array(location) locations);
//void RunExpansionTests();