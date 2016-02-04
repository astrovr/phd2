//
//  guide_gaussian_process.h
//  PHD2 Guiding
//
//  Created by Stephan Wenninger and Edgar Klenske.
//  Copyright 2014-2015, Max Planck Society.

/*
 *  This source code is distributed under the following "BSD" license
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *    Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *    Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *    Neither the name of Bret McKee, Dad Dog Development, nor the names of its
 *     Craig Stark, Stark Labs nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef GUIDE_GAUSSIAN_PROCESS
#define GUIDE_GAUSSIAN_PROCESS

#include "guide_algorithm.h"

#define GP_DEBUG_FILE_ 1

#if GP_DEBUG_FILE_
#include <iostream>
#include <iomanip>
#include <fstream>
#endif

#define CIRCULAR_BUFFER_SIZE 2048

class wxStopWatch;

class GuideAlgorithmGaussianProcess : public GuideAlgorithm
{
private:
    struct gp_guide_parameters;
    class GuideAlgorithmGaussianProcessDialogPane;

    gp_guide_parameters* parameters;

    void HandleTimestamps();
    void FilterDiffTime();
    void HandleMeasurements(double input);
    void HandleDarkGuiding();
    void HandleControls(double control_input);
    void StoreControls(double control_input);
    void HandleSNR(double noise);
    void UpdateGP();
    double PredictGearError();

protected:
    double GetControlGain() const;
    bool SetControlGain(double control_gain);

    // minimum amount of points for starting the inference
    bool SetNbElementForInference(int nb_elements);
    int GetNbMeasurementsMin() const;

    // minimum amount of points between consecutive calls to the optimisation
    int GetNbPointsBetweenOptimisation() const;
    bool SetNbPointsOptimisation(int);

    int GetNbPointsForApproximation() const;
    bool SetNbPointsForApproximation(int);

    bool GetBoolOptimizeHyperparameters() const;
    bool SetBoolOptimizeHyperparameters(bool);

    bool GetBoolComputePeriod() const;
    bool SetBoolComputePeriod(bool);

    bool SetGPHyperparameters(std::vector<double> const& hyperparameters);
    std::vector<double> GetGPHyperparameters() const;

    // Mixing parameter
    double GetMixingParameter() const;
    bool SetMixingParameter(double);


public:
    GuideAlgorithmGaussianProcess(Mount *pMount, GuideAxis axis);
    virtual ~GuideAlgorithmGaussianProcess(void);
    virtual GUIDE_ALGORITHM Algorithm(void);

    virtual ConfigDialogPane *GetConfigDialogPane(wxWindow *pParent);
    virtual double result(double input);
    virtual double deduceResult(void);
    virtual void reset();
    virtual wxString GetSettingsSummary();
    virtual wxString GetGuideAlgorithmClassName(void) const { return "Gaussian Process"; }

};

#endif  // GUIDE_GAUSSIAN_PROCESS
