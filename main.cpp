#include "DLPController.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    std::string outputfile = "./bin/test.bin";
    std::string inputfile =  "./bin/DLPC350_2017.bin";
    std::string imgfile = "./color/";

    DLPController dlp;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg.find("-input=") == 0) {
            inputfile = arg.substr(7);
        } 
        else if (arg.find("-output=") == 0) {
            outputfile = arg.substr(8);
        }
        else if (arg.find("-imgdir=") == 0) {
            imgfile = arg.substr(8);
        }
    }

    dlp.imgfile = imgfile;
    dlp.initbin = inputfile;
    dlp.newbin = outputfile;

    if (!dlp.readbinAndclear()) {
        std::cout<<"Error reading bin file"<<std::endl;
        return 1;
    }
    if (!dlp.buildNewBin()) {
        return 1;
    }

    return 0;
}
