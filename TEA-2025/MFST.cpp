#include <iomanip>
#include <iostream>
#include <map>
#include "MFST.h"

namespace MFST {
	int counter = 0; // Счетчик шагов 
	int saveCounter = 1; // Счетчик состояний

	Mfst::MfstDiagnosis::MfstDiagnosis() {
		this->lenta_position = -1;
		this->rc_step = SURPRISE;
		this->nrule = -1;
		this->nrule_chain = -1;
	}

	Mfst::MfstDiagnosis::MfstDiagnosis(short plenta_position, RC_STEP prc_step, short pnrule, short pnrule_chain) {
		this->lenta_position = plenta_position;
		this->rc_step = prc_step;
		this->nrule = pnrule;
		this->nrule_chain = pnrule_chain;
	}

	MfstState::MfstState() {
		this->lenta_position = 0;
		this->nrule = -1;
		this->nrulechain = -1;
	}

	// Конструктор с параметрами для инициализации состояния анализатора
	MfstState::MfstState(short pposition, MFSTSTSTACK pst, short pnrulechain) {
		this->lenta_position = pposition;
		this->st = pst;
		this->nrulechain = pnrulechain;
	}

	MfstState::MfstState(short pposition, MFSTSTSTACK pst, short pnrule, short pnrulechain) {
		this->lenta_position = pposition;
		this->st = pst;
		this->nrule = pnrule;
		this->nrulechain = pnrulechain;
	}

	Mfst::Mfst() {
		this->lenta = 0;
		this->lenta_size = 0;
		this->lenta_position = 0;
	}

	Mfst::Mfst(MFST::LEX plex, GRB::Greibach pgrebach, Log::LOG& plog) {
		this->lex = plex;
		this->greibach = pgrebach;
		this->log = plog;
		this->lenta = new short[plex.lextable.size];

		// Заполняем ленту термами из таблицы лексем
		for (int i = 0; i < plex.lextable.size; i++) {
			this->lenta[i] = GRB::Rule::Chain::T(plex.lextable.table[i].lexema);
		}
		this->lenta_size = plex.lextable.size;
		this->lenta_position = 0;
		this->nrulechain = -1;
		this->st.push(greibach.stbottomT);
		this->st.push(greibach.startN);
	}

	// текущее состояние стека
	char* Mfst::getCSt(char* buf) {
		MFSTSTSTACK pst;
		int size = st.size(); // основной стек

		for (int i = 0; i < size; i++) {
			pst.push(st.top());
			st.pop();
			buf[i] = GRB::Rule::Chain::alphabet_to_char(pst.top());
		}

		for (int i = 0; i < size; i++) {
			st.push(pst.top());
			pst.pop();
		}

		buf[size] = 0x00;
		return buf;
	}

	char* Mfst::getCLenta(char* buf, short pos, short n) {
		int i, k;
		if (pos + n < lenta_size) {
			k = pos + n;
		}
		else {
			k = lenta_size;
		}

		for (i = pos; i < k; i++) {
			buf[i - pos] = GRB::Rule::Chain::alphabet_to_char(lenta[i]);
		}
		buf[i - pos] = 0x00;
		return buf;
	}

	// диагностика ошибок
	char* Mfst::getDiagnosis(short n, char* buf, Log::LOG log) {
		if (n < MFST_DIAGN_NUMBER && diagnosis[n].lenta_position >= 0 && diagnosis[n].lenta_position < lenta_size) {
			Error::ERROR error = Error::geterrorin(
				greibach.getRule(diagnosis[n].nrule).idError,
				lex.lextable.table[diagnosis[n].lenta_position].sn,
				lex.lextable.table[diagnosis[n].lenta_position].cn);
			sprintf_s(buf, ERROR_MAXSIZE_MESSAGE, "Ошибка %d: строка %d,%s\n",
				error.id,
				lex.lextable.table[diagnosis[n].lenta_position].sn,
				error.message);
			Log::WriteError(log, error);
		}
		return buf;
	}

	bool Mfst::savestate() {
		storestate.push(MfstState(lenta_position, st, nrule, nrulechain));
		return true;
	}

