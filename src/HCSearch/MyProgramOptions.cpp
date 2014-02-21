#include <iostream>
#include "MyProgramOptions.hpp"
#include "../HCSearchLib/HCSearch.hpp"

namespace MyProgramOptions
{
	ProgramOptions::ProgramOptions()
	{
		// input and output directories

		inputDir = "";
		outputDir = "";
		splitsFolderName = "splits";

		// time bound

		timeBound = 0;

		// schedule or demo

		printUsageMode = false;
		demoMode = false;
		schedule = vector< HCSearch::SearchType >();

		// options

		searchProcedureMode = GREEDY;
		heuristicFeaturesMode = STANDARD;
		costFeaturesMode = STANDARD;
		initialFunctionMode = LOG_REG;
		successorsMode = STOCHASTIC_CONFIDENCES_NEIGHBORS;
		lossMode = HAMMING;

		stochasticCutMode = EDGES;
		beamSize = 1;
		cutParam = 1.0;

		saveAnytimePredictions = true;
		rankLearnerType = HCSearch::SVM_RANK;
		saveFeaturesFiles = false;
		numTrainIterations = 1;
		numTestIterations = 1;
		verboseMode = true;
		boundSuccessorCandidates = 100;
		uniqueIterId = 0;
		saveOutputMask = false;
	}

