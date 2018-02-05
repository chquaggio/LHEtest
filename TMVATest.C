#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TChainElement.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TString.h"
#include "TMath.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TNtuple.h"

#include "TMVA/MsgLogger.h"
#include "TMVA/Config.h"
#include "TMVA/Factory.h"
#include "TMVA/Reader.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Tools.h"
#include "TMVA/TMVAGui.h"

using namespace std;

int TMVATest()
{
    
    TMVA::Tools::Instance();
    
    //output file
    TString outfileName = "TMVAtry.root";
    TFile* outputFile = TFile::Open( outfileName, "RECREATE" );  
    
    //SigTree
    TFile* Sig_TFile = TFile::Open("./output_quaggio_ewk_20M.root");
    TTree* Sig_Tree = (TTree*)Sig_TFile->Get("tree");
    
    //BkgTree
    TFile* Bkg_TFile = TFile::Open("./output_quaggio_qcd_1M.root");
    TTree* Bkg_Tree = (TTree*)Bkg_TFile->Get("tree");
    
    TMVA::Factory* TMVAtest = new TMVA::Factory(
        "BlaBlaPROVAA", 
        outputFile,
        "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification"
        );

    TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataset");
    
    //Add Sig and Bkg trees
    dataloader->AddSignalTree(Sig_Tree, 1.);
    dataloader->AddBackgroundTree(Bkg_Tree, 1.);

    //Add trainingVariables
    dataloader->AddVariable("mjj_vbs", 'F');
    dataloader->AddVariable("deltaeta_vbs", 'F');
    dataloader->AddVariable("zepp_l", 'F');
    dataloader->AddVariable("zepp_q1", 'F');
    dataloader->AddVariable("zepp_q2", 'F');
    
    //Add SpectatorVariables
    dataloader->AddSpectator ("mjj_vjet", 'F');
    dataloader->AddSpectator ("mww", 'F');
    
    TCut mycuts = "";
    TCut mycutb = "";
    
    dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
                                        "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );
    
    // adding a BDT
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    
    int    NTrees         = 800 ; 
    bool   optimizeMethod = false ; 
    string BoostType      = "AdaBoost" ; 
    float  AdaBoostBeta   = 0.5 ; 
    string PruneMethod    = "NoPruning" ; 
    int    PruneStrength  = 5 ; 
    int    MaxDepth       = 5 ; 
    string SeparationType = "GiniIndex" ;

    TString Option = Form ("!H:!V:CreateMVAPdfs:NTrees=%d:BoostType=%s:AdaBoostBeta=%f:PruneMethod=%s:PruneStrength=%d:MaxDepth=%d:SeparationType=%s:Shrinkage=0.1:UseYesNoLeaf=F:MinNodeSize=2:nCuts=200", 
        NTrees, BoostType.c_str (), AdaBoostBeta, PruneMethod.c_str (), PruneStrength, MaxDepth, SeparationType.c_str ()) ;

//     string BDTname = string ("BDT_") + MVAname ;
    TMVAtest->BookMethod(dataloader, TMVA::Types::kBDT, "BDT_prova", Option.Data()) ;

    // adding a BDTG
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----

    float GradBaggingFraction = 0.6 ; 
    NTrees              = 100 ; 
    optimizeMethod      = false ; 
    PruneMethod         = "NoPruning" ; 
    PruneStrength       = 5 ; 
    MaxDepth            = 2 ; 
    SeparationType      = "GiniIndex" ;
    float Shrinkage     = 0.3;

    Option = Form ("CreateMVAPdfs:NTrees=%d:BoostType=Grad:UseBaggedGrad:BaggedSampleFraction=%f:PruneMethod=%s:PruneStrength=%d:MaxDepth=%d:SeparationType=%s:Shrinkage=%f:UseYesNoLeaf=F:nCuts=600",
        NTrees, GradBaggingFraction, PruneMethod.c_str (), PruneStrength, MaxDepth, SeparationType.c_str (), Shrinkage) ;

//     string BDTGname = string ("BDTG_") + MVAname ;
    TMVAtest->BookMethod(dataloader, TMVA::Types::kBDT,"BDTG_prova", Option.Data()) ;

    // start the training
    // ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    
    TMVAtest->TrainAllMethods () ;
    TMVAtest->TestAllMethods () ;
    TMVAtest->EvaluateAllMethods () ;
    
    outputFile->Close();
    
    delete TMVAtest ;
    delete outputFile ;

//     cout << "\n-====-====-====-====-====-====-====-====-====-====-====-====-====-\n\n" ;
//     cout << "Name tag of the weights file: " << BDTname << "\n" ;  
    if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );
    return 0 ;
}

int main()
{
    return TMVATest();
}