double q(RooRealVar *x, RooAbsPdf *model, RooDataSet *data){
  
   RooNLLVar *nll = (RooNLLVar*) model->createNLL(*data); 

   // at the end, we'll reset the value back to where it started.
   double x_original = x->getVal();

   // numerator  
   x->setConstant(true);
   RooMinimizer minimN(*nll);
   minimN.minimize("Minuit","migrad");
   double nllN = nll->getVal(); 

   // denominator
   x->setConstant(false);
   RooMinimizer minimD(*nll);
   minimD.minimize("Minuit","migrad");
   double nllD = nll->getVal(); 
   
   x->setVal(x_original);

   return 2*(nllN - nllD);
}

void my_fc(){
  
  RooMsgService::instance().setGlobalKillBelow(RooFit::ERROR);
  RooMsgService::instance().setSilentMode(true);

  // Directly taken from the hands-on part of Session 1 
  TFile *file = TFile::Open("../Session 1/tutorial.root");
  RooWorkspace *wspace = (RooWorkspace*) file->Get("workspace");
  RooDataSet *hgg_data = (RooDataSet*) wspace->data("dataset");
  RooRealVar *hgg_mass = (RooRealVar*) wspace->var("CMS_hgg_mass");
  
  // background function 
  RooRealVar alpha("alpha","#alpha",-0.05,-0.2,0.01);
  RooExponential expo("exp","exponential function",*hgg_mass,alpha);

  // signal function 
  RooRealVar MH("MH","mass of the Hypothetical Boson (H-boson) in GeV",125,122,129);
  RooRealVar sigma("resolution","#sigma",1,0,2); sigma.setConstant(true);
  RooGaussian hgg_signal("signal","Gaussian PDF",*hgg_mass,MH,sigma);

  // summed model 
  RooRealVar norm_s("norm_s","N_{s}",10,100);
  RooRealVar norm_b("norm_b","N_{b}",0,1000);

  const RooArgList components(hgg_signal,expo);
  const RooArgList coeffs(norm_s,norm_b);

  RooAddPdf model("model","f_{s+b}",components,coeffs);
  // First a basic fit to the data 
  model.fitTo(*hgg_data,RooFit::Extended());
  
  alpha.setConstant(true);
  norm_b.setConstant(true);
  
  double reset_norm_s = norm_s.getVal(); 

  // Feldman-cousins interval for MH ------------------------------
  //
  // number of toys for each point in FC scan
  int number_of_toys = 1000; 

  // Make a graph to keep the points of x vs p_x 
  TGraph *graph = new TGraph();
  int gp = 0; 

  // Also save a histogram for one of the points, to show the calculation 
  TH1F *h_125 = new TH1F("h_125",";q_{x} for m_{H}=125 GeV;f(q_{x}|m_{H}=125)",100,0,12);
  double q_125 = 0; 

  for (double mh_x=124; mh_x<125.5;mh_x+=0.05){
    MH.setVal(mh_x);
    double q_obs = q(&MH, &model, hgg_data);
    if (fabs(mh_x-125)<0.001) q_125=q_obs;
    // calculate px from toys 
    int p_pass=0, p_tot=0; 
    
    // Note that the value of norm_s may have changed so lets also reset it before throwing the toy 
    for (int t=0;t<number_of_toys;t++){
      norm_s.setVal(reset_norm_s);
      RooDataSet *toy_data = model.generate(RooArgSet(*hgg_mass));
      double q_t =  q(&MH, &model, toy_data);
      if (fabs(mh_x-125)<0.001) h_125->Fill(q_t);
      if (q_t > q_obs) p_pass++;
      p_tot++;
    }
    
    double px = ((double)p_pass) / p_tot; 
    std::cout << " at MH="<< mh_x << ", q_x (obs) = " <<  q_obs << ", px=" << p_pass << " / " << p_tot << " = "  << px << std::endl ; 
    graph->SetPoint(gp,mh_x,px);
    gp++;
  }

  h_125->Scale(1./h_125->Integral("width"));
  h_125->SetLineColor(4);
  h_125->SetMinimum(0.001);
  TLine L_125(q_125,0.001,q_125,0.1);
  L_125.SetLineColor(1); 

  graph->SetLineColor(4); 
  graph->SetLineWidth(2); 
  graph->SetMarkerStyle(20); 
  graph->SetMarkerSize(1.2);

  TCanvas *can = new TCanvas("c","c",800,400); 

  can->Divide(2);
  can->cd(1); 
  h_125->Draw("hist"); 
  L_125.Draw(); 

  can->cd(2);
  graph->Draw("ACP");
  graph->GetXaxis()->SetTitle("MH");
  graph->GetYaxis()->SetTitle("p_{x}");
  TLine L(124,0.32,125.5,0.32);
  L.SetLineColor(2); 
  L.Draw(); 

  can->Update(); 

  //return 0; 
}
