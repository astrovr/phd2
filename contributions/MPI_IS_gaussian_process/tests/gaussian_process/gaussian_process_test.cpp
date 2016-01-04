/*
 * Copyright 2014-2015, Max Planck Society.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Created by Edgar Klenske <edgar.klenske@tuebingen.mpg.de>
 *
 * Provides the test cases for the Gaussian Process functionality.
 *
 */

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include "math_tools.h"
#include "gaussian_process.h"
#include "covariance_functions.h"

#include <fstream>

class GPTest : public ::testing::Test
{
public:
    GPTest(): random_vector_(11), location_vector_(11), hyper_parameters_(4)
    {
        random_vector_ << -0.1799, -1.4215, -0.2774,  2.6056, 0.6471, -0.4366,
                       1.3820,  0.4340,  0.8970, -0.7286, -1.7046;
        location_vector_ << 0, 0.1000, 0.2000, 0.3000, 0.4000, 0.5000, 0.6000,
                         0.7000, 0.8000, 0.9000, 1.0000;
        hyper_parameters_ << 1, 2, 3, 4;

        covariance_function_ =
            covariance_functions::PeriodicSquareExponential(hyper_parameters_);
        gp_ = GP(covariance_function_);
    }
    GP gp_;
    Eigen::VectorXd random_vector_;
    Eigen::VectorXd location_vector_;
    Eigen::VectorXd hyper_parameters_;
    covariance_functions::PeriodicSquareExponential covariance_function_;
};

// This test is based on Matlab computations
TEST_F(GPTest, drawSample_prior_test)
{
    Eigen::VectorXd sample = gp_.drawSample(location_vector_, random_vector_);
    Eigen::VectorXd expected_sample(11);
    expected_sample << -3.6134, -4.5058, -5.4064, -6.2924, -7.1410, -7.9299,
                    -8.6382, -9.2472, -9.7404, -10.1045, -10.3298;
    for (int i = 0; i < expected_sample.rows(); i++)
    {
        EXPECT_NEAR(sample(i), expected_sample(i), 1e-1);
    }
}

// This test is based on statistical expectations (mean)
TEST_F(GPTest, drawSamples_prior_mean_test)
{

    hyper_parameters_ << 1, 1, 1, 1; // use of smaller hypers needs less samples

    covariance_function_ =
        covariance_functions::PeriodicSquareExponential(hyper_parameters_);
    gp_ = GP(covariance_function_);

    int N = 10000; // number of samples to draw
    location_vector_ = Eigen::VectorXd(1);
    location_vector_ << 1;
    Eigen::MatrixXd sample_collection(location_vector_.rows(), N);
    sample_collection.setZero(); // just in case

    for (int i = 0; i < N; i++)
    {
        Eigen::VectorXd sample = gp_.drawSample(location_vector_);
        sample_collection.col(i) = sample;
    }
    Eigen::VectorXd sample_mean = sample_collection.rowwise().mean();

    for (int i = 0; i < sample_mean.rows(); i++)
    {
        EXPECT_NEAR(0, sample_mean(i), 1e-1);
    }
}

// This test is based on statistical expectations (covariance)
TEST_F(GPTest, drawSamples_prior_covariance_test)
{

    hyper_parameters_ << 1, 1, 1, 1; // use of smaller hypers needs less samples

    covariance_function_ =
        covariance_functions::PeriodicSquareExponential(hyper_parameters_);
    gp_ = GP(covariance_function_);

    int N = 20000; // number of samples to draw
    location_vector_ = Eigen::VectorXd(1);
    location_vector_ << 1;
    Eigen::MatrixXd sample_collection(location_vector_.rows(), N);
    sample_collection.setZero(); // just in case

    for (int i = 0; i < N; i++)
    {
        Eigen::VectorXd sample = gp_.drawSample(location_vector_);
        sample_collection.col(i) = sample;
    }
    Eigen::MatrixXd sample_cov = sample_collection * sample_collection.transpose()
                                 / N;

    Eigen::MatrixXd expected_cov = covariance_function_.evaluate(location_vector_,
                                   location_vector_).first;

    for (int i = 0; i < sample_cov.rows(); i++)
    {
        for (int k = 0; k < sample_cov.cols(); k++)
        {
            EXPECT_NEAR(expected_cov(i, k), sample_cov(i, k), 1e-1);
        }
    }
}


