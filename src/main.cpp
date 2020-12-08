#include <iostream>
#include "code_gen.h"
#include <fstream>
std::ofstream ofs("./gdb.txt");

int main(int argc, char* argv[])
{
    PodCodeGenerator generator;
    return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
