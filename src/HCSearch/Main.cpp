#include <iostream>
#include <ctime>
#include "../HCSearchLib/HCSearch.hpp"
#include "MyFileSystem.hpp"
#include "MyProgramOptions.hpp"
#include "Main.hpp"
#include "Demo.hpp"

using namespace std;
using namespace MyLogger;

int main(int argc, char* argv[])
{
	// initialize HCSearch
	HCSearch::Setup::initialize(argc, argv);

	// parse arguments
	MyProgramOptions::ProgramOptions po = MyProgramOptions::ProgramOptions::parseArguments(argc, argv);

	// print usage
	if (po.printUsageMode)
	{
		if (HCSearch::Global::settings->RANK == 0)
			MyProgramOptions::ProgramOptions::printUsage();
		HCSearch::Setup::finalize();
		return 0;
	}

	// configure settings
	HCSearch::Global::settings->paths->BASE_PATH = po.baseDir;
	HCSearch::Global::settings->paths->INPUT_SPLITS_FOLDER_NAME = po.splitsFolderName;
	HCSearch::Global::settings->paths->INPUT_SPLITS_TRAIN_FILE_BASE = po.splitsTrainName;
	HCSearch::Global::settings->paths->INPUT_SPLITS_VALIDATION_FILE_BASE = po.splitsValidName;
	HCSearch::Global::settings->paths->INPUT_SPLITS_TEST_FILE_BASE = po.splitsTestName;
	HCSearch::Global::settings->paths->INPUT_NODES_FOLDER_NAME = po.nodesFolderName;
	HCSearch::Global::settings->paths->INPUT_EDGES_FOLDER_NAME = po.edgesFolderName;
	HCSearch::Global::settings->paths->INPUT_EDGE_FEATURES_FOLDER_NAME = po.edgeFeaturesFolderName;

	HCSearch::Global::settings->paths->OUTPUT_LOGS_FOLDER_NAME = po.logsFolderName;
	HCSearch::Global::settings->paths->OUTPUT_MODELS_FOLDER_NAME = po.modelsFolderName;
	HCSearch::Global::settings->paths->OUTPUT_RESULTS_FOLDER_NAME = po.resultsFolderName;
	HCSearch::Global::settings->paths->OUTPUT_TEMP_FOLDER_NAME = po.tempFolderName;

	HCSearch::Global::settings->USE_EDGE_WEIGHTS = po.useEdgeWeights;

	HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_MODEL_FILE_NAME = po.heuristicModelFileName;
	HCSearch::Global::settings->paths->OUTPUT_COST_H_MODEL_FILE_NAME = po.costModelFileName;
	HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_MODEL_FILE_NAME = po.costOracleHModelFileName;
	HCSearch::Global::settings->paths->OUTPUT_PRUNE_MODEL_FILE_NAME = po.pruneModelFileName;

	HCSearch::Setup::configure(po.inputDir, po.outputDir, po.baseDir);
	if (po.verboseMode)
		Logger::setLogLevel(DEBUG);

	// run all schedule option
	if (po.runAll || po.runAllLearn || po.runAllInfer)
		getRunAllSchedule(po);

	// print useful information
	printInfo(po);

	// demo or run full program
	if (po.demoMode)
		demo(po.timeBound);
	else
		run(po);

	// finalize
	HCSearch::Setup::finalize();
    return 0;
}

