#pragma once

#include "ath_dsp/dsp/Context.h"
#include "ath_dsp/dsp/Filter.h"
#include "ath_dsp/dsp/FIR.h"
#include "ath_dsp/dsp/cv/LinearSmoother.h"

namespace Ath::Echo700
{
    class Echo700
    {
        Dsp::Filter::Fir::Filter<double> antialiasingFilter;

        
        Dsp::Filter::TPT::LowPass1<double> preLowPass;
        Dsp::Cv::ConstantTimeLinearSmoother<double> preLowPassCutoffFrequencySmoother;

    public:

        void setContext (Dsp::Context context)
        {
            preLowPass.setContext(context);
            preLowPassCutoffFrequencySmoother.setContext(context);
            preLowPassCutoffFrequencySmoother.setTime(0.1);

            antialiasingFilter.setCoefficients(Dsp::Filter::Fir::WindowedSincLowpass(14e3, 0.002, context.SR));
        }

        void processBlock (float* buffer, int numberOfSamples)
        {
            for (int i = 0; i < numberOfSamples; i++)
            {
                preLowPass.setCutoffFrequency(preLowPassCutoffFrequencySmoother.process());

                buffer[i] = preLowPass.process(buffer[i]);
            }

            for (int i = 0; i < numberOfSamples; i++)
            {
                buffer[i] = antialiasingFilter.process(buffer[i]);
            }
            
        }

        void setPrefilterCutoff(float frequency)
        {
            preLowPassCutoffFrequencySmoother.setTargetValue(frequency);
        }
    };
}