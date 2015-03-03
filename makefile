# file: makefile
# author: Anush Krishnan (anush@bu.edu), Olivier Mesnard (mesnardo@gwu.edu)
# description: Compiles and link PetIBM using PETSc.


SRC_DIR = $(PETIBM_DIR)/src
SUFFIX = .cpp
SRCS = $(shell find $(SRC_DIR) -type f -name *$(SUFFIX))
OBJ = $(addsuffix .o, $(basename $(SRCS))) $(SRC_DIR)/PetIBM*d.o

BIN_DIR = $(PETIBM_DIR)/bin
PETIBM2D = $(BIN_DIR)/PetIBM2d
PETIBM3D = $(BIN_DIR)/PetIBM3d

LIB_DIR = $(PETIBM_DIR)/lib
LIBS = $(addprefix $(LIB_DIR)/, libclasses.a libsolvers.a)
EXT_LIBS = $(addprefix $(LIB_DIR)/, libyaml.a libgtest.a)

EXT_DIR = $(PETIBM_DIR)/external
YAML_OBJS = $(shell find $(EXT_DIR)/yaml-cpp-0.5.1 -type f -name *.o)
GTEST_OBJS = $(shell find $(EXT_DIR)/gtest-1.7.0 -type f -name *.o)

.PHONY: ALL

ALL: $(PETIBM2D) $(PETIBM3D)

include $(PETSC_DIR)/conf/variables
include $(PETSC_DIR)/conf/rules

# locations of include files
PETSC_CC_INCLUDES += -I./src/include \
										 -I./src/solvers \
										 -I./external/gtest-1.7.0/include

PCC_FLAGS += -std=c++0x -Wextra -pedantic
CXX_FLAGS += -std=c++0x -Wextra -pedantic
PCC_LINKER_FLAGS += -I./external/gtest-1.7.0/include

$(SRC_DIR)/PetIBM2d.o: $(SRC_DIR)/PetIBM.cpp
	$(PETSC_COMPILE) -D DIMENSIONS=2 $^ -o $@

$(SRC_DIR)/PetIBM3d.o: $(SRC_DIR)/PetIBM.cpp
	$(PETSC_COMPILE) -D DIMENSIONS=3 $^ -o $@

$(PETIBM2D): $(SRC_DIR)/PetIBM2d.o $(LIBS) $(EXT_LIBS)
	@echo "\n$@ - Linking ..."
	@mkdir -p $(BIN_DIR)
	$(CLINKER) $^ -o $@ $(PETSC_SYS_LIB)

$(PETIBM3D): $(SRC_DIR)/PetIBM3d.o $(LIBS) $(EXT_LIBS)
	@echo "\n$@ - Linking ..."
	@mkdir -p $(BIN_DIR)
	$(CLINKER) $^ -o $@ $(PETSC_SYS_LIB)

$(LIBS):
	@echo "\nGenerating static libraries ..."
	@mkdir -p $(LIB_DIR)
	cd src/include; $(MAKE)
	cd src/solvers; $(MAKE)

$(EXT_LIBS):
	@echo "\nGenerating external static libraries ..."
	@mkdir -p $(LIB_DIR)
	cd external/yaml-cpp-0.5.1; $(MAKE)
	cd external/gtest-1.7.0; $(MAKE)

################################################################################

TESTS_DIR = $(PETIBM_DIR)/tests
TESTS_SRCS = $(shell find $(TESTS_DIR) -type f -name *$(SUFFIX))
TESTS_OBJS = $(addsuffix .o, $(basename $(TESTS_SRCS)))
TESTS_BIN = $(TESTS_SRCS:$(SUFFIX)=)

.PHONY: tests

tests: $(TESTS_BIN)
	$(TESTS_DIR)/CartesianMeshTest
	$(TESTS_DIR)/NavierStokesTest -caseFolder tests/NavierStokes \
												 				-sys2_pc_type gamg -sys2_pc_gamg_type agg \
												 				-sys2_pc_gamg_agg_nsmooths 1
	$(TESTS_DIR)/TairaColoniusTest -caseFolder tests/TairaColonius \
																 -sys2_pc_type gamg -sys2_pc_gamg_type agg \
																 -sys2_pc_gamg_agg_nsmooths 1

$(TESTS_DIR)/CartesianMeshTest: $(TESTS_DIR)/CartesianMeshTest.cpp $(LIBS) $(EXT_LIBS)
	$(CXX) $(PETSC_CC_INCLUDES) -std=c++0x -pthread $^ -o $@ $(PETSC_SYS_LIB)

$(TESTS_DIR)/NavierStokesTest: $(TESTS_DIR)/NavierStokesTest.cpp $(LIBS) $(EXT_LIBS)
	$(CXX) $(PETSC_CC_INCLUDES) -std=c++0x -pthread $^ -o $@ $(PETSC_SYS_LIB)

$(TESTS_DIR)/TairaColoniusTest: $(TESTS_DIR)/TairaColoniusTest.cpp $(LIBS) $(EXT_LIBS)
	$(CXX) $(PETSC_CC_INCLUDES) -std=c++0x -pthread $^ -o $@ $(PETSC_SYS_LIB)

################################################################################

DOC_DIR = $(PETIBM_DIR)/doc
DOXYGEN = doxygen

.PHONY: doc

doc:
	@echo "\nGenerating Doxygen documentation ..."
	cd $(DOC_DIR); $(DOXYGEN) Doxyfile

################################################################################

CLEANFILES = $(OBJ) $(YAML_OBJS) $(GTEST_OBJS) $(TESTS_OBJS) $(TESTS_BIN)

.PHONY: clean cleanpetibm cleantests cleandoc cleanoutput cleanall