HCSearch::SearchSpace* setupSearchSpace(MyProgramOptions::ProgramOptions po)
{
	LOG() << "=== Search Space ===" << endl;

	// select some loss function
	LOG() << "Loss function: ";
	HCSearch::ILossFunction* lossFunc = NULL;
	switch (po.lossMode)
	{
	case MyProgramOptions::ProgramOptions::HAMMING:
		LOG() << "Hamming loss" << endl;
		lossFunc = new HCSearch::HammingLoss();
		break;
	case MyProgramOptions::ProgramOptions::PIXEL_HAMMING:
		LOG() << "Pixel Hamming loss" << endl;
		lossFunc = new HCSearch::PixelHammingLoss();
		break;
	default:
		LOG(ERROR) << "undefined loss mode.";
	}

	// select heuristic feature function
	LOG() << "Heuristic feature function: ";
	HCSearch::IFeatureFunction* heuristicFeatFunc = NULL;
	switch (po.heuristicFeaturesMode)
	{
	case MyProgramOptions::ProgramOptions::STANDARD:
		LOG() << "standard CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		heuristicFeatFunc = new HCSearch::StandardFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONTEXT:
		LOG() << "standard context CRF features" << endl;
		heuristicFeatFunc = new HCSearch::StandardContextFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_ALT:
		LOG() << "standard 2 CRF features" << endl;
		heuristicFeatFunc = new HCSearch::StandardAltFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF:
		LOG() << "standard 3 CRF features" << endl;
		heuristicFeatFunc = new HCSearch::StandardConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY:
		LOG() << "unary CRF features" << endl;
		heuristicFeatFunc = new HCSearch::UnaryFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY_CONF:
		LOG() << "unary confidences CRF features" << endl;
		heuristicFeatFunc = new HCSearch::UnaryConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_PAIR_COUNTS:
		LOG() << "pairwise bigram CRF features" << endl;
		heuristicFeatFunc = new HCSearch::StandardPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF_PAIR_COUNTS:
		LOG() << "pairwise bigram confidences CRF features" << endl;
		heuristicFeatFunc = new HCSearch::StandardConfPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::DENSE_CRF:
		LOG() << "dense CRF features" << endl;
		heuristicFeatFunc = new HCSearch::DenseCRFFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE:
		LOG() << "standard simple CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		heuristicFeatFunc = new HCSearch::StandardSimpleFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_CONTEXT:
		LOG() << "standard simple context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		heuristicFeatFunc = new HCSearch::StandardSimpleContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_ACTION_CONTEXT:
		LOG() << "standard simple action context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		heuristicFeatFunc = new HCSearch::StandardSimpleManualContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	default:
		LOG(ERROR) << "undefined feature mode.";
	}

	// select cost feature function
	LOG() << "Cost feature function: ";
	HCSearch::IFeatureFunction* costFeatFunc = NULL;
	switch (po.heuristicFeaturesMode)
	{
	case MyProgramOptions::ProgramOptions::STANDARD:
		LOG() << "standard CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		costFeatFunc = new HCSearch::StandardFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONTEXT:
		LOG() << "standard context CRF features" << endl;
		costFeatFunc = new HCSearch::StandardContextFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_ALT:
		LOG() << "standard 2 CRF features" << endl;
		costFeatFunc = new HCSearch::StandardAltFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF:
		LOG() << "standard 3 CRF features" << endl;
		costFeatFunc = new HCSearch::StandardConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY:
		LOG() << "unary CRF features" << endl;
		costFeatFunc = new HCSearch::UnaryFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY_CONF:
		LOG() << "unary confidences CRF features" << endl;
		costFeatFunc = new HCSearch::UnaryConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_PAIR_COUNTS:
		LOG() << "pairwise bigram CRF features" << endl;
		costFeatFunc = new HCSearch::StandardPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF_PAIR_COUNTS:
		LOG() << "pairwise bigram confidences CRF features" << endl;
		costFeatFunc = new HCSearch::StandardConfPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::DENSE_CRF:
		LOG() << "dense CRF features" << endl;
		costFeatFunc = new HCSearch::DenseCRFFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE:
		LOG() << "standard simple CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		costFeatFunc = new HCSearch::StandardSimpleFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_CONTEXT:
		LOG() << "standard simple context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		costFeatFunc = new HCSearch::StandardSimpleContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_ACTION_CONTEXT:
		LOG() << "standard simple action context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		costFeatFunc = new HCSearch::StandardSimpleManualContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	default:
		LOG(ERROR) << "undefined feature mode.";
	}

	// select prune feature function
	LOG() << "Prune feature function: ";
	HCSearch::IFeatureFunction* pruneFeatFunc = NULL;
	switch (po.pruneFeaturesMode)
	{
	case MyProgramOptions::ProgramOptions::STANDARD:
		LOG() << "standard CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		pruneFeatFunc = new HCSearch::StandardFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONTEXT:
		LOG() << "standard context CRF features" << endl;
		pruneFeatFunc = new HCSearch::StandardContextFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_ALT:
		LOG() << "standard 2 CRF features" << endl;
		pruneFeatFunc = new HCSearch::StandardAltFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF:
		LOG() << "standard 3 CRF features" << endl;
		pruneFeatFunc = new HCSearch::StandardConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY:
		LOG() << "unary CRF features" << endl;
		pruneFeatFunc = new HCSearch::UnaryFeatures();
		break;
	case MyProgramOptions::ProgramOptions::UNARY_CONF:
		LOG() << "unary confidences CRF features" << endl;
		pruneFeatFunc = new HCSearch::UnaryConfFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_PAIR_COUNTS:
		LOG() << "pairwise bigram CRF features" << endl;
		pruneFeatFunc = new HCSearch::StandardPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_CONF_PAIR_COUNTS:
		LOG() << "pairwise bigram confidences CRF features" << endl;
		pruneFeatFunc = new HCSearch::StandardConfPairwiseCountsFeatures();
		break;
	case MyProgramOptions::ProgramOptions::DENSE_CRF:
		LOG() << "dense CRF features" << endl;
		pruneFeatFunc = new HCSearch::DenseCRFFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_PRUNE:
		LOG() << "standard prune features" << endl;
		pruneFeatFunc = new HCSearch::StandardPruneFeatures();
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE:
		LOG() << "standard simple CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		pruneFeatFunc = new HCSearch::StandardSimpleFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_CONTEXT:
		LOG() << "standard simple context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		pruneFeatFunc = new HCSearch::StandardSimpleContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	case MyProgramOptions::ProgramOptions::STANDARD_SIMPLE_ACTION_CONTEXT:
		LOG() << "standard simple action context CRF features" << endl;
		LOG() << "\tlambda1=" << po.lambda1 << endl;
		LOG() << "\tlambda2=" << po.lambda2 << endl;
		LOG() << "\tlambda3=" << po.lambda3 << endl;
		pruneFeatFunc = new HCSearch::StandardSimpleManualContextFeatures(po.lambda1, po.lambda2, po.lambda3);
		break;
	default:
		LOG(ERROR) << "undefined feature mode.";
	}

	// select some successor function
	LOG() << "Successor function: ";
	HCSearch::ISuccessorFunction* successor = NULL;
	bool cutEdgesIndependently = po.stochasticCutMode == MyProgramOptions::ProgramOptions::EDGES;
	switch (po.successorsMode)
	{
	case MyProgramOptions::ProgramOptions::FLIPBIT:
		LOG() << "flipbit" << endl;
		successor = new HCSearch::FlipbitSuccessor();
		break;
	case MyProgramOptions::ProgramOptions::FLIPBIT_NEIGHBORS:
		LOG() << "flipbit neighbors" << endl;
		successor = new HCSearch::FlipbitNeighborSuccessor();
		break;
	case MyProgramOptions::ProgramOptions::FLIPBIT_CONFIDENCES_NEIGHBORS:
		LOG() << "flipbit confidences neighbors" << endl;
		successor = new HCSearch::FlipbitConfidencesNeighborSuccessor();
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC:
		LOG() << "stochastic" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tMax threshold for cutting: " << po.maxCuttingThreshold << endl;
		LOG() << "\tMin threshold for cutting: " << po.minCuttingThreshold << endl;
		successor = new HCSearch::StochasticSuccessor(cutEdgesIndependently, po.cutParam, po.maxCuttingThreshold, po.minCuttingThreshold);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_NEIGHBORS:
		LOG() << "stochastic neighbors" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tMax threshold for cutting: " << po.maxCuttingThreshold << endl;
		LOG() << "\tMin threshold for cutting: " << po.minCuttingThreshold << endl;
		successor = new HCSearch::StochasticNeighborSuccessor(cutEdgesIndependently, po.cutParam, po.maxCuttingThreshold, po.minCuttingThreshold);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_CONFIDENCES_NEIGHBORS:
		LOG() << "stochastic confidences neighbors" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tMax threshold for cutting: " << po.maxCuttingThreshold << endl;
		LOG() << "\tMin threshold for cutting: " << po.minCuttingThreshold << endl;
		successor = new HCSearch::StochasticConfidencesNeighborSuccessor(cutEdgesIndependently, po.cutParam, po.maxCuttingThreshold, po.minCuttingThreshold);
		break;
	case MyProgramOptions::ProgramOptions::CUT_SCHEDULE:
		LOG() << "cut schedule" << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		successor = new HCSearch::CutScheduleSuccessor(po.cutParam);
		break;
	case MyProgramOptions::ProgramOptions::CUT_SCHEDULE_NEIGHBORS:
		LOG() << "cut schedule neighbors" << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		successor = new HCSearch::CutScheduleNeighborSuccessor(po.cutParam);
		break;
	case MyProgramOptions::ProgramOptions::CUT_SCHEDULE_CONFIDENCES_NEIGHBORS:
		LOG() << "cut schedule confidences neighbors" << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		successor = new HCSearch::CutScheduleConfidencesNeighborSuccessor(po.cutParam);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_SCHEDULE:
		LOG() << "stochastic schedule" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tNode clamping: " << po.nodeClamp << ", Edge clamping: " << po.edgeClamp << endl;
		LOG() << "\t\tNode clamp threshold: " << po.nodeClampThreshold << endl;
		LOG() << "\t\tEdge clamp positive threshold: " << po.edgeClampPositiveThreshold << endl;
		LOG() << "\t\tEdge clamp negative threshold: " << po.edgeClampNegativeThreshold << endl;
		successor = new HCSearch::StochasticScheduleSuccessor(cutEdgesIndependently, po.cutParam, 
			po.nodeClamp, po.edgeClamp, po.nodeClampThreshold, po.edgeClampPositiveThreshold, po.edgeClampNegativeThreshold);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_SCHEDULE_NEIGHBORS:
		LOG() << "stochastic schedule neighbors" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tNode clamping: " << po.nodeClamp << ", Edge clamping: " << po.edgeClamp << endl;
		LOG() << "\t\tNode clamp threshold: " << po.nodeClampThreshold << endl;
		LOG() << "\t\tEdge clamp positive threshold: " << po.edgeClampPositiveThreshold << endl;
		LOG() << "\t\tEdge clamp negative threshold: " << po.edgeClampNegativeThreshold << endl;
		successor = new HCSearch::StochasticScheduleNeighborSuccessor(cutEdgesIndependently, po.cutParam, 
			po.nodeClamp, po.edgeClamp, po.nodeClampThreshold, po.edgeClampPositiveThreshold, po.edgeClampNegativeThreshold);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_SCHEDULE_CONFIDENCES_NEIGHBORS:
		LOG() << "stochastic schedule confidences neighbors" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tNode clamping: " << po.nodeClamp << ", Edge clamping: " << po.edgeClamp << endl;
		LOG() << "\t\tNode clamp threshold: " << po.nodeClampThreshold << endl;
		LOG() << "\t\tEdge clamp positive threshold: " << po.edgeClampPositiveThreshold << endl;
		LOG() << "\t\tEdge clamp negative threshold: " << po.edgeClampNegativeThreshold << endl;
		successor = new HCSearch::StochasticScheduleConfidencesNeighborSuccessor(cutEdgesIndependently, po.cutParam, 
			po.nodeClamp, po.edgeClamp, po.nodeClampThreshold, po.edgeClampPositiveThreshold, po.edgeClampNegativeThreshold);
		break;
	case MyProgramOptions::ProgramOptions::STOCHASTIC_CONSTRAINED:
		LOG() << "stochastic constrained" << endl;
		LOG() << "\tCut edges independently: " << cutEdgesIndependently << endl;
		LOG() << "\tTemperature parameter: " << po.cutParam << endl;
		LOG() << "\tNode clamping: " << po.nodeClamp << ", Edge clamping: " << po.edgeClamp << endl;
		LOG() << "\t\tNode clamp threshold: " << po.nodeClampThreshold << endl;
		LOG() << "\t\tEdge clamp positive threshold: " << po.edgeClampPositiveThreshold << endl;
		LOG() << "\t\tEdge clamp negative threshold: " << po.edgeClampNegativeThreshold << endl;
		successor = new HCSearch::StochasticConstrainedSuccessor(cutEdgesIndependently, po.cutParam, 
			po.nodeClamp, po.edgeClamp, po.nodeClampThreshold, po.edgeClampPositiveThreshold, po.edgeClampNegativeThreshold);
		break;
	default:
		LOG(ERROR) << "undefined successor mode.";
	}

	// use IID logistic regression as initial state prediction function
	LOG() << "Initial state prediction function: ";
	LOG() << "IID logistic regression" << endl;
	HCSearch::IInitialPredictionFunction* initPredFunc = new HCSearch::MutexLogRegInit(); //new HCSearch::LogRegInit();

	// select some pruning function
	LOG() << "Pruning function: ";
	HCSearch::IPruneFunction* pruneFunc = NULL;
	switch (po.pruneMode)
	{
	case MyProgramOptions::ProgramOptions::NO_PRUNE:
		LOG() << "no prune" << endl;
		pruneFunc = new HCSearch::NoPrune();
		break;
	case MyProgramOptions::ProgramOptions::RANKER_PRUNE:
		LOG() << "ranker prune" << endl;
		pruneFunc = new HCSearch::RankerPrune(po.pruneRatio, pruneFeatFunc);
		break;
	case MyProgramOptions::ProgramOptions::ORACLE_PRUNE:
		LOG() << "oracle prune" << endl;
		pruneFunc = new HCSearch::OraclePrune(lossFunc, po.badPruneRatio);
		break;
	case MyProgramOptions::ProgramOptions::SIMULATED_RANKER_PRUNE:
		LOG() << "simulated ranker prune" << endl;
		pruneFunc = new HCSearch::SimulatedRankerPrune(po.pruneRatio, pruneFeatFunc);
		break;
	default:
		LOG(ERROR) << "undefined prune mode.";
	}

	LOG() << endl;

	// construct search space from these functions that we specified
	return new HCSearch::SearchSpace(heuristicFeatFunc, costFeatFunc, initPredFunc, successor, pruneFunc, lossFunc);
}