TEST_F(GPTest, setCovarianceFunction)
{
    Eigen::VectorXd hyperparams(5);
    hyperparams << 0.1, 15, 700, 25, 5000;

    GP instance_gp;
    EXPECT_TRUE(instance_gp.setCovarianceFunction(covariance_functions::PeriodicSquareExponential(hyperparams.tail(4))));

    GP instance_gp2 = GP(covariance_functions::PeriodicSquareExponential(Eigen::VectorXd::Zero(4)));
    instance_gp2.setHyperParameters(hyperparams);

    for (int i = 1; i < 5; i++) // first element is different (non set)
    {
        EXPECT_NEAR(instance_gp.getHyperParameters()[i], instance_gp2.getHyperParameters()[i], 1e-8);
    }
}



TEST_F(GPTest, setCovarianceFunction_notworking_after_inference)
{
    Eigen::VectorXd hyperparams(5);
    hyperparams << 0.1, 15, 700, 25, 5000;

    GP instance_gp;
    EXPECT_TRUE(instance_gp.setCovarianceFunction(covariance_functions::PeriodicSquareExponential(hyperparams.tail(4))));

    int N = 250;
    Eigen::VectorXd location =
        400 * math_tools::generate_uniform_random_matrix_0_1(N, 1)
        - 200 * Eigen::MatrixXd::Ones(N, 1);

    Eigen::VectorXd output_from_converged_hyperparams = instance_gp.drawSample(location);

    instance_gp.infer(location, output_from_converged_hyperparams);
    EXPECT_FALSE(instance_gp.setCovarianceFunction(covariance_functions::PeriodicSquareExponential(hyperparams.tail(4))));
}

// to be moved to some other file
TEST_F(GPTest, periodic_covariance_function_test)
{
    covariance_functions::PeriodicSquareExponential u;
    EXPECT_EQ(u.getParameterCount(), 4);

    GP instance_gp = GP(covariance_functions::PeriodicSquareExponential());
    ASSERT_EQ(instance_gp.getHyperParameters().size(), 5);
    instance_gp.setHyperParameters(Eigen::VectorXd::Zero(5)); // should not assert
}



TEST_F(GPTest, infer_prediction_clear_test)
{
    Eigen::VectorXd data_loc(1);
    data_loc << 1;
    Eigen::VectorXd data_out(1);
    data_out << 1;
    gp_.infer(data_loc, data_out);

    Eigen::VectorXd prediction_location(2);
    prediction_location << 1, 2;

    Eigen::VectorXd prediction = gp_.predict(prediction_location).first;

    EXPECT_NEAR(prediction(0), 1, 1e-6);
    EXPECT_FALSE(std::abs(prediction(1) - 1) < 1e-6);

    gp_.clear();

    prediction = gp_.predict(prediction_location).first;

    EXPECT_NEAR(prediction(0), 0, 1e-6);
    EXPECT_NEAR(prediction(1), 0, 1e-6);
}

TEST_F(GPTest, squareDistanceTest)
{
    Eigen::MatrixXd a(4, 3);
    Eigen::MatrixXd b(4, 5);
    Eigen::MatrixXd c(3, 4);

    a <<  3, 5, 5, 4, 6, 6, 3, 2, 3, 1, 0, 3;

    b <<  1, 4, 5, 6, 7, 3, 4, 5, 6, 7, 0, 2, 4, 20, 2, 2, 3, -2, -2, 2;

    c <<  1, 2, 3, 4, 4, 5, 6, 7, 6, 7, 8, 9;


    Eigen::MatrixXd sqdistc(4, 4);
    Eigen::MatrixXd sqdistab(3, 5);

    // Computed by Matlab
    sqdistc << 0, 3, 12, 27, 3, 0, 3, 12, 12, 3, 0, 3, 27, 12, 3, 0;

    sqdistab << 15, 6, 15, 311, 27, 33, 14, 9, 329, 9, 35, 6, 27, 315, 7;

    // Test argument order
    EXPECT_EQ(math_tools::squareDistance(a, b), math_tools::squareDistance
              (b, a).transpose());

    // Test that two identical Matrices give the same result
    // (wether they are the same object or not)
    EXPECT_EQ(math_tools::squareDistance(a, Eigen::MatrixXd(a)),
              math_tools::squareDistance(a, a));
    EXPECT_EQ(math_tools::squareDistance(a), math_tools::squareDistance(
                  a, a));

    // Test that the implementation gives the same result as the Matlab
    // implementation
    EXPECT_EQ(math_tools::squareDistance(c, c), sqdistc);
    EXPECT_EQ(math_tools::squareDistance(a, b), sqdistab);
}