	bool Mfst::reststate() {
		MfstState state;
		if (storestate.size() > 0) {
			state = storestate.top();
			lenta_position = state.lenta_position;
			st = state.st;
			nrulechain = state.nrulechain;
			storestate.pop();
			return true;
		}
		return false;
	}

	bool Mfst::push_chain(GRB::Rule::Chain chain) {
		for (int i = 0; i < chain.size; i++) {
			st.push(chain.nt[chain.size - 1 - i]);
		};
		return true;
	}

	Mfst::RC_STEP Mfst::step() {
		char* buff = new char[200];
		RC_STEP state = SURPRISE;
		*log.stream << std::endl;
		(*log.stream) << std::setw(4) << std::left << counter << ": ";
		if (lenta_position < lenta_size) {
			if (GRB::Rule::Chain::isN(st.top())) {
				GRB::Rule rule;
				nrule = greibach.getRule(st.top(), rule);
				if (nrule >= 0) {
					GRB::Rule::Chain chain;
					nrulechain = rule.getNextChain(lenta[lenta_position], chain, nrulechain + 1);
					if (nrulechain >= 0) {
						char* buff = new char[200];
						rule.getCRule(buff, nrulechain);
						*log.stream << std::setw(20) << std::left << buff;
						getCLenta(buff, lenta_position);
						*log.stream << std::setw(30) << std::left << buff;
						getCSt(buff);
						*log.stream << std::setw(20) << std::left << buff << std::endl;
						savestate();
						*log.stream << std::setw(4) << std::left << counter << ": " << std::setw(20) << std::left << "SAVESTATE:" << std::setw(30) << std::left << saveCounter++;
						st.pop();
						push_chain(chain);
						state = NS_OK;
					}
					else {
						savediagnosis(NS_NORULECHAIN);
						if (reststate()) {
							state = NS_NORULECHAIN;
							*log.stream << std::setw(20) << std::left << "TS_NOK/NS_NORULECHAIN" << std::endl << std::setw(4) << std::left << counter << ": " << std::setw(20) << std::left << "RESTSTATE";
							saveCounter--;
						}
						else {
							state = NS_NORULE;
						}
					}
				}
			}
			else {
				if (st.top() == lenta[lenta_position]) {
					lenta_position++;
					st.pop();
					nrulechain = -1;
					state = TS_OK;
					char* buff = new char[200];
					*log.stream << std::setw(20) << std::left << "";
					getCLenta(buff, lenta_position);
					*log.stream << std::setw(30) << std::left << buff;
					getCSt(buff);
					*log.stream << std::setw(20) << std::left << buff;
					counter++;

				}
				else {
					if (reststate()) {
						state = TS_NOK;
						*log.stream << std::setw(20) << std::left << "TS_NOK/NS_NORULECHAIN" << std::endl << std::setw(4) << std::left << counter << ": " << std::setw(20) << std::left << "RESTSTATE";
						saveCounter--;
					}
					else {
						state = NS_NORULECHAIN;
						*log.stream << std::setw(20) << std::left << "TS_NOK/NS_NORULECHAIN" << std::endl << std::setw(4) << std::left << counter << ": " << std::setw(20) << std::left << "RESTSTATE";
						saveCounter--;
					}
				}
			}
		}
		else {
			state = LENTA_END;
			*log.stream << std::setw(20) << std::left << "LENTA_END";

		}
		return state;
	}