HCSearch::ISearchProcedure* setupSearchProcedure(MyProgramOptions::ProgramOptions po)
{
	LOG() << "=== Search Procedure ===" << endl;

	HCSearch::ISearchProcedure* searchProcedure = NULL;
	switch (po.searchProcedureMode)
	{
	case MyProgramOptions::ProgramOptions::GREEDY:
		LOG() << "Using greedy search." << endl;
		searchProcedure = new HCSearch::GreedySearchProcedure();
		break;
	case MyProgramOptions::ProgramOptions::BREADTH_BEAM:
		LOG() << "Using breadth-first beam search." << endl;
		LOG() << "Beam size=" << po.beamSize << endl;
		searchProcedure = new HCSearch::BreadthFirstBeamSearchProcedure(po.beamSize);
		break;
	case MyProgramOptions::ProgramOptions::BEST_BEAM:
		LOG() << "Using best-first beam search." << endl;
		LOG() << "Beam size=" << po.beamSize << endl;
		searchProcedure = new HCSearch::BestFirstBeamSearchProcedure(po.beamSize);
		break;
	default:
		LOG(ERROR) << "undefined search procedure mode.";
	}

	LOG() << endl;

	return searchProcedure;
}

void run(MyProgramOptions::ProgramOptions po)
{
	clock_t tic = clock();

	// time bound
	int timeBound = po.timeBound;

	// paths
	string heuristicModelPath = HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_MODEL_FILE;
	string costModelPath = HCSearch::Global::settings->paths->OUTPUT_COST_H_MODEL_FILE;
	string costOracleHModelPath = HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_MODEL_FILE;
	string pruneModelPath = HCSearch::Global::settings->paths->OUTPUT_PRUNE_MODEL_FILE;
	string mutexPath = HCSearch::Global::settings->paths->OUTPUT_MUTEX_FILE;

	// params
	HCSearch::RankerType rankerType = po.rankLearnerType;

	// datasets
	vector<string> trainFiles;
	vector<string> validationFiles;
	vector<string> testFiles;

	// load dataset
	HCSearch::Dataset::loadDataset(trainFiles, validationFiles, testFiles);

	// load search space functions and search space
	HCSearch::SearchSpace* searchSpace = setupSearchSpace(po);

	// load search procedure
	HCSearch::ISearchProcedure* searchProcedure = setupSearchProcedure(po);

	// run the appropriate mode
	for (vector< HCSearch::SearchType >::iterator it = po.schedule.begin();
		it != po.schedule.end(); ++it)
	{
		HCSearch::Global::settings->stats->resetSuccessorCount();

		HCSearch::SearchType mode = *it;

		if (mode != HCSearch::DISCOVER_PAIRWISE && po.pruneFeaturesMode == MyProgramOptions::ProgramOptions::STANDARD_PRUNE
			&& po.pruneMode == MyProgramOptions::ProgramOptions::RANKER_PRUNE)
		{
			HCSearch::IPruneFunction* pruneFunc = searchSpace->getPruneFunction();
			HCSearch::RankerPrune* pruneCast = dynamic_cast<HCSearch::RankerPrune*>(pruneFunc);
			HCSearch::IFeatureFunction* featFunc = pruneCast->getFeatureFunction();
			HCSearch::StandardPruneFeatures* featCast = dynamic_cast<HCSearch::StandardPruneFeatures*>(featFunc);
			HCSearch::IInitialPredictionFunction* initPredFunc = searchSpace->getInitialPredictionFunction();
			HCSearch::MutexLogRegInit* initPredFuncCast = dynamic_cast<HCSearch::MutexLogRegInit*>(initPredFunc);

			if (MyFileSystem::FileSystem::checkFileExists(mutexPath))
			{
				map<string, int> mutex = HCSearch::Model::loadPairwiseConstraints(mutexPath);
				featCast->setMutex(mutex);
				initPredFuncCast->setMutex(mutex);
			}
		}
		else if (mode != HCSearch::DISCOVER_PAIRWISE && po.pruneFeaturesMode == MyProgramOptions::ProgramOptions::STANDARD_PRUNE
			&& po.pruneMode == MyProgramOptions::ProgramOptions::SIMULATED_RANKER_PRUNE)
		{
			HCSearch::IPruneFunction* pruneFunc = searchSpace->getPruneFunction();
			HCSearch::SimulatedRankerPrune* pruneCast = dynamic_cast<HCSearch::SimulatedRankerPrune*>(pruneFunc);
			HCSearch::IFeatureFunction* featFunc = pruneCast->getFeatureFunction();
			HCSearch::StandardPruneFeatures* featCast = dynamic_cast<HCSearch::StandardPruneFeatures*>(featFunc);
			HCSearch::IInitialPredictionFunction* initPredFunc = searchSpace->getInitialPredictionFunction();
			HCSearch::MutexLogRegInit* initPredFuncCast = dynamic_cast<HCSearch::MutexLogRegInit*>(initPredFunc);

			if (MyFileSystem::FileSystem::checkFileExists(mutexPath))
			{
				map<string, int> mutex = HCSearch::Model::loadPairwiseConstraints(mutexPath);
				featCast->setMutex(mutex);
				initPredFuncCast->setMutex(mutex);
			}
		}

		if (po.pruneMode == MyProgramOptions::ProgramOptions::RANKER_PRUNE)
		{
			HCSearch::IPruneFunction* pruneFunc = searchSpace->getPruneFunction();
			HCSearch::RankerPrune* pruneCast = dynamic_cast<HCSearch::RankerPrune*>(pruneFunc);
			if (po.rankLearnerType == HCSearch::VW_RANK)
			{
				HCSearch::IRankModel* pruneModel;
				if (MyFileSystem::FileSystem::checkFileExists(pruneModelPath))
				{
					pruneModel = HCSearch::Model::loadModel(pruneModelPath, HCSearch::VW_RANK);
					LOG() << endl << "Loaded pruning model." << endl << endl;
				}
				else
				{
					pruneModel = new HCSearch::VWRankModel();
					LOG() << endl << "Creating empty pruning model." << endl << endl;
				}
				pruneCast->setRanker(pruneModel);
			}
			else
			{
				LOG(ERROR) << "only VW supported for prune ranker model" << endl;
				HCSearch::abort();
			}
		}

		switch (mode)
		{
		case HCSearch::LEARN_H:
		{
			LOG() << "=== Learning H ===" << endl;

			// learn heuristic, save heuristic model
			HCSearch::IRankModel* heuristicModel = HCSearch::Learning::learnH(trainFiles, validationFiles,
				timeBound, searchSpace, searchProcedure, po.rankLearnerType, po.numTrainIterations);
			
			if (HCSearch::Global::settings->RANK == 0)
			{
				HCSearch::Model::saveModel(heuristicModel, heuristicModelPath, rankerType);
				if (po.saveFeaturesFiles && HCSearch::RankerTypeSaveable[po.rankLearnerType])
					MyFileSystem::FileSystem::copyFile(HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_FEATURES_FILE, 
						HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_HEURISTIC_FEATURES_FILE);
			}
			
			MyFileSystem::FileSystem::deleteFile(HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_FEATURES_FILE);
			delete heuristicModel;

#ifdef USE_MPI
		EasyMPI::EasyMPI::synchronize("LEARNHSTART", "LEARNHEND");
#endif

			if (HCSearch::Global::settings->RANK == 0)
			{
				writeProgressToFile(HCSearch::LEARN_H);
			}

			break;
		}
		case HCSearch::LEARN_C:
		{
			LOG() << "=== Learning C with Learned H ===" << endl;

			// load heuristic, learn cost, save cost model
			HCSearch::IRankModel* heuristicModel = HCSearch::Model::loadModel(heuristicModelPath, rankerType);
			HCSearch::IRankModel* costModel = HCSearch::Learning::learnC(trainFiles, validationFiles,
				heuristicModel, timeBound, searchSpace, searchProcedure, po.rankLearnerType, po.numTrainIterations);
			
			if (HCSearch::Global::settings->RANK == 0)
			{
				HCSearch::Model::saveModel(costModel, costModelPath, rankerType);
				if (po.saveFeaturesFiles && HCSearch::RankerTypeSaveable[po.rankLearnerType])
					MyFileSystem::FileSystem::copyFile(HCSearch::Global::settings->paths->OUTPUT_COST_H_FEATURES_FILE, 
						HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_COST_H_FEATURES_FILE);
			}
			
			MyFileSystem::FileSystem::deleteFile(HCSearch::Global::settings->paths->OUTPUT_COST_H_FEATURES_FILE);
			delete heuristicModel;
			delete costModel;

#ifdef USE_MPI
		EasyMPI::EasyMPI::synchronize("LEARNCSTART", "LEARNCEND");
#endif

			if (HCSearch::Global::settings->RANK == 0)
			{
				writeProgressToFile(HCSearch::LEARN_C);
			}

			break;
		}
		case HCSearch::LEARN_C_ORACLE_H:
		{
			LOG() << "=== Learning C with Oracle H ===" << endl;

			// learn cost, save cost model
			HCSearch::IRankModel* costOracleHModel = HCSearch::Learning::learnCWithOracleH(trainFiles, validationFiles,
				timeBound, searchSpace, searchProcedure, po.rankLearnerType, po.numTrainIterations);
			
			if (HCSearch::Global::settings->RANK == 0)
			{
				HCSearch::Model::saveModel(costOracleHModel, costOracleHModelPath, rankerType);
				if (po.saveFeaturesFiles && HCSearch::RankerTypeSaveable[po.rankLearnerType])
					MyFileSystem::FileSystem::copyFile(HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_FEATURES_FILE, 
						HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_COST_ORACLE_H_FEATURES_FILE);
			}
			
			MyFileSystem::FileSystem::deleteFile(HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_FEATURES_FILE);
			delete costOracleHModel;

#ifdef USE_MPI
		EasyMPI::EasyMPI::synchronize("LEARNCOHSTART", "LEARNCOHEND");
#endif

			if (HCSearch::Global::settings->RANK == 0)
			{
				writeProgressToFile(HCSearch::LEARN_C_ORACLE_H);
			}

			break;
		}
		case HCSearch::LEARN_PRUNE:
		{
			LOG() << "=== Learning P ===" << endl;

			// learn cost, save cost model
			if (po.pruneMode == MyProgramOptions::ProgramOptions::RANKER_PRUNE)
			{
				HCSearch::IRankModel* pruneModel = HCSearch::Learning::learnP(trainFiles, validationFiles,
					timeBound, searchSpace, searchProcedure, HCSearch::VW_RANK, po.numTrainIterations);
				
				// set the prune function
				HCSearch::IPruneFunction* pruneFunc = searchSpace->getPruneFunction();
				HCSearch::RankerPrune* pruneCast = dynamic_cast<HCSearch::RankerPrune*>(pruneFunc);
				pruneCast->setRanker(pruneModel);

				if (HCSearch::Global::settings->RANK == 0)
				{
					pruneModel->save(pruneModelPath);
					if (po.saveFeaturesFiles)
						MyFileSystem::FileSystem::copyFile(HCSearch::Global::settings->paths->OUTPUT_PRUNE_FEATURES_FILE, 
							HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_PRUNE_FEATURES_FILE);
				}
				
				MyFileSystem::FileSystem::deleteFile(HCSearch::Global::settings->paths->OUTPUT_PRUNE_FEATURES_FILE);
				delete pruneModel;
			}
			else
			{
				LOG(ERROR) << "unsupported pruning" << endl;
			}

#ifdef USE_MPI
		EasyMPI::EasyMPI::synchronize("LEARNPSTART", "LEARNPEND");
#endif

			//if (HCSearch::Global::settings->RANK == 0)
			//{
			//	writeProgressToFile(HCSearch::LEARN_PRUNE);
			//}

			break;
		}
		case HCSearch::DISCOVER_PAIRWISE:
		{
			LOG() << "=== Discovering Mutex ===" << endl;

			// discover mutex constraints
			map<string, int> pairwiseConstraints = HCSearch::Learning::discoverPairwiseClassConstraints(trainFiles);
			
			if (HCSearch::Global::settings->RANK == 0)
			{
				HCSearch::Model::savePairwiseConstraints(pairwiseConstraints, mutexPath);
			}

#ifdef USE_MPI
		EasyMPI::EasyMPI::synchronize("DISCOVERMUTEXSTART", "DISCOVERMUTEXEND");
#endif

			break;
		}
		case HCSearch::LL:
		{
			// set up commands
			vector<string> commands;
			vector<string> messages;
			for (int imageID = 0; imageID < static_cast<int>(testFiles.size()); imageID++)
			{
				for (int iter = 0; iter < po.numTestIterations; iter++)
				{
					int iteration = iter;
					if (po.numTestIterations == 1)
						iteration = po.uniqueIterId;

					stringstream ssMessage;
					ssMessage << imageID << ":" << iteration;

					commands.push_back("INFERLL");
					messages.push_back(ssMessage.str());
				}
			}

			// schedule and perform tasks
			if (HCSearch::Global::settings->RANK == 0 && HCSearch::Global::settings->NUM_PROCESSES > 1)
			{
				EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
				writeProgressToFile(HCSearch::LL);
			}
			else
			{
				runSlave(commands, messages, trainFiles, validationFiles, testFiles, searchSpace, searchProcedure, po);
				if (HCSearch::Global::settings->NUM_PROCESSES == 1)
				{
					writeProgressToFile(HCSearch::LL);
				}
			}

			break;
		}
		case HCSearch::HL:
		{
			// set up commands
			vector<string> commands;
			vector<string> messages;
			for (int imageID = 0; imageID < static_cast<int>(testFiles.size()); imageID++)
			{
				for (int iter = 0; iter < po.numTestIterations; iter++)
				{
					int iteration = iter;
					if (po.numTestIterations == 1)
						iteration = po.uniqueIterId;

					stringstream ssMessage;
					ssMessage << imageID << ":" << iteration;

					commands.push_back("INFERHL");
					messages.push_back(ssMessage.str());
				}
			}

			// schedule and perform tasks
			if (HCSearch::Global::settings->RANK == 0 && HCSearch::Global::settings->NUM_PROCESSES > 1)
			{
				EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
				writeProgressToFile(HCSearch::HL);
			}
			else
			{
				runSlave(commands, messages, trainFiles, validationFiles, testFiles, searchSpace, searchProcedure, po);
				if (HCSearch::Global::settings->NUM_PROCESSES == 1)
				{
					writeProgressToFile(HCSearch::HL);
				}
			}

			break;
		}
		case HCSearch::LC:
		{
			// set up commands
			vector<string> commands;
			vector<string> messages;
			for (int imageID = 0; imageID < static_cast<int>(testFiles.size()); imageID++)
			{
				for (int iter = 0; iter < po.numTestIterations; iter++)
				{
					int iteration = iter;
					if (po.numTestIterations == 1)
						iteration = po.uniqueIterId;

					stringstream ssMessage;
					ssMessage << imageID << ":" << iteration;

					commands.push_back("INFERLC");
					messages.push_back(ssMessage.str());
				}
			}

			// schedule and perform tasks
			if (HCSearch::Global::settings->RANK == 0 && HCSearch::Global::settings->NUM_PROCESSES > 1)
			{
				EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
				writeProgressToFile(HCSearch::LC);
			}
			else
			{
				runSlave(commands, messages, trainFiles, validationFiles, testFiles, searchSpace, searchProcedure, po);
				if (HCSearch::Global::settings->NUM_PROCESSES == 1)
				{
					writeProgressToFile(HCSearch::LC);
				}
			}

			break;
		}
		case HCSearch::HC:
		{
			// set up commands
			vector<string> commands;
			vector<string> messages;
			for (int imageID = 0; imageID < static_cast<int>(testFiles.size()); imageID++)
			{
				for (int iter = 0; iter < po.numTestIterations; iter++)
				{
					int iteration = iter;
					if (po.numTestIterations == 1)
						iteration = po.uniqueIterId;

					stringstream ssMessage;
					ssMessage << imageID << ":" << iteration;

					commands.push_back("INFERHC");
					messages.push_back(ssMessage.str());
				}
			}

			// schedule and perform tasks
			if (HCSearch::Global::settings->RANK == 0 && HCSearch::Global::settings->NUM_PROCESSES > 1)
			{
				EasyMPI::EasyMPI::masterScheduleTasks(commands, messages);
				writeProgressToFile(HCSearch::HC);
			}
			else
			{
				runSlave(commands, messages, trainFiles, validationFiles, testFiles, searchSpace, searchProcedure, po);
				if (HCSearch::Global::settings->NUM_PROCESSES == 1)
				{
					writeProgressToFile(HCSearch::HC);
				}
			}

			break;
		}
		default:
			LOG(ERROR) << "invalid mode!";
		}

		LOG() << "Average number of successor candidates=" << HCSearch::Global::settings->stats->getSuccessorAverage() << endl;
	}

	// clean up
	delete searchSpace;
	delete searchProcedure;

	clock_t toc = clock();
	LOG() << "total run time: " << (double)(toc - tic)/CLOCKS_PER_SEC << endl << endl;
}