TEST_F(GPTest, CovarianceTest2)
{
    Eigen::Vector4d hyperParams;
    hyperParams << 1, 2, 3, 4;

    Eigen::VectorXd locations(5), X(3), Y(3);
    locations << 0, 50, 100, 150, 200;
    X << 0, 100, 200;
    Y << 1, -1, 1;

    covariance_functions::PeriodicSquareExponential covFunc(hyperParams);
    GP gp(covFunc);

    Eigen::MatrixXd kxx_matlab(5, 5);
    kxx_matlab <<
               403.4288,  234.9952,   57.6856,    7.7574,    0.4862,
                          234.9952,  403.4288,  234.9952,   57.6856,    7.7574,
                          57.6856,  234.9952,  403.4288,  234.9952,   57.6856,
                          7.7574,   57.6856,  234.9952,  403.4288,  234.9952,
                          0.4862,    7.7574,   57.6856,  234.9952,  403.4288;

    Eigen::MatrixXd kxX_matlab(5, 3);
    kxX_matlab <<
               403.4288,   57.6856,    0.4862,
                           234.9952,  234.9952,    7.7574,
                           57.6856,  403.4288,   57.6856,
                           7.7574,  234.9952,  234.9952,
                           0.4862,   57.6856,  403.4288;

    Eigen::MatrixXd kXX_matlab(3, 3);
    kXX_matlab <<
               403.4288,   57.6856,    0.4862,
                           57.6856,  403.4288,   57.6856,
                           0.4862,   57.6856,  403.4288;

    Eigen::MatrixXd kxx = covFunc.evaluate(locations, locations).first;
    Eigen::MatrixXd kxX = covFunc.evaluate(locations, X).first;
    Eigen::MatrixXd kXX = covFunc.evaluate(X, X).first;

    for (int col = 0; col < kxx.cols(); col++)
    {
        for (int row = 0; row < kxx.rows(); row++)
        {
            EXPECT_NEAR(kxx(row, col), kxx_matlab(row, col), 0.003);
        }
    }

    for (int col = 0; col < kxX.cols(); col++)
    {
        for (int row = 0; row < kxX.rows(); row++)
        {
            EXPECT_NEAR(kxX(row, col), kxX_matlab(row, col), 0.003);
        }
    }

    for (int col = 0; col < kXX.cols(); col++)
    {
        for (int row = 0; row < kXX.rows(); row++)
        {
            EXPECT_NEAR(kXX(row, col), kXX_matlab(row, col), 0.003);
        }
    }
}

TEST_F(GPTest, covariance_derivative_test)
{
    int N = 10; // number of tests
    double eps = 1e-6;
    Eigen::VectorXd hyperParams(4);
    hyperParams << 1, 2, 3, 4;

    for (int h = 0; h < hyperParams.rows(); ++h)
    {
        Eigen::VectorXd hyper_plus(hyperParams);
        Eigen::VectorXd hyper_minus(hyperParams);

        hyper_plus[h] += eps;
        hyper_minus[h] -= eps;

        covariance_functions::PeriodicSquareExponential covFunc(hyperParams);

        Eigen::ArrayXXd cov_plus(5, 5);
        Eigen::ArrayXXd cov_minus(5, 5);

        Eigen::ArrayXXd analytic_derivative(5, 5);
        Eigen::ArrayXXd numeric_derivative(5, 5);

        Eigen::ArrayXXd relative_error(5, 5);
        Eigen::ArrayXXd absolute_error(5, 5);

        for (int i = 0; i < N; i++)
        {
            Eigen::VectorXd location = math_tools::generate_normal_random_matrix(5, 1);

            covFunc.setParameters(hyperParams);
            analytic_derivative = covFunc.evaluate(location, location).second[h];

            covFunc.setParameters(hyper_plus);
            cov_plus = covFunc.evaluate(location, location).first;
            covFunc.setParameters(hyper_minus);
            cov_minus = covFunc.evaluate(location, location).first;

            numeric_derivative = (cov_plus - cov_minus) / (2 * eps);

            absolute_error = (numeric_derivative - analytic_derivative).abs();

            EXPECT_NEAR(absolute_error.maxCoeff(), 0, 1e-6);
        }
    }
}

