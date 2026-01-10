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

        
        Dsp::Filter::TPT::StateVariableFilter<double> preLowPass;
        Dsp::Filter::TPT::StateVariableFilter<double> preHighPass;

        Dsp::Cv::ConstantTimeLinearSmoother<double> preLowPassCutoffFrequencySmoother;
        Dsp::Cv::ConstantTimeLinearSmoother<double> preHighPassCutoffFrequencySmoother;

    public:

        void setContext (Dsp::Context context)
        {
            preLowPass.setContext(context);
            preLowPassCutoffFrequencySmoother.setContext(context);
            preLowPassCutoffFrequencySmoother.setTime(0.1);

            preHighPass.setContext(context);
            preHighPassCutoffFrequencySmoother.setContext(context);
            preHighPassCutoffFrequencySmoother.setTime(0.1);

            antialiasingFilter.setCoefficients(Dsp::Filter::Fir::WindowedSincLowpass(14e3, 0.002, context.SR));
        }

        void processBlock (float* buffer, int numberOfSamples)
        {

            for (int i = 0; i < numberOfSamples; i++)
            {
                preHighPass.setCutoffFrequency(preHighPassCutoffFrequencySmoother.process());

                buffer[i] = preHighPass.processHighPass(buffer[i]);
            }

            for (int i = 0; i < numberOfSamples; i++)
            {
                preLowPass.setCutoffFrequency(preLowPassCutoffFrequencySmoother.process());

                buffer[i] = preLowPass.processLowPass(buffer[i]);
            }

            for (int i = 0; i < numberOfSamples; i++)
            {
                buffer[i] = antialiasingFilter.process(buffer[i]);
            }
            
        }

        void setPreLowPassCutoff(float frequency)
        {
            preLowPassCutoffFrequencySmoother.setTargetValue(frequency);
        }

        void setPreHighPassCutoff(float frequency)
        {
            preHighPassCutoffFrequencySmoother.setTargetValue(frequency);
        }

    };
}