void runSlave(vector<string> commands, vector<string> messages, 
			  vector<string>& trainFiles, vector<string>& validationFiles, 
			  vector<string>& testFiles, HCSearch::SearchSpace*& searchSpace, 
			  HCSearch::ISearchProcedure*& searchProcedure, MyProgramOptions::ProgramOptions& po)
{
	vector<string> commandSet = commands;
	vector<string> messageSet = messages;

	// loop to wait for tasks
	while (true)
	{
		// wait for a task
		std::string command;
		std::string message;

		// wait for task if more than one process
		// otherwise if only one process, then perform task on master process
		if (HCSearch::Global::settings->NUM_PROCESSES > 1)
		{
			EasyMPI::EasyMPI::slaveWaitForTasks(command, message);
		}
		else
		{
			if (commandSet.empty() || messageSet.empty())
				break;

			command = commandSet.back();
			message = messageSet.back();
			commandSet.pop_back();
			messageSet.pop_back();
		}

		LOG(INFO) << "Got command '" << command << "' and message '" << message << "'";

		// define branches here to perform task depending on command
		if (command.compare("INFERLL") == 0)
		{

			LOG() << "=== Inference LL ===" << endl;

			// Declare
			int i; // image ID
			int iter; // iteration ID
			getImageIDAndIter(message, i, iter);

			LOG() << endl << "LL Search: (iter " << iter << ") beginning search on " << testFiles[i] << " (example " << i << ")..." << endl;

			// setup meta
			HCSearch::ISearchProcedure::SearchMetadata meta;
			meta.saveAnytimePredictions = po.saveAnytimePredictions;
			meta.setType = HCSearch::TEST;
			meta.exampleName = testFiles[i];
			meta.iter = iter;

			HCSearch::ImgFeatures* XTestObj = NULL;
			HCSearch::ImgLabeling* YTestObj = NULL;
			HCSearch::Dataset::loadImage(testFiles[i], XTestObj, YTestObj);

			// inference
			HCSearch::ImgLabeling YPred = HCSearch::Inference::runLLSearch(XTestObj, YTestObj, 
				po.timeBound, searchSpace, searchProcedure, meta);
				
			// save the prediction
			stringstream ssPredictNodes;
			ssPredictNodes << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final" 
				<< "_nodes_" << HCSearch::SearchTypeStrings[HCSearch::LL] 
				<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
				<< "_time" << po.timeBound 
					<< "_fold" << meta.iter 
					<< "_" << meta.exampleName << ".txt";
			HCSearch::SavePrediction::saveLabels(YPred, ssPredictNodes.str());

			// save the prediction mask
			if (po.saveOutputMask)
			{
				stringstream ssPredictSegments;
				ssPredictSegments << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final"
					<< "_" << HCSearch::SearchTypeStrings[HCSearch::LL] 
					<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
					<< "_time" << po.timeBound 
						<< "_fold" << meta.iter 
						<< "_" << meta.exampleName << ".txt";
				HCSearch::SavePrediction::saveLabelMask(*XTestObj, YPred, ssPredictSegments.str());
			}

			HCSearch::Dataset::unloadImage(XTestObj, YTestObj);

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();

		}
		else if (command.compare("INFERHL") == 0)
		{

			LOG() << "=== Inference HL ===" << endl;

			// load heuristic, run HL search on test examples
			HCSearch::IRankModel* heuristicModel = HCSearch::Model::loadModel(HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_MODEL_FILE, po.rankLearnerType);

			// Declare
			int i; // image ID
			int iter; // iteration ID
			getImageIDAndIter(message, i, iter);

			LOG() << endl << "HL Search: (iter " << iter << ") beginning search on " << testFiles[i] << " (example " << i << ")..." << endl;

			// setup meta
			HCSearch::ISearchProcedure::SearchMetadata meta;
			meta.saveAnytimePredictions = po.saveAnytimePredictions;
			meta.setType = HCSearch::TEST;
			meta.exampleName = testFiles[i];
			meta.iter = iter;

			HCSearch::ImgFeatures* XTestObj = NULL;
			HCSearch::ImgLabeling* YTestObj = NULL;
			HCSearch::Dataset::loadImage(testFiles[i], XTestObj, YTestObj);

			// inference
			HCSearch::ImgLabeling YPred = HCSearch::Inference::runHLSearch(XTestObj, YTestObj, 
				po.timeBound, searchSpace, searchProcedure, heuristicModel, meta);
				
			// save the prediction
			stringstream ssPredictNodes;
			ssPredictNodes << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final" 
				<< "_nodes_" << HCSearch::SearchTypeStrings[HCSearch::HL] 
				<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
				<< "_time" << po.timeBound 
					<< "_fold" << meta.iter 
					<< "_" << meta.exampleName << ".txt";
			HCSearch::SavePrediction::saveLabels(YPred, ssPredictNodes.str());

			// save the prediction mask
			if (po.saveOutputMask)
			{
				stringstream ssPredictSegments;
				ssPredictSegments << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final"
					<< "_" << HCSearch::SearchTypeStrings[HCSearch::HL] 
					<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
					<< "_time" << po.timeBound 
						<< "_fold" << meta.iter 
						<< "_" << meta.exampleName << ".txt";
				HCSearch::SavePrediction::saveLabelMask(*XTestObj, YPred, ssPredictSegments.str());
			}

			HCSearch::Dataset::unloadImage(XTestObj, YTestObj);

			delete heuristicModel;

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();

		}
		else if (command.compare("INFERLC") == 0)
		{

			LOG() << "=== Inference LC ===" << endl;

			// load cost oracle H, run LC search on test examples
			HCSearch::IRankModel* costModel = HCSearch::Model::loadModel(HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_MODEL_FILE, po.rankLearnerType);

			// Declare
			int i; // image ID
			int iter; // iteration ID
			getImageIDAndIter(message, i, iter);

			LOG() << endl << "LC Search: (iter " << iter << ") beginning search on " << testFiles[i] << " (example " << i << ")..." << endl;

			// setup meta
			HCSearch::ISearchProcedure::SearchMetadata meta;
			meta.saveAnytimePredictions = po.saveAnytimePredictions;
			meta.setType = HCSearch::TEST;
			meta.exampleName = testFiles[i];
			meta.iter = iter;

			HCSearch::ImgFeatures* XTestObj = NULL;
			HCSearch::ImgLabeling* YTestObj = NULL;
			HCSearch::Dataset::loadImage(testFiles[i], XTestObj, YTestObj);

			// inference
			HCSearch::ImgLabeling YPred = HCSearch::Inference::runLCSearch(XTestObj, YTestObj, 
				po.timeBound, searchSpace, searchProcedure, costModel, meta);
				
			// save the prediction
			stringstream ssPredictNodes;
			ssPredictNodes << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final" 
				<< "_nodes_" << HCSearch::SearchTypeStrings[HCSearch::LC] 
				<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
				<< "_time" << po.timeBound 
					<< "_fold" << meta.iter 
					<< "_" << meta.exampleName << ".txt";
			HCSearch::SavePrediction::saveLabels(YPred, ssPredictNodes.str());

			// save the prediction mask
			if (po.saveOutputMask)
			{
				stringstream ssPredictSegments;
				ssPredictSegments << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final"
					<< "_" << HCSearch::SearchTypeStrings[HCSearch::LC] 
					<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
					<< "_time" << po.timeBound 
						<< "_fold" << meta.iter 
						<< "_" << meta.exampleName << ".txt";
				HCSearch::SavePrediction::saveLabelMask(*XTestObj, YPred, ssPredictSegments.str());
			}

			HCSearch::Dataset::unloadImage(XTestObj, YTestObj);

			delete costModel;

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();

		}
		else if (command.compare("INFERHC") == 0)
		{

			LOG() << "=== Inference HC ===" << endl;

			// load heuristic and cost, run HC search on test examples
			HCSearch::IRankModel* heuristicModel = HCSearch::Model::loadModel(HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_MODEL_FILE, po.rankLearnerType);
			HCSearch::IRankModel* costModel = HCSearch::Model::loadModel(HCSearch::Global::settings->paths->OUTPUT_COST_H_MODEL_FILE, po.rankLearnerType);

			// Declare
			int i; // image ID
			int iter; // iteration ID
			getImageIDAndIter(message, i, iter);

			LOG() << endl << "HC Search: (iter " << iter << ") beginning search on " << testFiles[i] << " (example " << i << ")..." << endl;

			// setup meta
			HCSearch::ISearchProcedure::SearchMetadata meta;
			meta.saveAnytimePredictions = po.saveAnytimePredictions;
			meta.setType = HCSearch::TEST;
			meta.exampleName = testFiles[i];
			meta.iter = iter;

			HCSearch::ImgFeatures* XTestObj = NULL;
			HCSearch::ImgLabeling* YTestObj = NULL;
			HCSearch::Dataset::loadImage(testFiles[i], XTestObj, YTestObj);

			// inference
			HCSearch::ImgLabeling YPred = HCSearch::Inference::runHCSearch(XTestObj, YTestObj, 
				po.timeBound, searchSpace, searchProcedure, heuristicModel, costModel, meta);

			// save the prediction
			stringstream ssPredictNodes;
			ssPredictNodes << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final" 
				<< "_nodes_" << HCSearch::SearchTypeStrings[HCSearch::HC] 
				<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
				<< "_time" << po.timeBound 
					<< "_fold" << meta.iter 
					<< "_" << meta.exampleName << ".txt";
			HCSearch::SavePrediction::saveLabels(YPred, ssPredictNodes.str());

			// save the prediction mask
			if (po.saveOutputMask)
			{
				stringstream ssPredictSegments;
				ssPredictSegments << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR << "final"
					<< "_" << HCSearch::SearchTypeStrings[HCSearch::HC] 
					<< "_" << HCSearch::DatasetTypeStrings[meta.setType] 
					<< "_time" << po.timeBound 
						<< "_fold" << meta.iter 
						<< "_" << meta.exampleName << ".txt";
				HCSearch::SavePrediction::saveLabelMask(*XTestObj, YPred, ssPredictSegments.str());
			}

			HCSearch::Dataset::unloadImage(XTestObj, YTestObj);

			delete heuristicModel;
			delete costModel;

			// declare finished
			EasyMPI::EasyMPI::slaveFinishedTask();

		}
		else if (command.compare(EasyMPI::EasyMPI::MASTER_FINISH_MESSAGE) == 0)
		{
			LOG(INFO) << "Got the master finish command on process " << HCSearch::Global::settings->RANK
				<< ". Exiting slave loop...";

			break;
		}
		else
		{
			LOG(WARNING) << "Invalid command: " << command;
		}
	}
}

