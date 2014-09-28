#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "solver.h"
#include "resource.h"

////////////////////////////////////////////////////////////////////////////////
/*
	Algemeen spul
*/
void ClassSolver::AssignWork(const short (*input)[9]) {
	solveresult = false;

	for(unsigned int i = 0;i < 9;i++) {
		for(unsigned int j = 0;j < 9;j++) {
			matrix[i][j] = input[i][j];
		}
	}
	
	// Stel de werkmatrix op...
	for(unsigned int i = 0;i < 3;i++) {
		for(unsigned int j = 0;j < 3;j++) {
			blkneedwork[i][j] = true;
		}
	}
	numblkstodo = 9;
	
	// Stel de werkmatrix voor rijen en kolommen op...
	for(unsigned int i = 0;i < 9;i++) {
		rowneedwork[i] = true;
		colneedwork[i] = true;
	}
	numrowstodo = 9;
	numcolstodo = 9;
}
////////////////////////////////////////////////////////////////////////////////
/*
	Met blokken oplossen
*/
bool ClassSolver::SolveByBlocks() {
	while(numblkstodo > 0) {
		oldblkworkcount = blkworkcount;
		for(unsigned int X = 0;X < 3;X++) {
			for(unsigned int Y = 0;Y < 3;Y++) {
				if(blkneedwork[Y][X]) {
					FillBlock(X*3,Y*3);
					if(!BlockHasVal(X*3,Y*3,0)) {
						blkneedwork[Y][X] = false;
						numblkstodo--;
					}
				}
			}
		}
		
		// Als we na een volledige pass niks bereikt hebben, stoppen
		if(oldblkworkcount == blkworkcount) {
			return false;
		}
	}
	return true;
}
void ClassSolver::FillBlock(const unsigned int Xin,const unsigned int Yin) {
	for(unsigned int X = Xin;X < Xin+3;X++) {
		for(unsigned int Y = Yin;Y < Yin+3;Y++) {
			
			// Zoek een leeg vakje op
			if(matrix[Y][X] == 0) {

				// Nu gaan we dit gat proberen op te vullen
				for(unsigned int n = 1;n <= 9;n++) {
					
					// Nu weten we dat bijv. 3 niet in dit blok voorkomt...
					if(!BlockHasVal(Xin,Yin,n)) {

						// Probeer nu die waarde in te vullen...
						if(PutInBlock(Xin,Yin,n)) {
							break; // break from the testnumber loop
						}
					}
				}
			}
		}
	}
}
bool ClassSolver::PutInBlock(const unsigned int Xin,const unsigned int Yin,const short val) {
	
	// Nu het blok met als hoek X en Y aflezen en kijken of 
	// we maar één optie over hebben voor dit blok
	unsigned int left = Xin;
	unsigned int top  = Yin;
	
	short lastknownX = 0;
	short lastknownY = 0;
	
	int numpossiblespots = 0;
	
	// Loop door het blokje, en kijk op elke lege plek 
	// of deze met val gevuld mag worden
	for(unsigned int X = left;X < left+3;X++) {
		for(unsigned int Y = top;Y < top+3;Y++) {
			if(matrix[Y][X] == 0) {

				// Eerst deze rij proberen af te werken
				if(!RowHasVal(Y,val)) {
					
					// En dan de kolom...
					if(!ColHasVal(X,val)) {
						numpossiblespots++;

						if(numpossiblespots == 2) {
							return false;
						}
						
						lastknownX = X;
						lastknownY = Y;
					} else {
						
						// Stop gelijk met deze hele Y
						break;
					}
				}
			}
		}
	}
	
	// Dus we weten waar val neergezet kan worden...
	// En er is altijd wel een spot, dus als we hier aanlanden gaat het 
	// sowieso goed
	matrix[lastknownY][lastknownX] = val;
	blkworkcount++;
	return true;
}
bool ClassSolver::BlockHasVal(const unsigned int Xin,const unsigned int Yin,const short val) {
	for(unsigned int X = Xin;X < Xin+3;X++) {
		for(unsigned int Y = Yin;Y < Yin+3;Y++) {
			if(matrix[Y][X] == val) {
				return true;
			}
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/*
	Met rijen oplossen
*/
bool ClassSolver::SolveByRows() {
	while(numrowstodo > 0) {
		oldrowworkcount = rowworkcount;
		for(unsigned int Y = 0;Y < 9;Y++) {
			if(rowneedwork[Y]) {
				FillRow(Y);
				if(!RowHasVal(Y,0)) {
					rowneedwork[Y] = false;
					numrowstodo--;
				}
			}
		}
		
		// Als we na een volledige pass niks bereikt hebben, stoppen
		if(oldrowworkcount == rowworkcount) {
			return false;
		}
	}
	return true;
}
void ClassSolver::FillRow(const unsigned int Yin) {

	// Eerst kijken welke waardes we nog moeten invullen
	for(unsigned int X = 0;X < 9;X++) { // Right
		
		// We hebben een gaatje gevonden, nu kijken of we hier wat mogen dumpen
		if(matrix[Yin][X] == 0) {
			
			for(unsigned int n = 1;n <= 9;n++) { // Test alle getallen
				
				// Als we hier getal neer mogen zetten, vul hem in
				if(!RowHasVal(Yin,n)) {
					if(PutInRow(Yin,n)) {
						break;
					}
				}
			}
		}
	}
}
bool ClassSolver::PutInRow(const unsigned int Yin,const short val) {
	
	// We gaan val proberen hier neer te zetten, dus moet het alleen hier
	// passen...
	int numpossiblespots = 0;

	short lastknownX = 0;
	
	for(unsigned int X = 0;X < 9;X++) {
		if(matrix[Yin][X] == 0) {
			
			// Skip het checken van de rijen, dat is al gedaan
			if(!ColHasVal(X,val)) {
				
				// Hier wel linksboven meegeven
				if(!BlockHasVal(X - X%3,Yin - Yin%3,val)) {
					numpossiblespots++;

					// Al eentje gevonden? Quit
					if(numpossiblespots == 2) {
						return false;
					}
					
					lastknownX = X;
				} else {
					// Skip de rest van dit blok
					X += 2 - X%3;
				}
			}
		}
	}

	matrix[Yin][lastknownX] = val;
	rowworkcount++;
	return true;
}
bool ClassSolver::RowHasVal(const unsigned int Yin,const short val) {

	// X en Y zijn 0-based en geven linksboven aan
	for(unsigned int X = 0;X < 9;X++) {
		if(matrix[Yin][X] == val) {
			return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/*
	Met kolommem oplossen
*/
bool ClassSolver::SolveByCols() {
	while(numcolstodo > 0) {
		oldcolworkcount = colworkcount;
		for(unsigned int X = 0;X < 9;X++) {
			if(colneedwork[X]) {
				FillCol(X);
				if(!ColHasVal(X,0)) {
					colneedwork[X] = false;
					numcolstodo--;
				}
			}
		}
		
		// Als we na een volledige pass niks bereikt hebben, stoppen
		if(oldcolworkcount == colworkcount) {
			return false;
		}
	}
	return true;
}
void ClassSolver::FillCol(const unsigned int Xin) {

	// Eerst kijken welke waardes we nog moeten invullen
	for(unsigned int Y = 0;Y < 9;Y++) {
		
		// We hebben een gaatje gevonden, nu kijken of we hier wat mogen dumpen
		if(matrix[Y][Xin] == 0) {
			
			for(unsigned int n = 1;n <= 9;n++) { // Test alle getallen
				
				// Als we hier getal neer mogen zetten, vul hem in
				if(!ColHasVal(Xin,n)) {
					if(PutInCol(Xin,n)) {
						break;
					}
				}
			}
		}
	}
}
bool ClassSolver::PutInCol(const unsigned int Xin,const short val) {
	
	// We gaan val proberen hier neer te zetten, dus moet het alleen hier
	// passen...
	int numpossiblespots = 0;

	short lastknownY = 0;
	
	for(unsigned int Y = 0;Y < 9;Y++) {
		if(matrix[Y][Xin] == 0) {
			
			// First check this row, and skip testing this col
			if(!RowHasVal(Y,val)) {
				if(!BlockHasVal(Xin - Xin%3,Y - Y%3,val)) {
					numpossiblespots++;
					
					// Al eentje gevonden? Quit
					if(numpossiblespots == 2) {
						return false;
					}
					
					lastknownY = Y;
				} else {
					// Skip de rest van dit blok
					Y += 2 - Y%3;
				}
			}
		}
	}

	matrix[lastknownY][Xin] = val;
	colworkcount++;
	return true;
}
bool ClassSolver::ColHasVal(const unsigned int Xin,const short val) {
	for(unsigned int Y = 0;Y < 9;Y++) { // Down
		if(matrix[Y][Xin] == val) {
			return true;
		}
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////
/*
	Het callertje
*/
bool ClassSolver::Solve() {

	// Probeer alle aanpakken
	do {
		rowworkcount = 0;
		colworkcount = 0;
		blkworkcount = 0;
		if(SolveByBlocks() or SolveByRows() or SolveByCols()) {
			solveresult = true;
			return true;
		}
	} while(rowworkcount||colworkcount||blkworkcount);
	solveresult = false;
	return false;
}

