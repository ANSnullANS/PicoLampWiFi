#pragma once
#include <stdio.h>

#ifndef RAINBOW_H
#define RAINBOW_H

class Rainbow {
    public:
        Rainbow();
        uint32_t lLastUpdate = 0;
        int iCurrent =                0;
        int iRainbow =                0;
        int iRainbowNext =            1;
        uint32_t iMulti = 0;

        struct RGB {
            int r;
            int g;
            int b;
        };

        struct VpMSet {
            float r;
            float g;
            float b;
        };

        RGB CHA_Values[24];

        void updateMultiplier(uint32_t iTimeStamp);
        void calculateValues();
        VpMSet calculateVpM(RGB oColorDiff, unsigned long lDelay);
        RGB getDifference(RGB oSource, RGB oTarget);
        RGB calculateCurrent(VpMSet oValues, float fMULTI);
        int safeIncrement(int iValue, int iMax);

    private:
        
        uint32_t iNow = 0;
        VpMSet vResult =    {0,0,0};
        VpMSet vpmValue =   {0,0,0};

        RGB oResult =       {0,0,0};
        RGB oDiff =         {0,0,0};

        int iResult = 0;

        #define RAINBOWMAX 7

        RGB RainbowColors[RAINBOWMAX] = {
        {148, 0, 211},
        {75, 0, 130},
        {0, 0, 255},
        {0, 255, 0},
        {255, 255, 0},
        {255, 127, 0},
        {255, 0, 0}
        };
};

#endif