/**
 * \file decoupledibpm.h
 * \brief Definition of the class \c DecoupledIBPMSolver.
 * \copyright Copyright (c) 2016-2018, Barba group. All rights reserved.
 * \license BSD 3-Clause License.
 * \see decoupledibpm
 * \ingroup decoupledibpm
 */

#pragma once

#include <petibm/bodypack.h>

#include "../navierstokes/navierstokes.h"

/**
 * \class DecoupledIBPMSolver
 * \brief Immersed-boundary method proposed by Li et. al. (2016).
 * \see decoupledibpm, NavierStokesSolver
 * \ingroup decoupledibpm
 */
class DecoupledIBPMSolver : protected NavierStokesSolver
{
public:
    // public members that don't change
    using NavierStokesSolver::initializeASCIIFiles;
    using NavierStokesSolver::readTimeHDF5;
    using NavierStokesSolver::write;
    using NavierStokesSolver::writeTimeHDF5;

    /** \brief Default constructor. */
    DecoupledIBPMSolver() = default;

    /**
     * \brief Constructor; Set references to the mesh, boundary conditions, and
     *        immersed bodies.
     *
     * \param mesh [in] Structured Cartesian mesh object.
     * \param bc [in] Data object with the boundary conditions.
     * \param bodies [in] Data object with bodies information.
     * \param node [in] YAML configurations.
     */
    DecoupledIBPMSolver(const petibm::type::Mesh &mesh,
                        const petibm::type::Boundary &bc,
                        const petibm::type::BodyPack &bodies,
                        const YAML::Node &node);

    /** \brief Default destructor. */
    ~DecoupledIBPMSolver();

    /** \brief manually destroy data. */
    PetscErrorCode destroy();

    /** \brief Initialize vectors, operators, and linear solvers.
     *
     * \param mesh [in] Structured Cartesian mesh object.
     * \param bc [in] Data object with the boundary conditions.
     * \param bodies [in] Data object with bodies information.
     * \param node [in] YAML configurations.
     */
    PetscErrorCode initialize(const petibm::type::Mesh &mesh,
                              const petibm::type::Boundary &bc,
                              const petibm::type::BodyPack &bodies,
                              const YAML::Node &node);

    /** \brief Advance in time. */
    PetscErrorCode advance();

    /**
     * \brief Write the extra data that are required for restarting sessions.
     *
     * If file already exists, only extra necessary data will
     * be written in. Otherwise, solutions and extra data will all be written
     * in.
     *
     * \param t [in] Time.
     * \param filePath [in] Path of the file to write into.
     */
    PetscErrorCode writeRestartData(const PetscReal &t,
                                    const std::string &filePath);

    /**
     * \brief Read data that are required for restarting sessions.
     *
     * \param filePath [in] Path of the file to read from.
     * \param t [out] Time.
     */
    PetscErrorCode readRestartData(const std::string &filePath, PetscReal &t);

    /**
     * \brief Write number of iterations executed by each solver at current time
     *        step (to an ASCII file).
     *
     * \param timeIndex [in] Time-step index.
     * \param filePath [in] Path of the file to write into.
     */
    PetscErrorCode writeIterations(const int &timeIndex,
                                   const std::string &filePath);

    /**
     * \brief Write the integrated forces acting on the bodies into a ASCII
     * file.
     *
     * \param t [in] Time.
     * \param filePath [in] Path of the file to write into.
     */
    PetscErrorCode writeIntegratedForces(const PetscReal &t,
                                         const std::string &filePath);

protected:
    /** \brief A reference to immersed bodies. */
    petibm::type::BodyPack bodies;

    /** \brief Linear solver object for force solver. */
    petibm::type::LinSolver fSolver;

    /** \brief Operator interpolating Lagrangian forces to Eulerian forces. */
    Mat H;

    /** \brief Operator interpolating Eulerian forces to Lagrangian forces. */
    Mat E;

    /** \brief Coefficient matrix of the force system. */
    Mat EBNH;

    /** \brief Operator projecting force to intermediate velocity field. */
    Mat BNH;

    /** \brief Right-hand-side of force system. */
    Vec Eu;

    /** \brief Solution of Lagrangian force at time-step n. */
    Vec f;

    /** \brief Increment of force from time-step n to n+1. */
    Vec df;

    /** \brief Log RHS of forces system. */
    PetscLogStage stageRHSForces;

    /** \brief Log forces solver. */
    PetscLogStage stageSolveForces;

    /** \brief Log force integration. */
    PetscLogStage stageIntegrateForces;

    /** \brief Assemble the RHS vector of the velocity system.  */
    virtual PetscErrorCode assembleRHSVelocity();

    /** \brief Assemble the RHS vector of the Poisson system. */
    virtual PetscErrorCode assembleRHSPoisson();

    /** \brief Assemble the RHS vector of the system for the boundary forces. */
    virtual PetscErrorCode assembleRHSForces();

    /** \brief Solve the system for the boundary forces. */
    virtual PetscErrorCode solveForces();

    /** \brief Project the velocity to divergence-free space, update
     *         pressure field, and update force.  */
    virtual PetscErrorCode projectionStep();

    /** \brief Assembles operators and matrices. */
    PetscErrorCode createExtraOperators();

    /** \brief Create vectors. */
    PetscErrorCode createExtraVectors();

};  // DecoupledIBPMSolver