	bool Mfst::start(Log::LOG log) {
		*log.stream << std::endl << std::setw(4) << std::left << "step" << ": " << std::setw(20) << std::left << "rule" << std::setw(30) << std::left << "lenta" << std::setw(20) << std::left << "stack" << std::endl;
		RC_STEP state = SURPRISE;
		char buff[255];
		state = step();
		while (state == NS_OK || state == NS_NORULECHAIN || state == TS_OK || state == TS_NOK) {
			state = step();
		}
		if (state == LENTA_END) {
			st.pop();
			if (!st.empty()) {
				state = NS_ERROR;
			}
		}
		switch (state)
		{
		case MFST::Mfst::NS_NORULE: {
			*log.stream << std::setw(20) << std::left << "------>NO_RULE" << std::endl;
			*log.stream << "-------------------------------------------------------------------------------------" << std::endl;
			getDiagnosis(0, buff, log);
			getDiagnosis(1, buff, log);
			getDiagnosis(2, buff, log);


			short bestPos = diagnosis[0].lenta_position;
			short bestRule = diagnosis[0].nrule;
			int bestErrId = (bestRule >= 0 ? greibach.getRule(bestRule).idError : 600);

			for (int di = 0; di < MFST_DIAGN_NUMBER; di++) {
				short rp = diagnosis[di].lenta_position;
				short rr = diagnosis[di].nrule;
				if (rr < 0) continue;
				int rid = greibach.getRule(rr).idError;
				if (rid == 603) { bestErrId = rid; bestPos = rp; bestRule = rr; break; }
			}

			if (bestPos < 0 || bestPos >= lenta_size) {
				bestPos = (lenta_position > 0 ? (short)(lenta_position - 1) : 0);
			}
			throw Error::geterrorin(bestErrId, lex.lextable.table[bestPos].sn, lex.lextable.table[bestPos].cn);

			break;
		}
		case MFST::Mfst::NS_NORULECHAIN: {
			*log.stream << std::setw(20) << std::left << "------>NO_RULECHAIN";
			*log.stream << std::endl;
			*log.stream << "-------------------------------------------------------------------------------------" << std::endl;
			getDiagnosis(0, buff, log);
			getDiagnosis(1, buff, log);
			getDiagnosis(2, buff, log);

			short bestPos = diagnosis[0].lenta_position;
			short bestRule = diagnosis[0].nrule;
			int bestErrId = (bestRule >= 0 ? greibach.getRule(bestRule).idError : 600);

			for (int di = 0; di < MFST_DIAGN_NUMBER; di++) {
				short rp = diagnosis[di].lenta_position;
				short rr = diagnosis[di].nrule;
				if (rr < 0) continue;
				int rid = greibach.getRule(rr).idError;
				if (rid == 603) { bestErrId = rid; bestPos = rp; bestRule = rr; break; }
			}

			if (bestPos < 0 || bestPos >= lenta_size) {
				bestPos = (lenta_position > 0 ? (short)(lenta_position - 1) : 0);
			}
			throw Error::geterrorin(bestErrId, lex.lextable.table[bestPos].sn, lex.lextable.table[bestPos].cn);

			break;
		}
		case MFST::Mfst::NS_ERROR: {
			*log.stream << std::setw(20) << std::left << "------>NS_ERROR";
			*log.stream << std::setw(20) << std::left << "\nОшибка 600. MFST: Неверная структура программы";
			throw ERROR_THROW(600);
			return false;
			break;
		}
		case MFST::Mfst::LENTA_END: {
			*log.stream << std::setw(4) << std::left << counter << std::setw(20) << std::left << "------>LENTA_END" << std::endl;
			*log.stream << "-------------------------------------------------------------------------------------" << std::endl;
			*log.stream << std::setw(4) << std::left << "All lines " << lenta_size << ", syntax analysis is performed without errors\n\n";
			break;

		}
		case MFST::Mfst::SURPRISE: {
			*log.stream << std::setw(20) << std::left << "------>SURPRISE ";
			return false;
			break;
		}
		}
		return true;
	}

	bool Mfst::savediagnosis(RC_STEP pprc_step) {
		int i = 0;
		while (i < MFST_DIAGN_NUMBER && lenta_position <= diagnosis[i].lenta_position) {
			i++;
		}
		if (i < MFST_DIAGN_NUMBER) {
			diagnosis[i] = MfstDiagnosis(lenta_position, pprc_step, nrule, nrulechain);
			for (int j = i + 1; j < MFST_DIAGN_NUMBER; j++) {
				diagnosis[j].lenta_position = -1;
			}
			return true;
		}
		return false;
	}