void getImageIDAndIter(string message, int& imageID, int& iterID)
{
	string imageIDString;
	string iterIDString;
	stringstream ss(message);
	getline(ss, imageIDString, ':');
	getline(ss, iterIDString, ':');

	imageID = atoi(imageIDString.c_str());
	iterID = atoi(iterIDString.c_str());
}

void printInfo(MyProgramOptions::ProgramOptions po)
{
	LOG() << "=== Program Schedule ===" << endl;

	int cnt = 1;
	for (vector< HCSearch::SearchType >::iterator it = po.schedule.begin();
		it != po.schedule.end(); ++it)
	{
		HCSearch::SearchType mode = *it;
		LOG() << cnt << ". " << HCSearch::SearchTypeStrings[mode] << endl;
		cnt++;
	}

	LOG() << endl;

	if (!po.demoMode)
	{
		LOG() << "=== Program Options ===" << endl;
		LOG() << "Rank learner: " << HCSearch::RankerTypeStrings[po.rankLearnerType] << endl;
		LOG() << "Num training iterations: " << po.numTrainIterations << endl;
		LOG() << "Num testing iterations: " << po.numTestIterations << endl;
		if (po.numTestIterations == 1)
		{
			LOG() << "\tUnique iteration ID: " << po.uniqueIterId << endl;
		}
		LOG() << "Save anytime predictions: " << po.saveAnytimePredictions << endl;
		LOG() << "Save features files: " << po.saveFeaturesFiles << endl;
		LOG() << "Save output label masks: " << po.saveOutputMask << endl;
	}
	else
	{
		LOG() << "=== USING DEMO MODE ===" << endl;
	}

	LOG() << endl;

	if (po.verboseMode)
	{
		LOG() << "=== Paths ===" << endl;

		LOG() << "BASE_DIR: " << HCSearch::Global::settings->paths->BASE_PATH << endl;
		LOG() << "EXTERNAL_DIR: " << HCSearch::Global::settings->paths->EXTERNAL_DIR << endl;
		LOG() << "INPUT_DIR: " << HCSearch::Global::settings->paths->INPUT_DIR  << endl;
		LOG() << "OUTPUT_DIR: " << HCSearch::Global::settings->paths->OUTPUT_DIR  << endl;
		LOG() << "LIBLINEAR_DIR: " << HCSearch::Global::settings->paths->LIBLINEAR_DIR  << endl;
		LOG() << "LIBSVM_DIR: " << HCSearch::Global::settings->paths->LIBSVM_DIR  << endl;
		LOG() << "SVMRANK_DIR: " << HCSearch::Global::settings->paths->SVMRANK_DIR  << endl;
		LOG() << "VW_DIR: " << HCSearch::Global::settings->paths->VOWPALWABBIT_DIR  << endl;

		LOG() << endl;

		LOG() << "INPUT_NODES_DIR: " << HCSearch::Global::settings->paths->INPUT_NODES_DIR << endl;
		LOG() << "INPUT_EDGES_DIR: " << HCSearch::Global::settings->paths->INPUT_EDGES_DIR  << endl;
		LOG() << "INPUT_META_DIR: " << HCSearch::Global::settings->paths->INPUT_META_DIR  << endl;
		LOG() << "INPUT_SEGMENTS_DIR: " << HCSearch::Global::settings->paths->INPUT_SEGMENTS_DIR  << endl;
		LOG() << "INPUT_SPLITS_DIR: " << HCSearch::Global::settings->paths->INPUT_SPLITS_DIR  << endl;
		LOG() << "INPUT_SPLITS_FOLDER_NAME: " << HCSearch::Global::settings->paths->INPUT_SPLITS_FOLDER_NAME  << endl;

		LOG() << "INPUT_SPLITS_TRAIN_FILE: " << HCSearch::Global::settings->paths->INPUT_SPLITS_TRAIN_FILE << endl;
		LOG() << "INPUT_SPLITS_VALIDATION_FILE: " << HCSearch::Global::settings->paths->INPUT_SPLITS_VALIDATION_FILE  << endl;
		LOG() << "INPUT_SPLITS_TEST_FILE: " << HCSearch::Global::settings->paths->INPUT_SPLITS_TEST_FILE  << endl;

		LOG() << "INPUT_METADATA_FILE: " << HCSearch::Global::settings->paths->INPUT_METADATA_FILE  << endl;
		LOG() << "INPUT_CODEBOOK_FILE: " << HCSearch::Global::settings->paths->INPUT_CODEBOOK_FILE  << endl;
		LOG() << "INPUT_SPLITS_FOLDER_NAME: " << HCSearch::Global::settings->paths->INPUT_INITFUNC_TRAINING_FILE  << endl;

		LOG() << endl;

		LOG() << "OUTPUT_LOGS_DIR: " << HCSearch::Global::settings->paths->OUTPUT_LOGS_DIR << endl;
		LOG() << "OUTPUT_MODELS_DIR: " << HCSearch::Global::settings->paths->OUTPUT_MODELS_DIR  << endl;
		LOG() << "OUTPUT_RESULTS_DIR: " << HCSearch::Global::settings->paths->OUTPUT_RESULTS_DIR  << endl;
		LOG() << "OUTPUT_TEMP_DIR: " << HCSearch::Global::settings->paths->OUTPUT_TEMP_DIR  << endl;

		LOG() << "OUTPUT_HEURISTIC_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_FEATURES_FILE  << endl;
		LOG() << "OUTPUT_COST_H_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_COST_H_FEATURES_FILE  << endl;
		LOG() << "OUTPUT_COST_ORACLE_H_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_FEATURES_FILE << endl;

		LOG() << "OUTPUT_ARCHIVED_HEURISTIC_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_HEURISTIC_FEATURES_FILE  << endl;
		LOG() << "OUTPUT_ARCHIVED_COST_H_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_COST_H_FEATURES_FILE  << endl;
		LOG() << "OUTPUT_ARCHIVED_COST_ORACLE_H_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_ARCHIVED_COST_ORACLE_H_FEATURES_FILE  << endl;

		LOG() << "OUTPUT_HEURISTIC_MODEL_FILE: " << HCSearch::Global::settings->paths->OUTPUT_HEURISTIC_MODEL_FILE  << endl;
		LOG() << "OUTPUT_COST_H_MODEL_FILE: " << HCSearch::Global::settings->paths->OUTPUT_COST_H_MODEL_FILE  << endl;
		LOG() << "OUTPUT_COST_ORACLE_H_MODEL_FILE: " << HCSearch::Global::settings->paths->OUTPUT_COST_ORACLE_H_MODEL_FILE  << endl;

		LOG() << "OUTPUT_LOG_FILE: " << HCSearch::Global::settings->paths->OUTPUT_LOG_FILE  << endl;

		LOG() << "OUTPUT_INITFUNC_MODEL_FILE: " << HCSearch::Global::settings->paths->OUTPUT_INITFUNC_MODEL_FILE  << endl;
		LOG() << "OUTPUT_INITFUNC_FEATURES_FILE: " << HCSearch::Global::settings->paths->OUTPUT_INITFUNC_FEATURES_FILE << endl;
		LOG() << "OUTPUT_INITFUNC_PREDICT_FILE: " << HCSearch::Global::settings->paths->OUTPUT_INITFUNC_PREDICT_FILE  << endl;

		LOG() << endl;
	}
}

