/**
 * \file singlebodypoints.cpp
 * \brief Implementation of body::SingleBodyPoints.
 * \copyright Copyright (c) 2016-2018, Barba group. All rights reserved.
 * \license BSD 3-Clause License.
 */

// STL
#include <algorithm>

// PetIBM
#include <petibm/io.h>
#include <petibm/singlebodypoints.h>

namespace petibm
{
namespace body
{
SingleBodyPoints::SingleBodyPoints(const MPI_Comm &comm, const PetscInt &dim,
                                   const std::string &name,
                                   const std::string &filePath)
    : SingleBodyBase(comm, dim, name, filePath)
{
    init(comm, dim, name, filePath);
}  // SingleBodyPoints

PetscErrorCode SingleBodyPoints::init(const MPI_Comm &comm, const PetscInt &dim,
                                      const std::string &name,
                                      const std::string &filePath)
{
    PetscErrorCode ierr;

    PetscFunctionBeginUser;

    // read the body coordinates from the given file
    ierr = readBody(filePath); CHKERRQ(ierr);

    // check if the dimension of coordinates matches
    if ((unsigned)dim != coords[0].size())
        SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_FILE_READ,
                "The dimension of Lagrangian points are different than that "
                "of the background mesh!\n");

    // record the initial body coordinates
    coords0 = coords;

    // create a distributed 1D DMDA with DoF equal to dim; nLclPts, bgPt, edPt,
    // da, nLclAllProcs, and offsetsAllProcs are set up here
    ierr = createDMDA(); CHKERRQ(ierr);

    // initialize meshIdx, which only contains background mesh indices of local
    // Lagrangian points. The indices are defined by pressure cell.
    meshIdx = type::IntVec2D(nLclPts, type::IntVec1D(dim, 0));

