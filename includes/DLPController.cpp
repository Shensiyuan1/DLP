#include "DLPController.h"

unsigned char* DLPController::g_pImageBuffer = nullptr;

DLPController::DLPController()
{
    
    m_splashImageAddIndex = 0;
    m_splashImageCount = 0;
    m_splashImageAdded = 0;
    m_splashImageRemoved = 0;
    m_numExtraSplashLutEntries = 0;
    for (int i = 0; i < 256; i++) {
        m_addedSplashImages[i].clear();
    }
    if (g_pImageBuffer == nullptr) {
        g_pImageBuffer = (unsigned char*)malloc(PTN_WIDTH * PTN_HEIGHT * BYTES_PER_PIXEL);
        if (g_pImageBuffer) {
            memset(g_pImageBuffer, 0, PTN_WIDTH * PTN_HEIGHT * BYTES_PER_PIXEL);
        }
    }
    std::cout<<"init tool;"<<std::endl;
}

DLPController::~DLPController()
{
    for(int i = 0;i < MAX_SPLASH_IMAGES ; i++)
    {
        m_addedSplashImages[i].clear();
    }
    m_splashImageAddIndex = 0;
    if (g_pImageBuffer) {
        free(g_pImageBuffer);
        g_pImageBuffer = nullptr;
    };
}

int DLPController::getBMPFileCount(const std::string& directory) {
    WIN32_FIND_DATAA findData;
    std::string searchPath = directory + "/*.bmp";
    int count = 0;

    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        std::cout << "can't read file: " << directory << std::endl;
        return 0;
    }

    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            count++;
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    return count;
}

bool DLPController::readbinAndclear()
{
    std::string fileName = initbin;
    
    if(!fileName.empty())
    {
        std::fstream imgFileIn(fileName, std::ios::in | std::ios::out | std::ios::binary);
        if(!imgFileIn.is_open())
        {
            std::cout << "Unable to open image file. Copy image file to a folder with Admin/read/write permission and try again"<< std::endl;
            return 0;
        }
        

        // Clear image array
        for (int i = 0; i < 256; i++)
            m_addedSplashImages[i].clear();

        // Get file size
        imgFileIn.seekg(0, std::ios::end);
        std::streamoff offset = imgFileIn.tellg();
        if (offset > INT_MAX) {
            imgFileIn.close();
            std::cout << "File size exceeds maximum allowed size" << std::endl;
            return 0;
        }
        std::streamsize fileLen = static_cast<std::streamsize>(offset);
        imgFileIn.seekg(0, std::ios::beg);

        unsigned char *pByteArray = (unsigned char *)malloc(fileLen);
        if(pByteArray == NULL)
        {
            imgFileIn.close();
            std::cout << "Memory alloc for file read failed"<< std::endl;
            return 0;
        }

        // Read file
        imgFileIn.read((char *)pByteArray, fileLen);
        imgFileIn.close();

        int splash_count, actual_splash_count, ret;

        int i;

        ret = DLPC350_Frmw_CopyAndVerifyImage(pByteArray, static_cast<int>(fileLen));
        if (ret)
        {
            switch(ret)
            {
            case ERROR_FRMW_FLASH_TABLE_SIGN_MISMATCH:
                std::cout << "ERROR: Flash Table Signature doesn't match! Bad Firmware Image!"<< std::endl;
                free(pByteArray);
                return 0;
            case ERROR_NO_MEM_FOR_MALLOC:
                std::cout << "Fatal Error! System Run out of memory"<< std::endl;
                free(pByteArray);
                return 0;
            default:
                break;
            }
        }

        free(pByteArray);

        if ((DLPC350_Frmw_GetVersionNumber() & 0xFFFFFF) < RELEASE_FW_VERSION)
            std::cout << "WARNING: Old version of Firmware detected.\n\nDownload the latest release from http://www.ti.com/tool/dlpr350.";

        splash_count = DLPC350_Frmw_GetSplashCount();
        actual_splash_count = splash_count;
        if (splash_count < 0)
        {
            std::cout << "Firmware file image header format not recognized"<< std::endl;
        }
        else
        {
            std::cout << "Retrieving Pattern Images:" << splash_count<< std::endl;

            for (i = 0; i < splash_count; i++)
            {
                char file_name[11];

                //sprintf_s(file_name, sizeof(file_name), "temp_%d.bmp", i);
                std::fstream outFile(file_name, std::ios::in | std::ios::out | std::ios::binary);

                if(outFile.is_open())
                {
                    BMP_Image_t splashImage;

                    ret = DLPC350_Frmw_GetSpashImage(g_pImageBuffer, i);

                    if (ret)
                    {

                        outFile.close();
                        memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
                        switch(ret)
                        {
                        case ERROR_NO_MEM_FOR_MALLOC:
                            std::cout << "Fatal Error! System Run out of memory"<< std::endl;
                            return 0;
                        case ERROR_NO_SPLASH_IMAGE:
                            actual_splash_count--;
                            continue;
                        default:
                            continue;
                        }
                    }


                    BMP_InitImage(&splashImage, PTN_WIDTH, PTN_HEIGHT, 8*BYTES_PER_PIXEL);
                    BMP_StoreImage(&splashImage, (BMP_DataFunc_t *)My_FileWrite, &outFile, (BMP_PixelFunc_t *)My_ImgeGet, NULL);
                    outFile.close();
                    memset(g_pImageBuffer, 0, PTN_WIDTH*PTN_HEIGHT*BYTES_PER_PIXEL);
                    // Use file_name directly
                    m_addedSplashImages[i] = file_name;
                    std::cout << file_name << std::endl;
                }
                else
                    std::cout << "Cannot save pattern image as .bmp to display. Try relocating GUI to a folder that has write permission"<< std::endl;
            }
        }
        //std::cout << "splash image add index:"<< m_splashImageAddIndex <<"splash image count:"<< m_splashImageCount<<std::endl;
        m_splashImageAddIndex = actual_splash_count;
        m_splashImageCount = actual_splash_count;
    }

    std::cout << "solash Image Add Inex:"<< m_splashImageAddIndex << "solash Image count :"<<m_splashImageCount<<std::endl;

    if (!m_splashImageAddIndex)
    {
        std::cout << "No image in bin" << std::endl;
    }
    else
    {
        for(int i = 0;i < MAX_SPLASH_IMAGES ; i++)
        {
            m_addedSplashImages[i].clear();
        }
        m_splashImageAddIndex = 0;
        std::cout << "Images in bin have been cleared" << std::endl;
    }
    return 1;
}

