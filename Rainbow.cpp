#ifndef CONFIG_H
#include "config.h"
#endif
#include "Rainbow.h"

Rainbow::Rainbow() {

}

void Rainbow::updateMultiplier(uint32_t iTimeStamp) {
  iMulti = iTimeStamp - lLastUpdate;

  #ifdef DEBUG_RAINBOW
    printf("Multi: ");
    printf("%d", iMulti);
    printf("\n");
  #endif
}

void Rainbow::calculateValues() {
  oDiff = getDifference(RainbowColors[iRainbow], RainbowColors[iRainbowNext]);
  vpmValue = calculateVpM(oDiff, RAINBOW_UPDATE_INTERVAL);
  CHA_Values[iCurrent] = calculateCurrent(vpmValue, (float)iMulti);
}

Rainbow::RGB Rainbow::getDifference(RGB oSource, RGB oTarget) {
  oResult = {0,0,0};

  oResult.r = oTarget.r - oSource.r;
  oResult.g = oTarget.g - oSource.g;
  oResult.b = oTarget.b - oSource.b;

  #ifdef DEBUG_RAINBOW
    printf("Diff: ");
    printf("%d", oResult.r);
    printf(" / ");
    printf("%d", oResult.g);
    printf(" / ");
    printf("%d", oResult.b);
    printf("\n");
  #endif

  return oResult;
}

Rainbow::VpMSet Rainbow::calculateVpM(RGB oColorDiff, unsigned long lDelay) {
  vResult.r = (float)oColorDiff.r / (float)lDelay;
  vResult.g = (float)oColorDiff.g / (float)lDelay;
  vResult.b = (float)oColorDiff.b / (float)lDelay;

  #ifdef DEBUG_RAINBOW
    printf("VpM: ");
    printf("%f", vResult.r);
    printf(" / ");
    printf("%f", vResult.g);
    printf(" / ");
    printf("%f", vResult.b);
    printf("\n");
  #endif

  return vResult;
}

Rainbow::RGB Rainbow::calculateCurrent(VpMSet oValues, float fMULTI) {
  oResult.r = RainbowColors[iRainbow].r + (oValues.r * fMULTI);
  oResult.g = RainbowColors[iRainbow].g + (oValues.g * fMULTI);
  oResult.b = RainbowColors[iRainbow].b + (oValues.b * fMULTI);

  #ifdef DEBUG_RAINBOW
    printf("Current: ");
    printf("%d", oResult.r);
    printf(" / ");
    printf("%d", oResult.g);
    printf(" / ");
    printf("%d", oResult.b);
    printf("\n");
  #endif

  return oResult;
}

int Rainbow::safeIncrement(int iValue, int iMax) {
  iResult = ++iValue;
  if (iValue > iMax)
    return 0;

  return iResult;
}
