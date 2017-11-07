/**
 * \file navierstokes.h
 * \brief Definition of the class \c TairaColoniusSolver.
 */

#pragma once

// Navier-Stokes solver
# include "../navierstokes/navierstokes.h"

// PetIBM
# include <petibm/bodypack.h>


/**
 * \class TairaColoniusSolver
 * \brief Taira and Colonius (2007).
 */
class TairaColoniusSolver : protected NavierStokesSolver
{
public:

    // public methods that don't change
    using NavierStokesSolver::advance;
    using NavierStokesSolver::writeRestartData;
    using NavierStokesSolver::readRestartData;
    using NavierStokesSolver::writeIterations;
    
    /** \brief Default constructor.  */
    TairaColoniusSolver() = default;

    /**
     * \brief Constructor; Set references to the mesh, boundary conditions, and
     *        immersed bodies.
     *
     * \param mesh [in] a type::Mesh object.
     * \param bc [in] a type::Boundary object.
     * \param bodies [in] a type::BodyPack object.
     * \param node [in] YAML::Node containing settings.
     */
    TairaColoniusSolver(
            const petibm::type::Mesh &mesh,
            const petibm::type::Boundary &bc,
            const petibm::type::BodyPack &bodies,
            const YAML::Node &node);

    /**
     * \brief Default destructor.
     */
    ~TairaColoniusSolver() = default;

    /**
     * \brief Initialize vectors, operators, and linear solvers.
     */
    PetscErrorCode initialize(
            const petibm::type::Mesh &mesh,
            const petibm::type::Boundary &bc,
            const petibm::type::BodyPack &bodies,
            const YAML::Node &node);

    /**
     * \brief Write the solution into a file.
     *
     * \param filePath [in] path of the file to save (without the extension)
     */
    PetscErrorCode write(const std::string &filePath);

    /**
     * \brief Write the integrated forces acting on the bodies into a ASCII file.
     *
     * \param time [in] Time value
     * \param fileName [in] Name of the file to save.
     */
    PetscErrorCode writeIntegratedForces(
            const PetscReal &t, const std::string &filePath);

    /**
     * \brief Destroy PETSc objects (vectors and matrices) and linear solvers.
     */
    PetscErrorCode finalize();

    
protected:
    
    /** \brief a reference to immersed bodies. */
    petibm::type::BodyPack      bodies;
    
    /** \brief combination of pressure and forces. */
    Vec P;
    
    /** \brief PETSc IS objects indicating which entries in phi belonging to 
     *         pressure or forces. */
    IS isDE[2];
    
    /** \brief Log force integration. */
    PetscLogStage stageIntegrateForces; 
    


    /** \brief Assemble the RHS vector of the Poisson system.  */
    virtual PetscErrorCode assembleRHSPoisson();
    
    /** \brief create operators.  */
    virtual PetscErrorCode createOperators();
    
    /** \brief create vectors.  */
    virtual PetscErrorCode createVectors();
    
    /** \brief set null space or apply reference point.  */
    virtual PetscErrorCode setNullSpace();
};
