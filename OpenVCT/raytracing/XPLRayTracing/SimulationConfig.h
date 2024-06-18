// SimulationConfig.h

#pragma once

#if defined(_MSC_VER)
#include <cstdint>
#endif

class SimulationConfig
{
public:
    float DetectorLocation[3];
    float DetectorXDir[3];
    float DetectorYDir[3];
    float DetectorZDir[3];
    float SourceLocation[3];
    float DistanceSrcToDet;
};
