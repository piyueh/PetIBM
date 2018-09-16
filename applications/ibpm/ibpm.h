/**
 * \file ibpm.h
 * \brief Definition of the class \c IBPMSolver.
 * \copyright Copyright (c) 2016-2018, Barba group. All rights reserved.
 * \license BSD 3-Clause License.
 * \see ibpm
 * \ingroup ibpm
 */

#pragma once

#include <petibm/bodypack.h>

#include "../navierstokes/navierstokes.h"

/**
 * \class IBPMSolver
 * \brief Immersed-boundary method proposed by Taira and Colonius (2007).
 * \see ibpm, NavierStokesSolver
 * \ingroup ibpm
 */
class IBPMSolver : protected NavierStokesSolver
{
public:
    // public methods that don't change
    using NavierStokesSolver::advance;
    using NavierStokesSolver::initializeASCIIFiles;
    using NavierStokesSolver::readTimeHDF5;
    using NavierStokesSolver::writeIterations;
    using NavierStokesSolver::writeTimeHDF5;

    /** \brief Default constructor.  */
    IBPMSolver() = default;

    /**
     * \brief Constructor; Set references to the mesh, boundary conditions, and
     *        immersed bodies.
     *
     * \param mesh [in] Structured Cartesian mesh object.
     * \param bc [in] Data object with boundary conditions.
     * \param bodies [in] Data object with bodies information.
     * \param node [in] YAML configuration.
     */
    IBPMSolver(const petibm::type::Mesh &mesh, const petibm::type::Boundary &bc,
               const petibm::type::BodyPack &bodies, const YAML::Node &node);

    /** \brief Default destructor.  */
    ~IBPMSolver();

    /** \brief Manually destroy data.  */
    PetscErrorCode destroy();

    /** \brief Initialize vectors, operators, and linear solvers.
     *
     * \param mesh [in] Structured Cartesian mesh object.
     * \param bc [in] Data object with boundary conditions.
     * \param bodies [in] Data object with bodies information.
     * \param node [in] YAML configuration.
     */
    PetscErrorCode initialize(const petibm::type::Mesh &mesh,
                              const petibm::type::Boundary &bc,
                              const petibm::type::BodyPack &bodies,
                              const YAML::Node &node);

    /**
     * \brief Write the solution into a file.
     *
     * \param t [in] Time.
     * \param filePath [in] Path of the file to write into.
     */
    PetscErrorCode write(const PetscReal &t, const std::string &filePath);

    /**
     * \brief Write the extra data that are required for restarting sessions.
     *
     * If the file already has solutions in it, only extra necessary data will
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

    /** \brief Combination of pressure and forces. */
    Vec P;

    /** \brief PETSc IS objects indicating which entries in phi belonging to
     *         pressure or forces. */
    IS isDE[2];

    /** \brief Log force integration. */
    PetscLogStage stageIntegrateForces;

    /** \brief Assemble the RHS vector of the Poisson system.  */
    virtual PetscErrorCode assembleRHSPoisson();

    /** \brief Create operators.  */
    virtual PetscErrorCode createOperators();

    /** \brief Create vectors.  */
    virtual PetscErrorCode createVectors();

    /** \brief Set null space or apply reference point.  */
    virtual PetscErrorCode setNullSpace();

};  // IBPMSolver
