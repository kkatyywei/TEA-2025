#define _CRT_SECURE_NO_WARNINGS
#include "GRB.h"
#include <iostream>

typedef short GRBALPHABET;

#define GRB_ERROR_SERIES 600

using namespace std;

namespace GRB {
#define NS(n) Rule::Chain::N(n)
#define TS(n) Rule::Chain::T(n)	

	Greibach greibach(NS('S'), TS('$'),
		10,
		Rule(NS('S'), GRB_ERROR_SERIES + 0, 
			4,
			Rule::Chain(), 
			// announce type func id ( F ) { N R } S
			Rule::Chain(12, TS('a'), TS('t'), TS('f'), TS('i'), TS('('), NS('F'), TS(')'), TS('{'), NS('N'), NS('R'), TS('}'), NS('S')),
			// announce type func id ( F ) { N } S
			Rule::Chain(11, TS('a'), TS('t'), TS('f'), TS('i'), TS('('), NS('F'), TS(')'), TS('{'), NS('N'), TS('}'), NS('S')),
			// main { N }
			Rule::Chain(4, TS('m'), TS('{'), NS('N'), TS('}'))
		),
		Rule(NS('F'), GRB_ERROR_SERIES + 1, 
			2,
			Rule::Chain(3, TS('t'), TS('i'), NS('C')),
			Rule::Chain()
		),
		Rule(NS('C'), GRB_ERROR_SERIES + 1, 
			2,
			Rule::Chain(4, TS(','), TS('t'), TS('i'), NS('C')),
			Rule::Chain()
		),
		Rule(NS('R'), GRB_ERROR_SERIES + 2, 
			3,
			Rule::Chain(),
			Rule::Chain(3, TS('r'), TS('i'), TS(';')),
			Rule::Chain(3, TS('r'), TS('l'), TS(';'))
		),
		Rule(NS('N'), GRB_ERROR_SERIES + 3, 
			9,
			Rule::Chain(),
			// announce type id ; N
			Rule::Chain(5, TS('a'), TS('t'), TS('i'), TS(';'), NS('N')),
			// announce type id = E ; N
			Rule::Chain(7, TS('a'), TS('t'), TS('i'), TS('='), NS('E'), TS(';'), NS('N')),
			// id = E ; N
			Rule::Chain(5, TS('i'), TS('='), NS('E'), TS(';'), NS('N')),
			// cout ( O ) ; N
			Rule::Chain(6, TS('o'), TS('('), NS('O'), TS(')'), TS(';'), NS('N')),
			// id ( P ) ; N  (вызов функции)
			Rule::Chain(6, TS('i'), TS('('), NS('P'), TS(')'), TS(';'), NS('N')),
			// cycle ( E ) { N } N
			Rule::Chain(8, TS('c'), TS('('), NS('E'), TS(')'), TS('{'), NS('N'), TS('}'), NS('N')),
			// i++; N
			Rule::Chain(4, TS('i'), TS('u'), TS(';'), NS('N')),
			// i--; N
			Rule::Chain(4, TS('i'), TS('d'), TS(';'), NS('N'))
		),

		Rule(NS('E'), GRB_ERROR_SERIES + 4,
			30, 

			Rule::Chain(1, TS('i')),      // i
			Rule::Chain(1, TS('l')),      // l

			Rule::Chain(2, TS('i'), TS('u')),             
			Rule::Chain(2, TS('i'), TS('d')),             
			Rule::Chain(3, TS('i'), TS('u'), NS('M')),     
			Rule::Chain(3, TS('i'), TS('d'), NS('M')),    

			Rule::Chain(2, TS('i'), NS('M')), 
			Rule::Chain(2, TS('l'), NS('M')), 

			Rule::Chain(3, TS('('), NS('E'), TS(')')),
			Rule::Chain(4, TS('('), NS('E'), TS(')'), NS('M')),

			// 4. Унарный минус (-)
			Rule::Chain(2, TS('-'), TS('i')),
			Rule::Chain(2, TS('-'), TS('l')),
			Rule::Chain(3, TS('-'), TS('i'), NS('M')),
			Rule::Chain(3, TS('-'), TS('l'), NS('M')),
			Rule::Chain(4, TS('-'), TS('('), NS('E'), TS(')')),
			Rule::Chain(5, TS('-'), TS('('), NS('E'), TS(')'), NS('M')),

			// 5. Логическое отрицание (!)
			Rule::Chain(2, TS('!'), TS('i')),
			Rule::Chain(2, TS('!'), TS('l')),
			Rule::Chain(3, TS('!'), TS('i'), NS('M')),
			Rule::Chain(3, TS('!'), TS('l'), NS('M')),
			Rule::Chain(4, TS('!'), TS('('), NS('E'), TS(')')),
			Rule::Chain(5, TS('!'), TS('('), NS('E'), TS(')'), NS('M')),

			// 6. Битовая инверсия (~)
			Rule::Chain(2, TS('~'), TS('i')),
			Rule::Chain(2, TS('~'), TS('l')),
			Rule::Chain(3, TS('~'), TS('i'), NS('M')),
			Rule::Chain(3, TS('~'), TS('l'), NS('M')),
			Rule::Chain(4, TS('~'), TS('('), NS('E'), TS(')')),
			Rule::Chain(5, TS('~'), TS('('), NS('E'), TS(')'), NS('M')),

			// 7. Вызов функции
			Rule::Chain(4, TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('i'), TS('('), NS('P'), TS(')'), NS('M'))
		),

		// NS('M'): Продолжение выражения
		Rule(NS('M'), GRB_ERROR_SERIES + 5,
			52,

			Rule::Chain(2, TS('+'), TS('i')), Rule::Chain(2, TS('+'), TS('l')),
			Rule::Chain(3, TS('+'), TS('i'), NS('M')), Rule::Chain(3, TS('+'), TS('l'), NS('M')),

			Rule::Chain(2, TS('-'), TS('i')), Rule::Chain(2, TS('-'), TS('l')),
			Rule::Chain(3, TS('-'), TS('i'), NS('M')), Rule::Chain(3, TS('-'), TS('l'), NS('M')),

			Rule::Chain(2, TS('*'), TS('i')), Rule::Chain(2, TS('*'), TS('l')),
			Rule::Chain(3, TS('*'), TS('i'), NS('M')), Rule::Chain(3, TS('*'), TS('l'), NS('M')),

			Rule::Chain(2, TS('/'), TS('i')), Rule::Chain(2, TS('/'), TS('l')),
			Rule::Chain(3, TS('/'), TS('i'), NS('M')), Rule::Chain(3, TS('/'), TS('l'), NS('M')),

			Rule::Chain(2, TS('%'), TS('i')), Rule::Chain(2, TS('%'), TS('l')),
			Rule::Chain(3, TS('%'), TS('i'), NS('M')), Rule::Chain(3, TS('%'), TS('l'), NS('M')),

			Rule::Chain(2, TS('>'), TS('i')), Rule::Chain(2, TS('>'), TS('l')),
			Rule::Chain(3, TS('>'), TS('i'), NS('M')), Rule::Chain(3, TS('>'), TS('l'), NS('M')),

			Rule::Chain(2, TS('<'), TS('i')), Rule::Chain(2, TS('<'), TS('l')),
			Rule::Chain(3, TS('<'), TS('i'), NS('M')), Rule::Chain(3, TS('<'), TS('l'), NS('M')),

			Rule::Chain(2, TS('q'), TS('i')), Rule::Chain(2, TS('q'), TS('l')),
			Rule::Chain(3, TS('q'), TS('i'), NS('M')), Rule::Chain(3, TS('q'), TS('l'), NS('M')),

			Rule::Chain(2, TS('w'), TS('i')), Rule::Chain(2, TS('w'), TS('l')),
			Rule::Chain(3, TS('w'), TS('i'), NS('M')), Rule::Chain(3, TS('w'), TS('l'), NS('M')),

			Rule::Chain(2, TS('x'), TS('i')), Rule::Chain(2, TS('x'), TS('l')),
			Rule::Chain(3, TS('x'), TS('i'), NS('M')), Rule::Chain(3, TS('x'), TS('l'), NS('M')),

			Rule::Chain(2, TS('y'), TS('i')), Rule::Chain(2, TS('y'), TS('l')),
			Rule::Chain(3, TS('y'), TS('i'), NS('M')), Rule::Chain(3, TS('y'), TS('l'), NS('M')),
			Rule::Chain(4, TS('+'), TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('+'), TS('i'), TS('('), NS('P'), TS(')'), NS('M')),

			Rule::Chain(4, TS('-'), TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('-'), TS('i'), TS('('), NS('P'), TS(')'), NS('M')),

			Rule::Chain(4, TS('*'), TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('*'), TS('i'), TS('('), NS('P'), TS(')'), NS('M')),

			Rule::Chain(4, TS('/'), TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('/'), TS('i'), TS('('), NS('P'), TS(')'), NS('M')),

			Rule::Chain(4, TS('%'), TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain(5, TS('%'), TS('i'), TS('('), NS('P'), TS(')'), NS('M'))
		),

		Rule(NS('O'), GRB_ERROR_SERIES + 5,
			4,
			Rule::Chain(1, TS('i')),
			Rule::Chain(1, TS('l')),
			Rule::Chain(4, TS('i'), TS('('), NS('P'), TS(')')),
			Rule::Chain()
		),
		Rule(NS('P'), GRB_ERROR_SERIES + 6,
			5,
			Rule::Chain(),
			Rule::Chain(2, TS('i'), NS('K')),
			Rule::Chain(2, TS('l'), NS('K')),
			Rule::Chain(3, TS(','), TS('i'), NS('K')),
			Rule::Chain(3, TS(','), TS('l'), NS('K'))
		),
		Rule(NS('K'), GRB_ERROR_SERIES + 6,
			3,
			Rule::Chain(),
			Rule::Chain(3, TS(','), TS('i'), NS('P')),
			Rule::Chain(3, TS(','), TS('l'), NS('P'))
		)
	);

	Greibach getGreibach() { return greibach; }

	Rule::Chain::Chain(short psize, GRBALPHABET s, ...)
	{
		nt = new GRBALPHABET[size = psize];
		int* p = (int*)&s;
		for (short i = 0; i < size; i++)
			nt[i] = (GRBALPHABET)p[i];
	}

	Rule::Rule(GRBALPHABET pnn, int iderror, short psize, Chain c, ...)
	{
		nn = pnn;
		idError = iderror;
		chains = new Chain[size = psize];
		Chain* p = &c;
		for (short i = 0; i < size; i++)
			chains[i] = p[i];
	}

	Greibach::Greibach(GRBALPHABET pstartN, GRBALPHABET pstbottomT, short psize, Rule r, ...)
	{
		startN = pstartN;
		stbottomT = pstbottomT;
		rules = new Rule[size = psize];
		Rule* p = &r;
		for (short i = 0; i < size; i++)
			rules[i] = p[i];
	}

	short Greibach::getRule(GRBALPHABET pnn, Rule& prule)
	{
		short rc = -1;
		short k = 0;
		while (k < this->size && this->rules[k].nn != pnn)
			k++;
		if (k < this->size)
			prule = this->rules[rc = k];
		return rc;
	}

	Rule Greibach::getRule(short n)
	{
		Rule rc;
		if (n < this->size)
			rc = this->rules[n];
		return rc;
	}

	char* Rule::getCRule(char* b, short nchain)
	{
		char bchain[200];
		b[0] = Chain::alphabet_to_char(this->nn);
		b[1] = '-';
		b[2] = '>';
		b[3] = 0x00;
		chains[nchain].getCChain(bchain);
		strcat_s(b, sizeof(bchain) + 5, bchain);	 
		return b;
	}

	short Rule::getNextChain(GRBALPHABET t, Chain& pchain, short j)
	{
		if (size == 0) return -2;
		short rc = -1;
		while (j < size) {
			if (chains[j].nt[0] == t || chains[j].nt[0] == 0) {
				rc = j;
				break;
			}
			j++;
		}
		if (rc >= 0) pchain = chains[rc];
		return rc;
	}

	char* Rule::Chain::getCChain(char* b)
	{
		for (int i = 0; i < size; i++)
			b[i] = Chain::alphabet_to_char(nt[i]);
		b[size] = 0x00;
		return b;
	}
}