cleanall: clean cleanpetibm cleantests cleandoc cleanoutput

cleanpetibm:
	@echo "\nCleaning PetIBM ..."
	$(RM) -rf $(BIN_DIR) $(LIB_DIR)

cleantests:
	@echo "\nCleaning tests ..."
	$(RM) -f $(TESTS_BIN)

cleandoc:
	@echo "\nCleaning documentation ..."
	find $(DOC_DIR) ! -name 'Doxyfile' -type f -delete
	find $(DOC_DIR)/* ! -name 'Doxyfile' -type d -delete

cleanoutput:
	@echo "\nCleaning outputs ..."
	find . -name '*.d' -exec rm -rf {} \;
	find ./cases -name '*.txt' -exec rm -rf {} \;
	find ./cases -name '0*' -prune -exec rm -rf {} \;
	find ./cases -name 'images' -prune -exec rm -rf {} \;
	find ./cases -name 'vtk_files' -prune -exec rm -rf {} \;
	find ./cases -name 'data' -prune -exec rm -rf {} \;
	find ./tests -name '*.txt' -exec rm -rf {} \;
	find . -name '._*' -exec rm -rf {} \;
	find . -name '.DS_Store' -exec rm -rf {} \;

################################################################################

.PHONY: variables

variables:
	@echo "\n-> PetIBM:"
	@echo SRCS: $(SRCS)
	@echo OBJ: $(OBJ)
	@echo CLEANFILES: $(CLEANFILES)
	@echo "\n-> PETSc variables:"
	@echo PETSC_DIR: $(PETSC_DIR)
	@echo PETSC_ARCH: $(PETSC_ARCH)
	@echo PETSC_COMPILE_SINGLE: $(PETSC_COMPILE_SINGLE)
	@echo PETSC_CC_INCLUDES: $(PETSC_CC_INCLUDES)
	@echo "\n-> Compilers:"
	@echo CXX: $(CXX)
	@echo PCC: $(PCC)
	@echo MPIEXEC: $(MPIEXEC)
	@echo "\n-> Linkers:"
	@echo CLINKER: $(CLINKER)
	@echo PCC_LINKER: $(PCC_LINKER)
	@echo "\n-> Flags:"
	@echo PCC_LINKER_FLAGS: $(PCC_LINKER_FLAGS)
	@echo CFLAGS: $(CFLAGS)
	@echo CC_FLAGS: $(CC_FLAGS)
	@echo PCC_FLAGS: $(PCC_FLAGS)
	@echo CXX_FLAGS: $(CXX_FLAGS)
	@echo CPPFLAGS: $(CPPFLAGS)
	@echo CPP_FLAGS: $(CPP_FLAGS)
	@echo CCPPFLAGS: $(CCPPFLAGS)
	@echo "\n-> Commands:"
	@echo RM: $(RM)
	@echo MV: $(MV)
	@echo MAKE: $(MAKE)
	@echo MFLAGS: $(MFLAGS)
	@echo OMAKE: $(OMAKE)
	@echo AR: $(AR)
	@echo ARFLAGS: $(ARFLAGS)
	@echo RANLIB: $(RANLIB)
	
################################################################################

### Two-dimensional cases ###

cavity2dRe100Serial:
	${MPIEXEC} -n 1 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re100 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity2dRe100Parallel:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re100 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity2dRe100NonUniform:
	${MPIEXEC} -n 1 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re100NonUniform -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity2dRe1000:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re1000 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity2dRe3200:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re3200 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity2dRe5000:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/lidDrivenCavity/Re5000 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinder2dRe40:
	${MPIEXEC} -n 2 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re40 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinder2dRe40PeriodicDomain:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re40PeriodicDomain -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinder2dRe150:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re150 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinder2dRe250:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re250 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinderRe550:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re550 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinderRe3000:
	${MPIEXEC} -n 4 $(PETIBM2D) -caseFolder cases/2d/cylinder/Re3000 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

memoryCheck2dSerial:
	${MPIEXEC} -n 1 valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes $(PETIBM2D) -caseFolder cases/2d/memoryTest

memoryCheck2dParallel:
	${MPIEXEC} -n 2 valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes $(PETIBM2D) -caseFolder cases/2d/memoryTest

memoryCheck2dBodySerial:
	${MPIEXEC} -n 1 valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes $(PETIBM2D) -caseFolder cases/2d/memoryTestBody

### Three-dimensional cases ###

cavity3dRe100PeriodicX:
	${MPIEXEC} -n 4 $(PETIBM3D) -caseFolder cases/3d/lidDrivenCavity/Re100PeriodicX -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity3dRe100PeriodicY:
	${MPIEXEC} -n 4 $(PETIBM3D) -caseFolder cases/3d/lidDrivenCavity/Re100PeriodicY -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cavity3dRe100PeriodicZ:
	${MPIEXEC} -n 4 $(PETIBM3D) -caseFolder cases/3d/lidDrivenCavity/Re100PeriodicZ -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

cylinder3dRe40:
	${MPIEXEC} -n 4 $(PETIBM3D) -caseFolder cases/3d/cylinder/Re40 -sys2_pc_type gamg -sys2_pc_gamg_type agg -sys2_pc_gamg_agg_nsmooths 1

memoryCheck3dSerial:
	${MPIEXEC} -n 1 valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes $(PETIBM3D) -caseFolder cases/3d/memoryTest

memoryCheck3dParallel:
	${MPIEXEC} -n 2 valgrind --tool=memcheck --leak-check=full --show-reachable=yes --track-origins=yes $(PETIBM3D) -caseFolder cases/3d/memoryTest