/**
 * \file delta_test.cpp
 * \brief Unit-tests for the discrete delta functions.
 * \copyright Copyright (c) 2016-2018, Barba group. All rights reserved.
 * \license BSD 3-Clause License.
 */

#include <algorithm>
#include <random>
#include <vector>

#include <petsc.h>

#include "gtest/gtest.h"

#include "petibm/delta.h"

using namespace petibm::delta;

// check value is zero outside region of influence
TEST(deltaRomaEtAlTest, zeroOutside)
{
    const PetscReal h = 1.0;
    EXPECT_EQ(0.0, Roma_et_al_1999(1.5, h));
    EXPECT_EQ(0.0, Roma_et_al_1999(2.0, h));
}

// check maximum value at 0
TEST(deltaRomaEtAlTest, maximumValue)
{
    const PetscReal h = 1.0;
    EXPECT_EQ(2.0 / 3.0, Roma_et_al_1999(0.0, h));
}

// check delta function is monotonically decreasing
TEST(deltaRomaEtAlTest, decreasingInfluence)
{
    const PetscReal h = 1.0;
    // create a sorted vector of random reals between 0.0 and 1.5
    std::vector<PetscReal> vals(10);
    std::default_random_engine engine;
    std::uniform_real_distribution<PetscReal> distrib(0.0, 1.5);
    std::generate(vals.begin(), vals.end(), [&]() { return distrib(engine); });
    std::sort(vals.begin(), vals.end());
    // assert decreasing influence as the distance increases
    for (unsigned int i = 0; i < vals.size() - 1; i++)
        ASSERT_GT(Roma_et_al_1999(vals[i], h), Roma_et_al_1999(vals[i + 1], h));
}

// Run all tests
int main(int argc, char **argv)
{
    PetscErrorCode ierr, status;

    ::testing::InitGoogleTest(&argc, argv);
    ierr = PetscInitialize(&argc, &argv, nullptr, nullptr); CHKERRQ(ierr);
    status = RUN_ALL_TESTS();
    ierr = PetscFinalize(); CHKERRQ(ierr);

    return status;
}  // main