TEST_F(GPTest, CovarianceTest3)
{
    Eigen::Matrix<double, 6, 1> hyperParams;
    hyperParams << 10, 1, 1, 1, 100, 1;
    hyperParams = hyperParams.array().log();

    Eigen::VectorXd periodLength(1);
    periodLength << std::log(80);

    Eigen::VectorXd locations(5), X(3), Y(3);
    locations << 0, 50, 100, 150, 200;
    X << 0, 100, 200;

    covariance_functions::PeriodicSquareExponential2 covFunc(hyperParams);
    covFunc.setExtraParameters(periodLength);

    Eigen::MatrixXd kxx_matlab(5, 5);
    kxx_matlab <<
               3.00000, 1.06389, 0.97441, 1.07075, 0.27067,
                        1.06389, 3.00000, 1.06389, 0.97441, 1.07075,
                        0.97441, 1.06389, 3.00000, 1.06389, 0.97441,
                        1.07075, 0.97441, 1.06389, 3.00000, 1.06389,
                        0.27067, 1.07075, 0.97441, 1.06389, 3.00000;

    Eigen::MatrixXd kxX_matlab(5, 3);
    kxX_matlab <<
               3.00000, 0.97441, 0.27067, 1.06389, 1.06389,
                        1.07075, 0.97441, 3.00000, 0.97441, 1.07075,
                        1.06389, 1.06389, 0.27067, 0.97441, 3.00000;

    Eigen::MatrixXd kXX_matlab(3, 3);
    kXX_matlab <<
               3.00000, 0.97441, 0.27067,
                        0.97441, 3.00000, 0.97441,
                        0.27067, 0.97441, 3.00000;

    Eigen::MatrixXd kxx = covFunc.evaluate(locations, locations).first;
    Eigen::MatrixXd kxX = covFunc.evaluate(locations, X).first;
    Eigen::MatrixXd kXX = covFunc.evaluate(X, X).first;

    for (int col = 0; col < kxx.cols(); col++)
    {
        for (int row = 0; row < kxx.rows(); row++)
        {
            EXPECT_NEAR(kxx(row, col), kxx_matlab(row, col), 0.01);
        }
    }

    for (int col = 0; col < kxX.cols(); col++)
    {
        for (int row = 0; row < kxX.rows(); row++)
        {
            EXPECT_NEAR(kxX(row, col), kxX_matlab(row, col), 0.01);
        }
    }

    for (int col = 0; col < kXX.cols(); col++)
    {
        for (int row = 0; row < kXX.rows(); row++)
        {
            EXPECT_NEAR(kXX(row, col), kXX_matlab(row, col), 0.01);
        }
    }
}

TEST_F(GPTest, CovarianceDerivativeTest3)
{
    int N = 10; // number of tests
    double eps = 1e-6;
    Eigen::Matrix<double, 6, 1> hyperParams;
    hyperParams << 10, 1, 1, 1, 100, 1;
    hyperParams = hyperParams.array().log();

    Eigen::VectorXd periodLength(1);
    periodLength << std::log(80);

    for (int h = 0; h < hyperParams.rows(); ++h)
    {
        Eigen::VectorXd hyper_plus(hyperParams);
        Eigen::VectorXd hyper_minus(hyperParams);

        hyper_plus[h] += eps;
        hyper_minus[h] -= eps;

        covariance_functions::PeriodicSquareExponential2 covFunc(hyperParams);
        covFunc.setExtraParameters(periodLength);

        Eigen::ArrayXXd cov_plus(5, 5);
        Eigen::ArrayXXd cov_minus(5, 5);

        Eigen::ArrayXXd analytic_derivative(5, 5);
        Eigen::ArrayXXd numeric_derivative(5, 5);

        Eigen::ArrayXXd relative_error(5, 5);
        Eigen::ArrayXXd absolute_error(5, 5);

        for (int i = 0; i < N; i++)
        {
            Eigen::VectorXd location = math_tools::generate_normal_random_matrix(5, 1);

            covFunc.setParameters(hyperParams);
            analytic_derivative = covFunc.evaluate(location, location).second[h];

            covFunc.setParameters(hyper_plus);
            cov_plus = covFunc.evaluate(location, location).first;
            covFunc.setParameters(hyper_minus);
            cov_minus = covFunc.evaluate(location, location).first;

            numeric_derivative = (cov_plus - cov_minus) / (2 * eps);

            absolute_error = (numeric_derivative - analytic_derivative).abs();

            EXPECT_NEAR(absolute_error.maxCoeff(), 0, 1e-6);
        }
    }
}