void getRunAllSchedule(MyProgramOptions::ProgramOptions& po)
{
	//bool learnP = false;
	bool learnH = false;
	bool learnC = false;
	bool learnCOH = false;
	bool inferLL = false;
	bool inferHL = false;
	bool inferHC = false;
	bool inferLC = false;

	// read in file
	if (MyFileSystem::FileSystem::checkFileExists(HCSearch::Global::settings->paths->OUTPUT_PROGRESS_FILE))
	{
		string line;
		ifstream fh(HCSearch::Global::settings->paths->OUTPUT_PROGRESS_FILE.c_str());
		if (fh.is_open())
		{
			while (fh.good())
			{
				getline(fh, line);
				//if (line.compare("LEARNP") == 0)
				//{
				//	learnP = true;
				//}
				if (line.compare("LEARNH") == 0)
				{
					learnH = true;
				}
				else if (line.compare("LEARNC") == 0)
				{
					learnC = true;
				}
				else if (line.compare("LEARNCOH") == 0)
				{
					learnCOH = true;
				}
				else if (line.compare("INFERLL") == 0)
				{
					inferLL = true;
				}
				else if (line.compare("INFERHL") == 0)
				{
					inferHL = true;
				}
				else if (line.compare("INFERHC") == 0)
				{
					inferHC = true;
				}
				else if (line.compare("INFERLC") == 0)
				{
					inferLC = true;
				}
			}
			fh.close();
		}
		else
		{
			LOG(ERROR) << "cannot open progress file!";
			abort();
		}
	}

	// create schedule
	//if (!learnP)
	//	schedule.push_back(HCSearch::LEARN_PRUNE);
	if (!learnH && (po.runAll || po.runAllLearn))
		po.schedule.push_back(HCSearch::LEARN_H);
	if (!learnC && (po.runAll || po.runAllLearn))
		po.schedule.push_back(HCSearch::LEARN_C);
	if (!learnCOH && (po.runAll || po.runAllLearn))
		po.schedule.push_back(HCSearch::LEARN_C_ORACLE_H);
	if (!inferLL && (po.runAll || po.runAllInfer))
		po.schedule.push_back(HCSearch::LL);
	if (!inferHL && (po.runAll || po.runAllInfer))
		po.schedule.push_back(HCSearch::HL);
	if (!inferHC && (po.runAll || po.runAllInfer))
		po.schedule.push_back(HCSearch::HC);
	if (!inferLC && (po.runAll || po.runAllInfer))
		po.schedule.push_back(HCSearch::LC);
}

void writeProgressToFile(HCSearch::SearchType completedSearchType)
{
	ofstream fh(HCSearch::Global::settings->paths->OUTPUT_PROGRESS_FILE.c_str(), std::ios_base::app);
	if (fh.is_open())
	{
		switch (completedSearchType)
		{
		//case HCSearch::LEARN_P:
		//	fh << "LEARNP" << endl;
		//	break;
		case HCSearch::LEARN_H:
			fh << "LEARNH" << endl;
			break;
		case HCSearch::LEARN_C:
			fh << "LEARNC" << endl;
			break;
		case HCSearch::LEARN_C_ORACLE_H:
			fh << "LEARNCOH" << endl;
			break;
		case HCSearch::LL:
			fh << "INFERLL" << endl;
			break;
		case HCSearch::HL:
			fh << "INFERHL" << endl;
			break;
		case HCSearch::HC:
			fh << "INFERHC" << endl;
			break;
		case HCSearch::LC:
			fh << "INFERLC" << endl;
			break;
		default:
			LOG(ERROR) << "invalid search type for writing to progress file!";
			abort();
		}
	}
	else
	{
		LOG(ERROR) << "cannot open progress file!";
		abort();
	}
}