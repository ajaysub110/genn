//--------------------------------------------------------------------------
/*! \file decode_matrix_cont_globalg_dense/test.cc

\brief Main test code that is part of the feature testing
suite of minimal models with known analytic outcomes that are used for continuous integration testing.
*/
//--------------------------------------------------------------------------


// Google test includes
#include "gtest/gtest.h"

// Auto-generated simulation code includess
#include DEFINITIONS_HEADER

// **NOTE** base-class for simulation tests must be
// included after auto-generated globals are includes
#include "../../utils/simulation_test.h"

//----------------------------------------------------------------------------
// SimTest
//----------------------------------------------------------------------------
class SimTest : public SimulationTest
{
public:
    //----------------------------------------------------------------------------
    // SimulationTest virtuals
    //----------------------------------------------------------------------------
    virtual void Init()
    {
    }

    //----------------------------------------------------------------------------
    // Public API
    //----------------------------------------------------------------------------
    bool Simulate()
    {
        for (int i = 0; i < (int)(10.0f / DT); i++)
        {
            // How many neurons should be active this timestep
            const unsigned int num_active_pre = i / 10;

            // Activate this many neurons
            for(unsigned int j = 0; j < 10; j++) {
                xPre[j] = (j < num_active_pre) ? 1.0f : 0.0f;
            }

#ifndef CPU_ONLY
            if(GetParam())
            {
                pushPreStateToDevice();
            }
#endif  // CPU_ONLY

            // Step GeNN
            StepGeNN();

            // Loop through output neurons
            for(unsigned int j = 0; j < 4; j++)
            {
                // If activation of postsynaptic neuron is incorrect fail
                if(fabs(xPost[j] - (float)num_active_pre) >= 1E-5)
                {
                    return false;
                }
            }

        }

        return true;
    }
};

TEST_P(SimTest, CorrectDecoding)
{
    // Check total error is less than some tolerance
    EXPECT_TRUE(Simulate());
}

#ifndef CPU_ONLY
auto simulatorBackends = ::testing::Values(true, false);
#else
auto simulatorBackends = ::testing::Values(false);
#endif

WRAPPED_INSTANTIATE_TEST_CASE_P(MODEL_NAME,
                                SimTest,
                                simulatorBackends);