	void Mfst::printrules() {
		int size = deducation.size;
		if (size <= 0) return;

		// Вектор для хранения состояния каждого шага
		struct StepInfo {
			std::string ruleStr;
			std::string left;      // левая часть правила (без пробелов)
			std::string right;     // правая часть правила (без пробелов)
			bool isEpsilon = false;
			std::vector<int> children; // индексы дочерних шагов
			bool collapsesToEpsilon = false;
			bool processed = false;
		};

		std::vector<StepInfo> stepInfos(size);

		// собираем все правила
		for (int i = 0; i < size; i++) {
			char buff[256] = { 0 };
			GRB::Rule rule = greibach.getRule(deducation.nrules[i]);
			rule.getCRule(buff, deducation.nrulechains[i]);
			stepInfos[i].ruleStr = buff;

			if (stepInfos[i].ruleStr.find("->") != std::string::npos) {
				stepInfos[i].left = stepInfos[i].ruleStr.substr(0, stepInfos[i].ruleStr.find("->"));
				stepInfos[i].right = stepInfos[i].ruleStr.substr(stepInfos[i].ruleStr.find("->") + 2);

				// Убираем пробелы
				stepInfos[i].left.erase(std::remove_if(stepInfos[i].left.begin(), stepInfos[i].left.end(),
					[](char c) { return std::isspace(c); }), stepInfos[i].left.end());
				stepInfos[i].right.erase(std::remove_if(stepInfos[i].right.begin(), stepInfos[i].right.end(),
					[](char c) { return std::isspace(c); }), stepInfos[i].right.end());

				stepInfos[i].isEpsilon = stepInfos[i].right.empty();
				if (stepInfos[i].isEpsilon) {
					stepInfos[i].collapsesToEpsilon = true;
					stepInfos[i].processed = true;
				}
			}
		}

		// 2. Строим дерево вывода правильно
		std::vector<std::pair<int, int>> stack; // (stepIndex, positionInRight)

		for (int i = 0; i < size; i++) {
			if (stepInfos[i].right.empty()) continue;

			// Идем по правой части слева направо
			for (size_t pos = 0; pos < stepInfos[i].right.length(); pos++) {
				char c = stepInfos[i].right[pos];
				if (c >= 'A' && c <= 'Z') {
					stack.push_back(std::make_pair(i, static_cast<int>(pos) + 1));
				}
			}

		}

		// 3. Связываем правила с их развертываниями
		std::vector<int> ntToStep(256, -1); 

		for (int i = 0; i < size; i++) {
			if (stepInfos[i].left.empty()) continue;

			char nt = stepInfos[i].left[0]; // нетерминал слева

			// Ищем в стеке, кто ожидает развертывания этого нетерминала
			for (size_t s = 0; s < stack.size(); s++) {
				int parentIdx = stack[s].first;
				int expectedPos = stack[s].second;

				if (parentIdx < i && expectedPos > 0) {
					std::string parentRight = stepInfos[parentIdx].right;
					if (expectedPos - 1 < static_cast<int>(parentRight.length()) &&
						parentRight[expectedPos - 1] == nt) {

						stepInfos[parentIdx].children.push_back(i);
						stack.erase(stack.begin() + s);
						s--; 
						break; 
					}
				}
			}
		}

		// 4. Рекурсивно проверяем, какие нетерминалы действительно сворачиваются в epsilon
		bool changed;
		do {
			changed = false;
			for (int i = 0; i < size; i++) {
				if (stepInfos[i].processed) continue;

				if (stepInfos[i].right.empty()) {
					continue;
				}

				// Проверяем, все ли дети обработаны
				bool allChildrenProcessed = true;
				for (int childIdx : stepInfos[i].children) {
					if (!stepInfos[childIdx].processed) {
						allChildrenProcessed = false;
						break;
					}
				}

				if (!allChildrenProcessed) continue;

				bool allChildrenEpsilon = true;
				for (int childIdx : stepInfos[i].children) {
					if (!stepInfos[childIdx].collapsesToEpsilon) {
						allChildrenEpsilon = false;
						break;
					}
				}

				bool onlyNonTerminals = true;
				for (char c : stepInfos[i].right) {
					if (!(c >= 'A' && c <= 'Z')) {
						onlyNonTerminals = false;
						break;
					}
				}

				stepInfos[i].collapsesToEpsilon = allChildrenEpsilon && onlyNonTerminals;
				stepInfos[i].processed = true;
				changed = true;
			}
		} while (changed);

		// 5. Формируем вывод, удаляя только те нетерминалы, которые сворачиваются в epsilon
		for (int i = 0; i < size; i++) {
			if (stepInfos[i].isEpsilon) continue;

			std::string ruleStr = stepInfos[i].ruleStr;

			if (ruleStr.find("->") != std::string::npos) {
				std::string leftPart = ruleStr.substr(0, ruleStr.find("->") + 2);
				std::string rightPart = ruleStr.substr(ruleStr.find("->") + 2);

				while (!rightPart.empty() &&
					(rightPart.back() == ' ' || rightPart.back() == '\t')) {
					rightPart.pop_back();
				}

				if (!rightPart.empty()) {
					std::vector<std::pair<int, char>> nonterminals; // (position, char)
					for (int pos = 0; pos < static_cast<int>(rightPart.length()); pos++) {
						if (rightPart[pos] >= 'A' && rightPart[pos] <= 'Z') {
							nonterminals.push_back(std::make_pair(pos, rightPart[pos]));
						}
					}

					// Проверяем нетерминалы справа налево
					for (int ntIdx = static_cast<int>(nonterminals.size()) - 1; ntIdx >= 0; ntIdx--) {
						int pos = nonterminals[ntIdx].first;
						char nt = nonterminals[ntIdx].second;

						// Ищем ребенка для этого нетерминала
						bool foundChild = false;
						bool childIsEpsilon = false;

						for (int childIdx : stepInfos[i].children) {
							if (!stepInfos[childIdx].left.empty() && stepInfos[childIdx].left[0] == nt) {
								foundChild = true;
								childIsEpsilon = stepInfos[childIdx].collapsesToEpsilon;
								break;
							}
						}

						if (foundChild && childIsEpsilon) {
							// Удаляем этот нетерминал
							bool hasSeparatorBefore = false;

							if (pos > 0) {
								char beforeChar = rightPart[pos - 1];
								hasSeparatorBefore = (beforeChar == ';' || beforeChar == ')' ||
									beforeChar == '}' || beforeChar == ',' ||
									beforeChar == ']');
							}

							rightPart.erase(pos, 1);

							for (int j = ntIdx; j < static_cast<int>(nonterminals.size()); j++) {
								if (nonterminals[j].first > pos) {
									nonterminals[j].first--;
								}
							}

							if (hasSeparatorBefore) {
								if (pos < static_cast<int>(rightPart.length()) &&
									(rightPart[pos] == ' ' || rightPart[pos] == '\t')) {
									rightPart.erase(pos, 1);

									for (int j = ntIdx; j < static_cast<int>(nonterminals.size()); j++) {
										if (nonterminals[j].first > pos) {
											nonterminals[j].first--;
										}
									}
								}
							}
						}
						else {
							}
					}
				}

				ruleStr = leftPart + rightPart;
			}

			(*log.stream) << std::setw(4) << std::left << deducation.nsteps[i] << ": " << ruleStr << std::endl;
		}
	}

	bool Mfst::savededucation()
	{
		MfstState state;
		std::stack<MfstState> st;
		GRB::Rule rule;
		deducation.size = (short)storestate.size();
		deducation.nsteps = new short[deducation.size];
		deducation.nrules = new short[deducation.size];
		deducation.nrulechains = new short[deducation.size];
		int size = storestate.size();
		for (unsigned short i = 0; i < size; i++) {
			st.push(storestate.top());
			storestate.pop();
		}
		for (unsigned short i = 0; i < size; i++)
		{
			state = st.top();
			st.pop();
			deducation.nsteps[i] = state.lenta_position;
			deducation.nrules[i] = state.nrule;
			deducation.nrulechains[i] = state.nrulechain;
		};
		return true;
	};
}