bool DLPController::buildNewBin(){
    // For file operations
    std::string imageFile = imgfile;
    int bmpCount = getBMPFileCount(imageFile);
    if (bmpCount == 0) {
        std::cout << "No BMP files found in the specified directory" << std::endl;
        return 0;
    }
    if (bmpCount > MAX_SPLASH_IMAGES) {
        
        bmpCount = MAX_SPLASH_IMAGES;
        std::cout << "Number of BMP files exceeds the maximum allowed limit and bmpCount is setted 256" << std::endl;
    }
    for(int i = 0;i < bmpCount ; i++)
    {
        std::string fullPath = imageFile + std::to_string(i) + ".bmp";
        m_addedSplashImages[m_splashImageAddIndex] = fullPath;
        std::cout << fullPath << std::endl;
        m_splashImageAddIndex++;
    }
    std::cout <<"splash image add index:"<< m_splashImageAddIndex<<std::endl;

    std::string newframe = newbin;
    unsigned char *newFrmwImage;
    uint32 newFrmwSize;

    const char *pCh = "131";
    size_t strLength = strlen(pCh);
    if (strLength > INT_MAX) {
        std::cout << "String length exceeds maximum allowed size" << std::endl;
        return 0;
    }
    int length = static_cast<int>(strLength);

    uint32 params[32];

    for(int i = 0; i < length; i++)
    {
        params[i] = (pCh[i]&0xFF);

        if(i > 31)
        {

            length = i;
            break;
        }
    }

    if (DLPC350_Frmw_WriteApplConfigData(const_cast<char*>("DEFAULT.FIRMWARE_TAG"), &params[0], length))
    {
        std::cout << "Unable to add firmware tag information"<< std::endl;
        return 0;
    }


    uint32 param = 0;
    if (DLPC350_Frmw_WriteApplConfigData(const_cast<char*>("DEFAULT.LED_ENABLE_MAN_MODE"), &param, 1))
    {
        std::cout << "Unable to add illumination configuration color/monochrome information"<< std::endl;
        return 0;
    }
    param = 0;

    if (DLPC350_Frmw_WriteApplConfigData(const_cast<char*>("MACHINE_DATA.COLORPROFILE_0_BRILLIANTCOLORLOOK"), &param, 1))
    {
        std::cout << "Unable to add illumination configuration color/monochrome information"<< std::endl;
        return 0;
    }

    int count = 0;
    for(int i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (!m_addedSplashImages[i].empty())
            count++;
    }

    //std::cout << count << DLPC350_Frmw_SPLASH_InitBuffer(count) ;

    if(DLPC350_Frmw_SPLASH_InitBuffer(count) < 0)
    {
        std::cout << "Unable to allocate memory for splash images"<< std::endl;
        return 0;
    }

    // Use standard file stream for log file
    std::ofstream logFile("Frmw-build.log");
    logFile << "Building Images from specified BMPs\n\n";

    for(int i = 0; i < MAX_SPLASH_IMAGES; i++)
    {
        if (m_addedSplashImages[i].empty())
            continue;

        std::fstream imgFile(m_addedSplashImages[i], std::ios::in | std::ios::binary);
        unsigned char *pByteArray;
        uint8 compression;
        uint32 compSize;
        int fileLen, ret;
        char dbgStr[1024];

        imgFile.seekg(0, std::ios::end);
        std::streamoff offset = imgFile.tellg();
        if (offset > INT_MAX) {
            imgFile.close();
            std::cout << "File size exceeds maximum allowed size" << std::endl;
            return 0;
        }
        fileLen = static_cast<int>(offset);
        imgFile.seekg(0, std::ios::beg);

        pByteArray = (unsigned char *)malloc(fileLen);

        if(pByteArray == NULL)
        {
            imgFile.close();
            std::cout << "Memory alloc for file read failed"<< std::endl;
            return 0;
        }
        imgFile.read((char *)pByteArray, fileLen);
        imgFile.close();

        logFile << m_addedSplashImages[i] << "\n";
        logFile << "\t" << "Uncompressed Size = " << fileLen << " Compression type : ";

        if (m_addedSplashImages[i].find("_nocomp.bmp") != std::string::npos)
            compression = SPLASH_UNCOMPRESSED;
        else if (m_addedSplashImages[i].find("_rle.bmp") != std::string::npos)
            compression = SPLASH_RLE_COMPRESSION;
        else if (m_addedSplashImages[i].find("_4line.bmp") != std::string::npos)
            compression = SPLASH_4LINE_COMPRESSION;
        else
            compression = SPLASH_NOCOMP_SPECIFIED;

        ret = DLPC350_Frmw_SPLASH_AddSplash(pByteArray, &compression, &compSize);
        if (ret < 0)
        {
            switch(ret)
            {
            case ERROR_NOT_BMP_FILE:
                sprintf_s(dbgStr, sizeof(dbgStr), "Error building firmware - %s not in BMP format", m_addedSplashImages[i].c_str());
                break;
            case ERROR_NOT_24bit_BMP_FILE:
                sprintf_s(dbgStr, sizeof(dbgStr), "Error building firmware - %s not in 24-bit format", m_addedSplashImages[i].c_str());
                break;
            case ERROR_NO_MEM_FOR_MALLOC:
                sprintf_s(dbgStr, sizeof(dbgStr), "Error building firmware with %s - Insufficient memory", m_addedSplashImages[i].c_str());
                break;
            default:
                sprintf_s(dbgStr, sizeof(dbgStr), "Error building firmware with %s - error code %d", m_addedSplashImages[i].c_str(), ret);
                break;
            }
            return 0;
        }

        switch(compression)
        {
        case SPLASH_UNCOMPRESSED:
            logFile << "Uncompressed";
            break;
        case SPLASH_RLE_COMPRESSION:
            logFile << "RLE Compression";
            break;
        case SPLASH_4LINE_COMPRESSION:
            logFile << "4 Line Compression";
            break;
        default:
            break;
        }
        logFile << " Compressed Size = " << compSize << "\n\n";

        free(pByteArray);
    }

    logFile.close();
    DLPC350_Frmw_Get_NewFlashImage(&newFrmwImage, &newFrmwSize);

    // Modify file opening code
    std::fstream outFile(newframe, std::ios::out | std::ios::binary | std::ios::trunc);
    if(outFile.is_open())
    {
        outFile.write((char*)newFrmwImage, newFrmwSize);
        if(outFile.fail())
        {
            std::cout << "Write output file error: "  << std::endl;
        }
        outFile.close();
    }
    else
    {
        std::cout << "Could not open output file: " << std::endl;
    }

    std::cout << "Build Complete";
    return 1;
}

int DLPController::My_FileWrite(void *Param, unsigned char *Data, unsigned int Size)
{
    std::fstream *pOutFile = (std::fstream *)Param;

    if(Data == NULL)
    {
        pOutFile->seekp(Size, std::ios::beg);
        return pOutFile->fail() ? -1 : 0;
    }
    else if(Size > 0)
    {
        pOutFile->write((char *)Data, Size);
        return pOutFile->fail() ? -1 : 0;
    }

    return 0;
}

// Keep the image acquisition function unchanged
int DLPController::My_ImgeGet(void *Param, unsigned int X, unsigned int Y, 
                     unsigned char *Pix, unsigned int Count)
{
    if(Param == NULL)
        Param = NULL;

    if(X >= PTN_WIDTH || Y >= PTN_HEIGHT)
        return 0;
    if(X + Count > PTN_WIDTH)
    {
        Count = PTN_WIDTH - X;
    }
    memcpy(Pix, g_pImageBuffer + (X + Y * PTN_WIDTH) * BYTES_PER_PIXEL,\
           Count * BYTES_PER_PIXEL);
    return 0;
}