#pragma once

#include "ath_dsp/dsp/Context.h"
#include "ath_dsp/dsp/FIR.h"

namespace Ath::Echo700
{
    class Echo700
    {
        Dsp::Filter::Fir::Filter<double> filter;

    public:

        void setContext (Dsp::Context context)
        {
            filter.setCoefficients(Dsp::Filter::Fir::WindowedSincLowpass(14e3, 0.002, context.SR));
        }

        void processBlock (float* buffer, int numberOfSamples)
        {
            for (int i = 0; i < numberOfSamples; i++)
            {
                buffer[i] = filter.process(buffer[i]);
            }
        }
    };
}