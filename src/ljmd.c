/*
 * simple lennard-jones potential MD code with velocity verlet.
 * units: Length=Angstrom, Mass=amu; Energy=kcal
 *
 * baseline c version.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "ljmd.h"


#define BLEN 200

/* a few physical constants */
const double kboltz=0.0019872067;     /* boltzman constant in kcal/mol/K */
const double mvsq2e=2390.05736153349; /* m*v^2 in kcal/mol */

int read_from_py(int atoms, double mass, double rcut, double box, double epsilon,
   double sigma, char * restfile, char * trajfile, char * ergfile,
   int nsteps, double dt, mdsys_t * sys)
{
  FILE *fp;
  int i;

    /* read input file */
    sys->natoms=atoms;
    sys->mass=mass;
    sys->epsilon=epsilon;
    sys->sigma=sigma;
    sys->rcut=rcut;
    sys->box=box;
    sys->nsteps=nsteps;
    sys->dt=dt;
    //*nprint=atoi(line);


    /* allocate memory */
    sys->rx=(double *)malloc(sys->natoms*sizeof(double));
    sys->ry=(double *)malloc(sys->natoms*sizeof(double));
    sys->rz=(double *)malloc(sys->natoms*sizeof(double));
    sys->vx=(double *)malloc(sys->natoms*sizeof(double));
    sys->vy=(double *)malloc(sys->natoms*sizeof(double));
    sys->vz=(double *)malloc(sys->natoms*sizeof(double));
    sys->fx=(double *)malloc(sys->natoms*sizeof(double));
    sys->fy=(double *)malloc(sys->natoms*sizeof(double));
    sys->fz=(double *)malloc(sys->natoms*sizeof(double));

    fp=fopen(restfile,"r");
    /* read restart */
    if(fp) {
        for (i=0; i<sys->natoms; ++i) {
            fscanf(fp,"%lf%lf%lf",sys->rx+i, sys->ry+i, sys->rz+i);
        }
        for (i=0; i<sys->natoms; ++i) {
            fscanf(fp,"%lf%lf%lf",sys->vx+i, sys->vy+i, sys->vz+i);
        }
        fclose(fp);
        azzero(sys->fx, sys->natoms);
        azzero(sys->fy, sys->natoms);
        azzero(sys->fz, sys->natoms);
    } else {
        perror("cannot read restart file");
        return 3;
    }

    return 0;
}


int compute_ljmd(int atoms, double mass, double rcut,  double box, double epsilon,
   double sigma, char restfile[BLEN], char trajfile[BLEN], char ergfile[BLEN], int nsteps, double dt, int nprint)
{


    FILE *traj,*erg;
    mdsys_t sys;


    read_from_py(atoms, mass, rcut, box, epsilon, sigma, restfile, trajfile, ergfile,
       nsteps, dt, &sys);


    /* initialize forces and energies.*/
    sys.nfi=0;
    force(&sys);
    ekin(&sys);

    erg=fopen(ergfile,"w");
    traj=fopen(trajfile,"w");

    printf("Starting simulation with %d atoms for %d steps.\n",sys.natoms, sys.nsteps);
    printf("     NFI            TEMP            EKIN                 EPOT              ETOT\n");
    output(&sys, erg, traj);

    /**************************************************/
    /* main MD loop */
    for(sys.nfi=1; sys.nfi <= sys.nsteps; ++sys.nfi) {

        /* write output, if requested */
        if ((sys.nfi % nprint) == 0)
            output(&sys, erg, traj);

        /* propagate system and recompute energies */
        velverlet(&sys);
        ekin(&sys);
    }
    /**************************************************/

    /* clean up: close files, free memory */
    printf("Simulation Done.\n");
    fclose(erg);
    fclose(traj);

    free(sys.rx);
    free(sys.ry);
    free(sys.rz);
    free(sys.vx);
    free(sys.vy);
    free(sys.vz);
    free(sys.fx);
    free(sys.fy);
    free(sys.fz);

    return 0;
}