	ProgramOptions ProgramOptions::parseArguments(int argc, char* argv[])
	{
		ProgramOptions po;

		if ((argc >= 2 && strcmp(argv[1], "--help") == 0) || 
			(argc >= 3 && strcmp(argv[2], "--help") == 0) || 
			(argc >= 4 && strcmp(argv[3], "--help") == 0))
		{
			po.printUsageMode = true;
			return po;
		}

		if (argc < 4)
		{
			if (HCSearch::Global::settings->RANK == 0)
			{
				LOG(ERROR) << "Too few arguments!";
				printUsage();
			}
			HCSearch::abort();
		}
		po.inputDir = argv[1];
		po.outputDir = argv[2];
		po.timeBound = atoi(argv[3]);

		for (int i = 4; i < argc; i++)
		{
			if (strcmp(argv[i], "--help") == 0)
			{
				po.printUsageMode = true;
				break;
			}
			else if (strcmp(argv[i], "--demo") == 0)
			{
				po.demoMode = true;
			}
			else if (strcmp(argv[i], "--splits-path") == 0)
			{
				if (i + 1 != argc)
				{
					po.splitsFolderName = argv[i+1];
				}
			}
			else if (strcmp(argv[i], "--learn") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "H") == 0 || strcmp(argv[i+1], "h") == 0)
						po.schedule.push_back(HCSearch::LEARN_H);
					else if (strcmp(argv[i+1], "C") == 0 || strcmp(argv[i+1], "c") == 0)
						po.schedule.push_back(HCSearch::LEARN_C);
					else if (strcmp(argv[i+1], "COH") == 0 || strcmp(argv[i+1], "coh") == 0)
						po.schedule.push_back(HCSearch::LEARN_C_ORACLE_H);
					else if (strcmp(argv[i+1], "CRH") == 0 || strcmp(argv[i+1], "crh") == 0)
						po.schedule.push_back(HCSearch::LEARN_C_RANDOM_H);
					else if (strcmp(argv[i+1], "ALL") == 0 || strcmp(argv[i+1], "all") == 0)
					{
						po.schedule.push_back(HCSearch::LEARN_H);
						po.schedule.push_back(HCSearch::LEARN_C);
						po.schedule.push_back(HCSearch::LEARN_C_ORACLE_H);
					}
					else
					{
						po.schedule.push_back(HCSearch::LEARN_H);
						po.schedule.push_back(HCSearch::LEARN_C);
					}
				}
				else
				{
					po.schedule.push_back(HCSearch::LEARN_H);
					po.schedule.push_back(HCSearch::LEARN_C);
				}
			}
			else if (strcmp(argv[i], "--infer") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "HC") == 0 || strcmp(argv[i+1], "hc") == 0)
						po.schedule.push_back(HCSearch::HC);
					else if (strcmp(argv[i+1], "HL") == 0 || strcmp(argv[i+1], "hl") == 0)
						po.schedule.push_back(HCSearch::HL);
					else if (strcmp(argv[i+1], "LC") == 0 || strcmp(argv[i+1], "lc") == 0)
						po.schedule.push_back(HCSearch::LC);
					else if (strcmp(argv[i+1], "LL") == 0 || strcmp(argv[i+1], "ll") == 0)
						po.schedule.push_back(HCSearch::LL);
					else if (strcmp(argv[i+1], "RL") == 0 || strcmp(argv[i+1], "rl") == 0)
						po.schedule.push_back(HCSearch::RL);
					else if (strcmp(argv[i+1], "RC") == 0 || strcmp(argv[i+1], "rc") == 0)
						po.schedule.push_back(HCSearch::RC);
					else if (strcmp(argv[i+1], "ALL") == 0 || strcmp(argv[i+1], "all") == 0)
					{
						po.schedule.push_back(HCSearch::HC);
						po.schedule.push_back(HCSearch::HL);
						po.schedule.push_back(HCSearch::LC);
						po.schedule.push_back(HCSearch::LL);
					}
					else
					{
						po.schedule.push_back(HCSearch::HC);
					}
				}
				else
				{
					po.schedule.push_back(HCSearch::HC);
				}
			}
			else if (strcmp(argv[i], "--learner") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "svmrank") == 0)
						po.rankLearnerType = HCSearch::SVM_RANK;
					else if (strcmp(argv[i+1], "online") == 0)
						po.rankLearnerType = HCSearch::ONLINE_RANK;
				}
			}
			else if (strcmp(argv[i], "--search") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "greedy") == 0)
						po.searchProcedureMode = GREEDY;
					else if (strcmp(argv[i+1], "beam") == 0)
						po.searchProcedureMode = BREADTH_BEAM;
					else if (strcmp(argv[i+1], "breadthbeam") == 0)
						po.searchProcedureMode = BREADTH_BEAM;
					else if (strcmp(argv[i+1], "bestbeam") == 0)
						po.searchProcedureMode = BEST_BEAM;
				}
			}
			else if (strcmp(argv[i], "--beam-size") == 0)
			{
				if (i + 1 != argc)
				{
					po.beamSize = atoi(argv[i+1]);
					if (po.beamSize <= 0)
					{
						LOG(ERROR) << "Invalid beam size!";
						HCSearch::abort();
					}
				}
			}
			else if (strcmp(argv[i], "--successor") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "flipbit") == 0)
						po.successorsMode = FLIPBIT;
					else if (strcmp(argv[i+1], "flipbit-neighbors") == 0)
						po.successorsMode = FLIPBIT_NEIGHBORS;
					else if (strcmp(argv[i+1], "flipbit-confidences-neighbors") == 0)
						po.successorsMode = FLIPBIT_CONFIDENCES_NEIGHBORS;
					else if (strcmp(argv[i+1], "stochastic") == 0)
						po.successorsMode = STOCHASTIC;
					else if (strcmp(argv[i+1], "stochastic-neighbors") == 0)
						po.successorsMode = STOCHASTIC_NEIGHBORS;
					else if (strcmp(argv[i+1], "stochastic-confidences-neighbors") == 0)
						po.successorsMode = STOCHASTIC_CONFIDENCES_NEIGHBORS;
					else if (strcmp(argv[i+1], "cut-schedule") == 0)
						po.successorsMode = CUT_SCHEDULE;
					else if (strcmp(argv[i+1], "cut-schedule-neighbors") == 0)
						po.successorsMode = CUT_SCHEDULE_NEIGHBORS;
					else if (strcmp(argv[i+1], "cut-schedule-confidences-neighbors") == 0)
						po.successorsMode = CUT_SCHEDULE_CONFIDENCES_NEIGHBORS;
				}
			}
			else if (strcmp(argv[i], "--cut-param") == 0)
			{
				if (i + 1 != argc)
				{
					po.cutParam = atof(argv[i+1]);
				}
			}
			else if (strcmp(argv[i], "--anytime") == 0)
			{
				po.saveAnytimePredictions = true;
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "false") == 0)
						po.saveAnytimePredictions = false;
				}
			}
			else if (strcmp(argv[i], "--save-features") == 0)
			{
				po.saveFeaturesFiles = true;
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "false") == 0)
						po.saveFeaturesFiles = false;
				}
			}
			else if (strcmp(argv[i], "--num-train-iters") == 0)
			{
				if (i + 1 != argc)
				{
					po.numTrainIterations = atoi(argv[i+1]);
					if (po.numTrainIterations <= 0)
					{
						LOG(ERROR) << "Invalid number of iterations!";
						HCSearch::abort();
					}
				}
			}
			else if (strcmp(argv[i], "--num-test-iters") == 0)
			{
				if (i + 1 != argc)
				{
					po.numTestIterations = atoi(argv[i+1]);
					if (po.numTestIterations <= 0)
					{
						LOG(ERROR) << "Invalid number of iterations!";
						HCSearch::abort();
					}
				}
			}
			else if (strcmp(argv[i], "--verbose") == 0)
			{
				po.verboseMode = true;
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "false") == 0)
						po.verboseMode = false;
				}
			}
			else if (strcmp(argv[i], "--bound-successor") == 0)
			{
				if (i + 1 != argc)
				{
					po.boundSuccessorCandidates = atoi(argv[i+1]);
					if (po.boundSuccessorCandidates <= 0)
					{
						LOG(ERROR) << "Invalid bound!";
						HCSearch::abort();
					}
				}
			}
			else if (strcmp(argv[i], "--cut-mode") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "state") == 0)
						po.stochasticCutMode = STATE;
					else if (strcmp(argv[i+1], "edges") == 0)
						po.stochasticCutMode = EDGES;
				}
			}
			else if (strcmp(argv[i], "--unique-iter") == 0)
			{
				if (i + 1 != argc)
				{
					po.uniqueIterId = atoi(argv[i+1]);
					if (po.uniqueIterId < 0)
					{
						LOG(ERROR) << "Unique iteration ID needs to be >= 0";
						HCSearch::abort();
					}
				}
			}
			else if (strcmp(argv[i], "--save-mask") == 0)
			{
				po.saveOutputMask = true;
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "false") == 0)
						po.saveOutputMask = false;
				}
			}
			else if (strcmp(argv[i], "--hfeatures") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "standard") == 0)
						po.heuristicFeaturesMode = STANDARD;
					else if (strcmp(argv[i+1], "standard2") == 0)
						po.heuristicFeaturesMode = STANDARD2;
					else if (strcmp(argv[i+1], "standard3") == 0)
						po.heuristicFeaturesMode = STANDARD3;
					else if (strcmp(argv[i+1], "dense-crf") == 0)
						po.heuristicFeaturesMode = DENSE_CRF;
					else if (strcmp(argv[i+1], "unary") == 0)
						po.heuristicFeaturesMode = UNARY;
					else if (strcmp(argv[i+1], "unary2") == 0)
						po.heuristicFeaturesMode = UNARY2;
					else if (strcmp(argv[i+1], "standard-bigram") == 0)
						po.heuristicFeaturesMode = STANDARD_BIGRAM;
					else if (strcmp(argv[i+1], "standard3-bigram") == 0)
						po.heuristicFeaturesMode = STANDARD3_BIGRAM;
				}
			}
			else if (strcmp(argv[i], "--cfeatures") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "standard") == 0)
						po.costFeaturesMode = STANDARD;
					else if (strcmp(argv[i+1], "standard2") == 0)
						po.costFeaturesMode = STANDARD2;
					else if (strcmp(argv[i+1], "standard3") == 0)
						po.costFeaturesMode = STANDARD3;
					else if (strcmp(argv[i+1], "dense-crf") == 0)
						po.costFeaturesMode = DENSE_CRF;
					else if (strcmp(argv[i+1], "unary") == 0)
						po.costFeaturesMode = UNARY;
					else if (strcmp(argv[i+1], "unary2") == 0)
						po.costFeaturesMode = UNARY2;
					else if (strcmp(argv[i+1], "standard-bigram") == 0)
						po.costFeaturesMode = STANDARD_BIGRAM;
					else if (strcmp(argv[i+1], "standard3-bigram") == 0)
						po.costFeaturesMode = STANDARD3_BIGRAM;
				}
			}
			else if (strcmp(argv[i], "--loss") == 0)
			{
				if (i + 1 != argc)
				{
					if (strcmp(argv[i+1], "hamming") == 0)
						po.lossMode = HAMMING;
					else if (strcmp(argv[i+1], "pixel-hamming") == 0)
						po.lossMode = PIXEL_HAMMING;
				}
			}
		}

		// demo mode if nothing specified or used --demo flag
		if (po.schedule.empty() && !po.demoMode)
			po.demoMode = true;
		else if (po.demoMode)
			po.schedule.clear();

		return po;
	}

	void ProgramOptions::printUsage()
	{
		cerr << endl;
		cerr << "Program usage: ./HCSearch INPUT_DIR OUTPUT_DIR TIMEBOUND "
			<< "[--learn (H|C|COH)]* [--infer (HC|HL|LC|LL)]* ... [--option=value]" << endl;

		cerr << "Main options:" << endl;
		cerr << "\t--help\t\t" << ": produce help message" << endl;
		cerr << "\t--demo\t\t" << ": run the demo program (ignores --learn and --infer)" << endl;
		cerr << "\t--learn arg\t" << ": learning" << endl;
		cerr << "\t\t\t\tH: learn heuristic" << endl;
		cerr << "\t\t\t\tC: learn cost" << endl;
		cerr << "\t\t\t\tCOH: learn cost with oracle H" << endl;
		cerr << "\t\t\t\tCRH: learn cost with random H" << endl;
		cerr << "\t\t\t\tALL: short-hand for H, C, COH" << endl;
		cerr << "\t\t\t\t(none): short-hand for H, C" << endl;
		cerr << "\t--infer arg\t" << ": inference" << endl;
		cerr << "\t\t\t\tHC: learned heuristic and cost" << endl;
		cerr << "\t\t\t\tHL: learned heuristic and oracle cost" << endl;
		cerr << "\t\t\t\tLC: oracle heuristic and learned cost" << endl;
		cerr << "\t\t\t\tLL: oracle heuristic and cost" << endl;
		cerr << "\t\t\t\tRL: random heuristic and oracle cost" << endl;
		cerr << "\t\t\t\tRC: random heuristic and learned cost" << endl;
		cerr << "\t\t\t\tALL: short-hand for HC, HL, LC, LL" << endl;
		cerr << "\t\t\t\t(none): short-hand for HC" << endl;
		cerr << endl;

		cerr << "Advanced options:" << endl;
		cerr << "\t--anytime arg\t\t" << ": turn on saving anytime predictions if true" << endl;
		cerr << "\t--beam-size arg\t\t" << ": beam size for beam search" << endl;
		cerr << "\t--bound-successor arg\t" << ": maximum number of successor candidates (default=1000)" << endl;
		cerr << "\t--cut-mode arg\t\t" << ": edges|state (cut edges by edges independently or by state)" << endl;
		cerr << "\t--cut-param arg\t\t" << ": temperature parameter for stochastic cuts" << endl;
		cerr << "\t--hfeatures arg\t\t" << ": standard|standard2|standard3|dense-crf|unary|unary2|standard-bigram|standard3-bigram" << endl;
		cerr << "\t--cfeatures arg\t\t" << ": standard|standard2|standard3|dense-crf|unary|unary2|standard-bigram|standard3-bigram" << endl;
		cerr << "\t--num-test-iters arg\t" << ": number of test iterations" << endl;
		cerr << "\t--num-train-iters arg\t" << ": number of training iterations" << endl;
		cerr << "\t--learner arg\t\t" << ": svmrank|online" << endl;
		cerr << "\t--loss arg\t\t" << ": hamming|pixel-hamming" << endl;
		cerr << "\t--save-features arg\t" << ": save rank features during learning if true" << endl;
		cerr << "\t--save-mask arg\t\t" << ": save final prediction label masks if true" << endl;
		cerr << "\t--search arg\t\t" << ": greedy|breadthbeam|bestbeam" << endl;
		cerr << "\t--splits-path arg\t" << ": specify alternate path to splits folder" << endl;
		cerr << "\t--successor arg\t\t" << ": flipbit|flipbit-neighbors|flipbit-confidences-neighbors|"
			<< "stochastic|stochastic-neighbors|stochastic-confidences-neighbors|"
			<< "cut-schedule|cut-schedule-neighbors|cut-schedule-confidences-neighbors" << endl;
		cerr << "\t--unique-iter arg\t" << ": unique iteration ID (num-test-iters needs to be 1)" << endl;
		cerr << "\t--verbose arg\t\t" << ": turn on verbose output if true" << endl;
		cerr << endl;

		cerr << "Notes:" << endl;
		cerr << "* The first three arguments are required. They are the input directory, output directory and time bound." << endl;
		cerr << "* Can use multiple --infer and --learn options in any order to define a schedule. Must come after the mandatory arguments.";
		cerr << endl << endl;
	}
}