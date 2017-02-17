#pragma once

#include "Sink.h"

struct VideoCaptureAttribute {
    int width;
    int height;
    int fps;
};

struct AudioCaptureAttribute {
    int channel;
    int samplerate;
};

class Capture
{

public:
    static int EnumVideoCature(TCHAR * vCaptureList[]);
    static int EnumAudioCature(TCHAR * aCaptureList[]);
    static Capture* GetVideoCature(int index);
    static Capture* GetAudioCature(int index);

public:
    virtual int AddSink(Sink * sink) = 0;
    virtual int EnumAttribute(void** attribute) = 0;
    virtual int Config(void* attribute) = 0;
    virtual int Start() = 0;
    virtual int Stop() = 0;
};