    // create info string
    ierr = createInfoString(); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // init

PetscErrorCode SingleBodyPoints::createDMDA()
{
    PetscErrorCode ierr;
    DMDALocalInfo lclInfo;

    PetscFunctionBeginUser;

    ierr = DMDACreate1d(comm, DM_BOUNDARY_NONE, nPts, dim, 0, nullptr, &da);
    CHKERRQ(ierr);
    ierr = DMSetUp(da); CHKERRQ(ierr);

    ierr = DMDAGetLocalInfo(da, &lclInfo); CHKERRQ(ierr);

    // copy necessary local info
    bgPt = lclInfo.xs;
    nLclPts = lclInfo.xm;
    edPt = bgPt + nLclPts;

    // gather local info from other processes
    ierr = MPI_Allgather(&nLclPts, 1, MPIU_INT, nLclAllProcs.data(), 1,
                         MPIU_INT, comm); CHKERRQ(ierr);

    // each point has "dim" degree of freedom, so we have to multiply that
    for (auto &it : nLclAllProcs) it *= dim;

    // calculate the offset of the unpacked DM
    for (PetscMPIInt r = mpiSize - 1; r > 0; r--)
        offsetsAllProcs[r] = nLclAllProcs[r - 1];
    for (PetscMPIInt r = 1; r < mpiSize; r++)
        offsetsAllProcs[r] += offsetsAllProcs[r - 1];

    PetscFunctionReturn(0);
}  // createDMDA

PetscErrorCode SingleBodyPoints::updateMeshIdx(const type::Mesh &mesh)
{
    PetscFunctionBeginUser;

    // loop through points owned locally and find indices
    for (PetscInt i = bgPt, c = 0; i < edPt; ++i, ++c)
    {
        for (PetscInt d = 0; d < dim; ++d)
        {
            if (mesh->min[d] >= coords[i][d] || mesh->max[d] <= coords[i][d])
            {
                SETERRQ3(PETSC_COMM_WORLD, PETSC_ERR_MAX_VALUE,
                         "body coordinate %g is outside domain [%g, %g] !",
                         coords[i][d], mesh->min[d], mesh->max[d]);
            }

            meshIdx[c][d] = std::upper_bound(mesh->coord[4][d],
                                             mesh->coord[4][d] + mesh->n[4][d],
                                             coords[i][d]) -
                            mesh->coord[4][d] - 1;
        }
    }

    PetscFunctionReturn(0);
}  // updateMeshIdx

PetscErrorCode SingleBodyPoints::createInfoString()
{
    PetscErrorCode ierr;
    std::stringstream ss;

    PetscFunctionBeginUser;

    // only rank 0 prepares the header of info string
    if (mpiRank == 0)
    {
        ss << std::string(80, '=') << std::endl;
        ss << "Body " << name << ":" << std::endl;
        ss << std::string(80, '=') << std::endl;
        ss << "\tInput mesh file: " << filePath << std::endl << std::endl;
        ss << "\tDimension: " << dim << std::endl << std::endl;
        ss << "\tTotal number of Lagrangian points: " << nPts << std::endl
           << std::endl;
        ss << "\tBody is distributed to " << mpiSize << " processes"
           << std::endl
           << std::endl;
        ss << "\tDistribution of Lagrangian points:" << std::endl << std::endl;
    }

    ss << "\t\tRank " << mpiRank << ":" << std::endl;
    ss << "\t\t\tNumber of points: " << nLclPts << std::endl;
    ss << "\t\t\tRange of points: [" << bgPt << ", " << edPt << ")"
       << std::endl;

    info = ss.str();

    ierr = MPI_Barrier(comm); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // createInfoString

PetscErrorCode SingleBodyPoints::findProc(const PetscInt &i,
                                          PetscMPIInt &p) const
{
    PetscFunctionBeginUser;

    if ((i < 0) || (i >= nPts))
        SETERRQ2(comm, PETSC_ERR_ARG_SIZ,
                 "Index %d of Lagrangian point on the body %s is out of range.",
                 i, name.c_str());

    // find the process that own THE 1ST DoF OF THE POINT i
    p = std::upper_bound(offsetsAllProcs.begin(), offsetsAllProcs.end(),
                         i * dim) -
        offsetsAllProcs.begin() - 1;

    PetscFunctionReturn(0);
}  // findProc

PetscErrorCode SingleBodyPoints::getGlobalIndex(const PetscInt &i,
                                                const PetscInt &dof,
                                                PetscInt &idx) const
{
    PetscFunctionBeginUser;

    if ((i < 0) || (i >= nPts))
        SETERRQ2(comm, PETSC_ERR_ARG_SIZ,
                 "Index %d of Lagrangian point on the body %s is out of range.",
                 i, name.c_str());

    if ((dof < 0) || (dof >= dim))
        SETERRQ2(comm, PETSC_ERR_ARG_SIZ,
                 "DoF %d is not correct. The dimension is %d.", dof, dim);

    // for single body DM, the global is simple due to we use 1D DMDA.
    idx = i * dim + dof;

    PetscFunctionReturn(0);
}  // getGlobalIndex

PetscErrorCode SingleBodyPoints::getGlobalIndex(const MatStencil &s,
                                                PetscInt &idx) const
{
    PetscErrorCode ierr;

    PetscFunctionBeginUser;

    ierr = getGlobalIndex(s.i, s.c, idx); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // getGlobalIndex

PetscErrorCode SingleBodyPoints::calculateAvgForces(const Vec &f,
                                                    type::RealVec1D &fAvg) const
{
    PetscErrorCode ierr;

    PetscFunctionBeginUser;

    PetscReal **fArry;

    type::RealVec1D fAvgLocal(dim, 0.0);

    ierr = DMDAVecGetArrayDOF(da, f, &fArry); CHKERRQ(ierr);

    for (PetscInt i = bgPt; i < edPt; ++i)
    {
        for (PetscInt dof = 0; dof < dim; ++dof)
        {
            fAvgLocal[dof] -=
                fArry[i][dof];  // fArray is the force applied to fluid
        }
    }
    ierr = MPI_Barrier(comm); CHKERRQ(ierr);

    ierr = MPI_Allreduce(fAvgLocal.data(), fAvg.data(), dim, MPIU_REAL, MPI_SUM,
                         comm); CHKERRQ(ierr);

    ierr = DMDAVecRestoreArrayDOF(da, f, &fArry); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // calculateAvgForces

PetscErrorCode SingleBodyPoints::readBody(const std::string &filepath)
{
    PetscErrorCode ierr;

    PetscFunctionBeginUser;

    // read the body coordinates from the given file;
    // attributes `nPts` and `coords` are set up here
    ierr = io::readLagrangianPoints(filePath, nPts, coords); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // readBody

// write coordinates of the Lagrangian points into ASCII file
PetscErrorCode SingleBodyPoints::writeBody(const std::string &filepath)
{
    PetscErrorCode ierr;

    PetscFunctionBeginUser;

    PetscViewer viewer;
    ierr = PetscViewerCreate(comm, &viewer); CHKERRQ(ierr);
    ierr = PetscViewerSetType(viewer, PETSCVIEWERASCII); CHKERRQ(ierr);
    ierr = PetscViewerFileSetMode(viewer, FILE_MODE_WRITE); CHKERRQ(ierr);
    ierr = PetscViewerFileSetName(viewer, filepath.c_str()); CHKERRQ(ierr);
    if (dim == 3)
    {
        for (PetscInt k = 0; k < nPts; ++k)
        {
            ierr = PetscViewerASCIIPrintf(
                viewer, "%10.8e\t%10.8e\t%10.8e\n",
                coords[k][0], coords[k][1], coords[k][2]); CHKERRQ(ierr);
        }
    }
    else if (dim == 2)
    {
        for (PetscInt k = 0; k < nPts; ++k)
        {
            ierr = PetscViewerASCIIPrintf(
                viewer, "%10.8e\t%10.8e\n",
                coords[k][0], coords[k][1]); CHKERRQ(ierr);
        }
    }
    else
         SETERRQ(PETSC_COMM_WORLD, PETSC_ERR_FILE_WRITE,
                "Function only supports 2D and 3D bodies.\n");
    ierr = PetscViewerDestroy(&viewer); CHKERRQ(ierr);

    PetscFunctionReturn(0);
}  // writeBody

}  // end of namespace body

}  // end of namespace petibm