TEST_F(GPTest, CovarianceTest4)
{
    Eigen::Matrix<double, 5, 1> hyperParams;
    hyperParams << 10, 1, 1, 80, 1;
    hyperParams = hyperParams.array().log();

    Eigen::VectorXd locations(5), X(3), Y(3);
    locations << 0, 50, 100, 150, 200;
    X << 0, 100, 200;

    covariance_functions::SquareExponentialPeriodic covFunc(hyperParams);

    Eigen::MatrixXd kxx_matlab(5, 5);
    kxx_matlab <<
               2.00000, 1.82258, 1.45783, 1.17242, 1.04394,
                        1.82258, 2.00000, 1.82258, 1.45783, 1.17242,
                        1.45783, 1.82258, 2.00000, 1.82258, 1.45783,
                        1.17242, 1.45783, 1.82258, 2.00000, 1.82258,
                        1.04394, 1.17242, 1.45783, 1.82258, 2.00000;

    Eigen::MatrixXd kxX_matlab(5, 3);
    kxX_matlab <<
               2.00000, 1.45783, 1.04394, 1.82258, 1.82258,
                        1.17242, 1.45783, 2.00000, 1.45783, 1.17242,
                        1.82258, 1.82258, 1.04394, 1.45783, 2.00000;

    Eigen::MatrixXd kXX_matlab(3, 3);
    kXX_matlab <<
               2.00000, 1.45783, 1.04394,
                        1.45783, 2.00000, 1.45783,
                        1.04394, 1.45783, 2.00000;

    Eigen::MatrixXd kxx = covFunc.evaluate(locations, locations).first;
    Eigen::MatrixXd kxX = covFunc.evaluate(locations, X).first;
    Eigen::MatrixXd kXX = covFunc.evaluate(X, X).first;

    for (int col = 0; col < kxx.cols(); col++)
    {
        for (int row = 0; row < kxx.rows(); row++)
        {
            EXPECT_NEAR(kxx(row, col), kxx_matlab(row, col), 0.01);
        }
    }

    for (int col = 0; col < kxX.cols(); col++)
    {
        for (int row = 0; row < kxX.rows(); row++)
        {
            EXPECT_NEAR(kxX(row, col), kxX_matlab(row, col), 0.01);
        }
    }

    for (int col = 0; col < kXX.cols(); col++)
    {
        for (int row = 0; row < kXX.rows(); row++)
        {
            EXPECT_NEAR(kXX(row, col), kXX_matlab(row, col), 0.01);
        }
    }
}

