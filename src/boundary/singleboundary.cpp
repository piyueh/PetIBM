/***************************************************************************//**
 * \file singleboundary.cpp
 * \author Anush Krishnan (anus@bu.edu)
 * \author Olivier Mesnard (mesnardo@gwu.edu)
 * \author Pi-Yueh Chuang (pychuang@gwu.edu)
 * \brief Definition of the member functions of class `SingleBoundary`.
 */

// here goes headers from our PetIBM
# include <petibm/singleboundary.h>
# include <petibm/singleboundaryperiodic.h>
# include <petibm/singleboundarydirichlet.h>
# include <petibm/singleboundaryneumann.h>
# include <petibm/singleboundaryconvective.h>


namespace petibm
{
namespace boundary
{
    
PetscErrorCode createSingleBoundary(
        const type::Mesh &mesh, const type::BCLoc &loc, 
        const type::Field &field, const PetscReal &value,
        const type::BCType &bcType,
        type::SingleBoundary &singleBd)
{
    PetscFunctionBeginUser;
    
    switch (bcType)
    {
        case type::BCType::NOBC:
            singleBd = 
                std::make_shared<SingleBoundaryBase>(mesh, loc, field, value);
            break;
        case type::BCType::PERIODIC:
            singleBd = 
                std::make_shared<SingleBoundaryPeriodic>(mesh, loc, field, value);
            break;
        case type::BCType::DIRICHLET:
            singleBd = 
                std::make_shared<SingleBoundaryDirichlet>(mesh, loc, field, value);
            break;
        case type::BCType::NEUMANN:
            singleBd = 
                std::make_shared<SingleBoundaryNeumann>(mesh, loc, field, value);
            break;
        case type::BCType::CONVECTIVE:
            singleBd = 
                std::make_shared<SingleBoundaryConvective>(mesh, loc, field, value);
            break;
    }
    
    PetscFunctionReturn(0);
}

} // end of namespace boundary
} // end of namespace petibm
