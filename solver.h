#ifndef SOLVER_H
#define SOLVER_H

class ClassSolver {
	// Werkbijhouders
	int blkworkcount;
	int rowworkcount;
	int colworkcount;
	
	// Werkbijhouders too
	int oldblkworkcount;
	int oldrowworkcount;
	int oldcolworkcount;

	// Solve by blocks
	bool SolveByBlocks();
	void FillBlock(const unsigned int Xin,const unsigned int Yin);
	bool PutInBlock(const unsigned int Xin,const unsigned int Yin,const short val);
	bool BlockHasVal(const unsigned int Xin,const unsigned int Yin,const short val);
	
	short numblkstodo;
	bool blkneedwork[3][3];
	
	// Solve by rows
	bool SolveByRows();
	void FillRow(const unsigned int Yin);
	bool PutInRow(const unsigned int Yin,const short val);
	bool RowHasVal(const unsigned int Yin,const short val);

	short numrowstodo;
	bool rowneedwork[9];

	// Solve by cols	
	bool SolveByCols();
	void FillCol(const unsigned int Xin);
	bool PutInCol(const unsigned int Xin,const short val);
	bool ColHasVal(const unsigned int Xin,const short val);

	short numcolstodo;
	bool colneedwork[9];

	public:
		// Constructor
		void AssignWork(const short (*input)[9]);
	
		bool Solve();
		
		short matrix[9][9];
		bool solveresult;
};

#endif