TEST_F(GPTest, CovarianceDerivativeTest4)
{
    int N = 10; // number of tests
    double eps = 1e-6;
    Eigen::Matrix<double, 5, 1> hyperParams;
    hyperParams << 10, 1, 1, 80, 1;
    hyperParams = hyperParams.array().log();

    for (int h = 0; h < hyperParams.rows(); ++h)
    {
        Eigen::VectorXd hyper_plus(hyperParams);
        Eigen::VectorXd hyper_minus(hyperParams);

        hyper_plus[h] += eps;
        hyper_minus[h] -= eps;

        covariance_functions::SquareExponentialPeriodic covFunc(hyperParams);

        Eigen::ArrayXXd cov_plus(5, 5);
        Eigen::ArrayXXd cov_minus(5, 5);

        Eigen::ArrayXXd analytic_derivative(5, 5);
        Eigen::ArrayXXd numeric_derivative(5, 5);

        Eigen::ArrayXXd relative_error(5, 5);
        Eigen::ArrayXXd absolute_error(5, 5);

        for (int i = 0; i < N; i++)
        {
            Eigen::VectorXd location = math_tools::generate_normal_random_matrix(5, 1);

            covFunc.setParameters(hyperParams);
            analytic_derivative = covFunc.evaluate(location, location).second[h];

            covFunc.setParameters(hyper_plus);
            cov_plus = covFunc.evaluate(location, location).first;
            covFunc.setParameters(hyper_minus);
            cov_minus = covFunc.evaluate(location, location).first;

            numeric_derivative = (cov_plus - cov_minus) / (2 * eps);

            absolute_error = (numeric_derivative - analytic_derivative).abs();

            EXPECT_NEAR(absolute_error.maxCoeff(), 0, 1e-6);
        }
    }
}

TEST_F(GPTest, likelihood_test)
{

    Eigen::VectorXd hyperParams(5);
    hyperParams << std::log(0.1), 1, 2, 3, 4;

    Eigen::VectorXd locations(5), X(3), Y(3);
    locations << 0, 50, 100, 150, 200;
    X << 0, 100, 200;
    Y << 1, -1, 1;

    gp_.setHyperParameters(hyperParams);

    Eigen::MatrixXd kXX_matlab(3, 3);
    kXX_matlab <<
               403.4288,   57.6856,    0.4862,
                           57.6856,  403.4288,   57.6856,
                           0.4862,   57.6856,  403.4288;

    gp_.infer(X, Y);

    double calculated_nll = gp_.neg_log_likelihood();

    Eigen::MatrixXd data_cov = kXX_matlab;
    data_cov += std::exp(2 * hyperParams[0])
                * Eigen::MatrixXd::Identity(data_cov.rows(), data_cov.cols());
    double expected_nll = Y.transpose() * data_cov.ldlt().solve(Y);
    expected_nll += data_cov.ldlt().vectorD().array().log().sum();
    expected_nll += data_cov.rows() * std::log(2 * M_PI);
    expected_nll *= 0.5;

    EXPECT_NEAR(calculated_nll, expected_nll, 1e-6);
}

TEST_F(GPTest, likelihood_derivative_test)
{
    int N = 1; // number of tests
    double eps = 1e-5;
    Eigen::VectorXd hyperParams(5);
    hyperParams << 1, 1, 2, 1, 2;

    for (int h = 0; h < hyperParams.rows(); ++h)
    {
        Eigen::VectorXd hyper_plus(hyperParams);
        Eigen::VectorXd hyper_minus(hyperParams);

        hyper_plus[h] += eps;
        hyper_minus[h] -= eps;

        double lik_plus;
        double lik_minus;

        double analytic_derivative;
        double numeric_derivative;

        double relative_error;
        double absolute_error;

        for (int i = 0; i < N; ++i)
        {
            Eigen::VectorXd location =
                100 * math_tools::generate_normal_random_matrix(50, 1);
            Eigen::VectorXd output = gp_.drawSample(location);
            gp_.infer(location, output);

            gp_.setHyperParameters(hyperParams);
            analytic_derivative = gp_.neg_log_likelihood_gradient()[h];

            gp_.setHyperParameters(hyper_plus);
            lik_plus = gp_.neg_log_likelihood();
            gp_.setHyperParameters(hyper_minus);
            lik_minus = gp_.neg_log_likelihood();

            numeric_derivative = (lik_plus - lik_minus) / (2 * eps);

            absolute_error = std::abs(numeric_derivative - analytic_derivative);

            relative_error = absolute_error /
                             (0.5 * (std::abs(numeric_derivative) + std::abs(analytic_derivative)));
            if (relative_error > 1e-4)
            {
                std::cout << "Failing derivative: " << h << std::endl;
                std::cout << "Numeric derivative: " << numeric_derivative << std::endl;
                std::cout << "Analyt. derivative: " << analytic_derivative << std::endl;
            }

            EXPECT_NEAR(relative_error, 0, 1e-4);
        }
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
