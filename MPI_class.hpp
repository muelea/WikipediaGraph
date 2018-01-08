#ifndef MPI_CLASS_H
#define MPI_CLASS_H
#include <mpi.h>

class MPIManager {
  public:
    MPIManager(int argc, char** argv) {
      if (MPI_SUCCESS != MPI_Init(&argc, &argv ))
        throw std::runtime_error("called MPI_Init twice");
    }
    
    ~MPIManager() {
      MPI_Finalize();
    }
};

#endif
