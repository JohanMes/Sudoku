#define IDI_MAINICON 1

#define ID_SOLVE 1001
#define ID_OPEN 1002
#define ID_CLEAR 1003
#define ID_LOG 1004
#define ID_THREADS 1005
#define ID_THREADSTEXT 1006
#define ID_PASSES 1007
#define ID_PASSESTEXT 1008

typedef struct {
	unsigned int numpasses;
	ClassSolver *Solver;
} ArgStruct;
