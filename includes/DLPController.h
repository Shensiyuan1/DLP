#pragma once
#include "dlpc350_BMPParser.h"
#include "dlpc350_firmware.h"
#include <string>
#include <vector>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <direct.h>  

class DLPController{
public:
    DLPController();
    ~DLPController();   

    bool readbinAndclear();
    bool buildNewBin();

    std::string imgfile = "./color/";
    std::string initbin = "./bin/DLPC350_2017.bin";
    std::string newbin = "./bin/test.bin";

private:

    int m_splashImageAddIndex;
    int m_splashImageCount;
    int m_splashImageAdded;
    int m_splashImageRemoved;
    int m_numExtraSplashLutEntries;
    std::string m_addedSplashImages[256];
    

    int getBMPFileCount(const std::string& directory);

    static int My_FileWrite(void *Param, unsigned char *Data, unsigned int Size);
    static int My_ImgeGet(void *Param, unsigned int X, unsigned int Y, unsigned char *Pix, unsigned int Count);
    static unsigned char *g_pImageBuffer;
};