//---------------------------------------------------------------------------
//.............................. FREE SURFACE ...............................
//---------------------------------------------------------------------------
#ifndef __surf_h__
#define __surf_h__
//---------------------------------------------------------------------------

#define _max_layers_ 20

//---------------------------------------------------------------------------

// free surface grid

typedef struct
{
	JacRes *jr;             // global residual context
	DM      DA_SURF;        // free surface grid
	Vec     ltopo, gtopo;   // topography vectors                (local and global)
	Vec     vx, vy, vz;     // velocity vectors                  (local)
	Vec     vpatch, vmerge; // patch and merged velocity vectors (global)

	// flags/parameters
	PetscBool   UseFreeSurf; // free surface activation flag
	PetscScalar InitLevel;   // initial level
	PetscScalar avg_topo;    // average topography
	PetscBool   flat;        // flat free surface flag
	PetscInt    AirPhase;    // air phase number
	PetscScalar MaxAngle;    // maximum angle with horizon (smoothed if larger)

	// erosion/sedimentation parameters
	PetscInt    phase;            // current sediment phase
	PetscInt    ErosionModel;     // [0-none, 1-infinitely fast, ...]
	PetscInt    SedimentModel;    // [0-none, 1-prescribed rate, ...]
	PetscInt    numRateIntervals; // number of sedimentation rate intervals
	PetscInt    numPhaseLayers;   // number of sediment phase layers
	PetscScalar RateDelims [_max_layers_-1]; // rate intervals time delimiters
	PetscScalar PhaseDelims[_max_layers_-1]; // phase layers time delimiters
	PetscScalar sedRates   [_max_layers_  ]; // sedimentation rates
	PetscInt    sedPhases  [_max_layers_  ]; // sediment phases

} FreeSurf;

//---------------------------------------------------------------------------

PetscErrorCode FreeSurfClear(FreeSurf *surf);

PetscErrorCode FreeSurfCreate(FreeSurf *surf, JacRes *jr);

PetscErrorCode FreeSurfReadFromOptions(FreeSurf *surf, Scaling *scal);

PetscErrorCode FreeSurfReadFromFile(FreeSurf *surf, Scaling *scal);

PetscErrorCode FreeSurfDestroy(FreeSurf *surf);

// advect topography on the free surface mesh
PetscErrorCode FreeSurfAdvect(FreeSurf *surf);

// get single velocity component on the free surface
PetscErrorCode FreeSurfGetVelComp(
	FreeSurf *surf,
	PetscErrorCode (*interp)(FDSTAG *, Vec, Vec, InterpFlags),
	Vec vcomp_grid, Vec vcomp_surf);

// advect/interpolate topography of the free surface
PetscErrorCode FreeSurfAdvectTopo(FreeSurf *surf);

// correct phase ratios based on actual position of the free surface
PetscErrorCode FreeSurfGetAirPhaseRatio(FreeSurf *surf);

// apply erosion to the free surface
PetscErrorCode FreeSurfAppErosion(FreeSurf *surf);

// apply sedimentation to the free surface
PetscErrorCode FreeSurfAppSedimentation(FreeSurf *surf);

//---------------------------------------------------------------------------
// SERVICE FUNCTIONS
//---------------------------------------------------------------------------

PetscInt InterpolateTriangle(
	PetscScalar *x,   // x-coordinates of triangle
	PetscScalar *y,   // y-coordinates of triangle
	PetscScalar *f,   // interpolated field
	PetscInt    *i,   // indices of triangle corners
	PetscScalar  xp,  // x-coordinate of point
	PetscScalar  yp,  // y-coordinate of point
	PetscScalar  tol, // relative tolerance
	PetscScalar *fp); // field value in the point

PetscScalar IntersectTriangularPrism(
	PetscScalar *x,     // x-coordinates of prism base
	PetscScalar *y,     // y-coordinates of prism base
	PetscScalar *z,     // z-coordinates of prism top surface
	PetscInt    *i,     // indices of base corners
	PetscScalar  vcell, // total volume of cell
	PetscScalar  bot,   // z-coordinate of bottom plane
	PetscScalar  top,   // z-coordinate of top plane
	PetscScalar  tol);  // relative tolerance

//---------------------------------------------------------------------------
// MACROS
//---------------------------------------------------------------------------

// NOTE! this macro computes double of actual area
#define GET_AREA_TRIANG(x1, x2, x3, y1, y2, y3) PetscAbsScalar((x1-x3)*(y2-y3)-(x2-x3)*(y1-y3))

// NOTE! this macro computes double of actual volume
#define GET_VOLUME_PRISM(x1, x2, x3, y1, y2, y3, z1, z2, z3, level) \
	((z1+z2+z3)/3.0 > level ? ((z1+z2+z3)/3.0-level)*PetscAbsScalar((x1-x3)*(y2-y3)-(x2-x3)*(y1-y3)) : 0)

#define INTERSECT_EDGE(x1, y1, z1, x2, y2, z2, xp, yp, zp, level, dh) \
	zp = level; \
	w  = z1; if(z2 < w) w = z2; if(zp < w) zp = w; \
	w  = z1; if(z2 > w) w = z2; if(zp > w) zp = w; \
	w  = 0.0; \
	if(PetscAbsScalar(z2-z1) > dh) w = (zp-z1)/(z2-z1); \
	xp = x1 + w*(x2-x1); \
	yp = y1 + w*(y2-y1);

//---------------------------------------------------------------------------
#endif

/*
// map uniform free surface onto non-uniform computational grid
PetscErrorCode FreeSurfGetPartition(
	Discret1D    *ds,  // discretization
	PetscScalar   beg, // starting coordinate
	PetscScalar   len, // domain length
	PetscScalar   h,   // target mesh step
	PetscInt     *n,   // total number of nodes
	PetscInt    **l);  // free surface partitioning vector
*/
