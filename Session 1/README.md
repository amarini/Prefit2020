# 1. RooFit
<b>RooFit</b> is a OO analysis environment built on ROOT. It is essentially a collection of classes designed to augment root for data modeling whose aim is summarised below (shamelessly stolen from Wouter Verkerke)...

![](files/roofit.png)

We will cover a few of the basics in the session today but note there are many more tutorials available at this link: [https://root.cern.ch/root/html600/tutorials/roofit/index.html](https://root.cern.ch/root/html600/tutorials/roofit/index.html)


## Objects
In Roofit, any variable, data point, function, PDF (etc.) is represented by a c++ object
The most basic of these is the `RooRealVar`. Let's create one which will represent the mass of some hypothetical particle, we name it and give it an initial starting value and range.

```c++
RooRealVar MH("MH","mass of the Hypothetical Boson (H-boson) in GeV",125,120,130);
MH.Print();
```

ok, great. This variable is now an object we can play around with. We can access this object and modify it's properties, such as its value. 

```c++
MH.setVal(130);
MH.getVal();
MH.Print();
```

In particle detectors we typically don't observe this particle mass but usually define some observable which is *sensitive* to this mass. Lets assume we can detect and reconstruct the decay products of the H-boson and measure the invariant mass of those particles. We need to make another variable which represents that invariant mass.

```c++
RooRealVar mass("m","m (GeV)",100,80,200);
mass.Print();
```

In the perfect world we would perfectly measure the exact mass of the particle in every single event. However, our detectors are usually far from perfect so there will be some resolution effect. Lets assume the resolution of our measurement of the invariant mass is 10 GeV and call it "sigma"

```c++
RooRealVar sigma("resolution","#sigma",10,0,20);
sigma.Print();
```

More exotic variables can be constructed out of these `RooRealVar`s using `RooFormulaVars`. For example, suppose we wanted to make a function out of the variables which represented the relative resolution as a function of the hypothetical mass MH. 

```c++
RooFormulaVar func("R","@0/@1",RooArgList(sigma,mass));
func.Print("v");
```

Notice how there is a list of the variables we passed (the servers or "actual vars"). We can now plot the function. RooFit has a special plotting object `RooPlot` which keeps track of the objects (and their normalisations) which we want to draw. Since RooFit doesn't know the difference between which objects are/aren't dependant, we need to tell it. 

Right now, we have the relative resolution as ![R(m,\sigma)](https://render.githubusercontent.com/render/math?math=R(m%2C%5Csigma)) whereas we want to plot ![R_{\sigma}(m)](https://render.githubusercontent.com/render/math?math=R_%7B%5Csigma%7D(m)).

```c++
TCanvas *can = new TCanvas();

//make the x-axis the "mass"
RooPlot *plot = mass.frame(); 
func.plotOn(plot);

plot->Draw();
can->Draw();
```

The main objects we are interested in using from RooFit are <b>"probability denisty functions"</b> or (PDFs). We can construct the PDF.

![f(m|M_{H},\sigma)](https://render.githubusercontent.com/render/math?math=f(m%7CM_%7BH%7D%2C%5Csigma))

as a simple Gaussian shape for example or a `RooGaussian` in RooFit language (think McDonald's logic, everything is a `RooSomethingOrOther`)

```c++
RooGaussian gauss("gauss","f(m|M_{H},#sigma)",mass,MH,sigma);
gauss.Print("V");
```

Notice how the gaussian PDF, like the `RooFormulaVar` depends on our `RooRealVar` objects, these are its servers.  Its evaluation will depend on their values. 

The main difference between PDFs and Functions in RooFit is that PDFs are <b>automatically normalised to unitiy</b>, hence they represent a probability density, you don't need to normalise yourself. Lets plot it for the current values of $m$ and $\sigma$.

```c++
plot = mass.frame();
    
gauss.plotOn(plot);

plot->Draw();
can->Draw();
```

If we change the value of MH, the PDF gets updated at the same time.

```c++
MH.setVal(125);
gauss.plotOn(plot,RooFit::LineColor(kRed));

MH.setVal(135);
gauss.plotOn(plot,RooFit::LineColor(kGreen));

plot->Draw();

can->Update();
can->Draw();
```

PDFs can be used to generate Monte Carlo data. One of the benefits of RooFit is that to do so only uses a single line of code!

As before, we have to tell RooFit which variables to generate in (e.g which are the observables for an experiment). In this case, each of our events will be a single value of "mass" $m$.

```c++
RooDataSet *data = (RooDataSet*) gauss.generate(RooArgSet(mass),500); 
//The arguments are the set of observables, follwed by the number of events

data->Print();
```

Now we can plot the data as with other RooFit objects.

```c++
plot = mass.frame();

data->plotOn(plot);
gauss.plotOn(plot);
gauss.paramOn(plot);

plot->Draw();
can->Update();
can->Draw();
```

Of course we're not in the business of generating MC events, but collecting <b>real data!</b>. Next we will look at using real data in RooFit...

## Datasets

A dataset is essentially just a collection of points in N-dimensional (N-observables) space. There are two basic implementations in RooFit, 

1) an "unbinned" dataset - `RooDataSet`

2) a "binned" dataset - `RooDataHist`

both of these use the same basic structure as below

![](files/datastructure.png)

Lets create an empty dataset where the only observable, the mass. Points can be added to the dataset one by one ...

```c++
RooDataSet mydata("dummy","My dummy dataset",RooArgSet(mass)); 
// We've made a dataset with one observable (mass)

mass.setVal(123.4);
mydata.add(RooArgSet(mass));
mass.setVal(145.2);
mydata.add(RooArgSet(mass));
mass.setVal(170.8);
mydata.add(RooArgSet(mass));


mydata.Print();
```

There are also other ways to manipulate datasets in this way as shown in the diagram below 

![](files/datasets_manip.png)


Luckily there are also Constructors for a `RooDataSet` from a `TTree` and for a `RooDataHist` from a `TH1` so its simple to convert from your usual ROOT objects.

Let's take an example dataset put together already.

```c++
TFile *file = TFile::Open("tutorial.root");
file->ls();
```

Inside the file, there is something called a `RooWorkspace`. This is just the RooFit way of keeping a persistent link between the objects for a model. It is a very useful way to share data and PDFs/functions etc among CMS collaborators.

Let's take a look at it. It contains a `RooDataSet` and one variable. This time we called our variable (or observable) `CMS_hgg_mass`, let's assume now that this is the invariant mass of photon pairs where we assume our H-boson decays to photons.  

```c++
RooWorkspace *wspace = (RooWorkspace*) file->Get("workspace");
wspace->Print("v");
```

Let's have a look at the data. The `RooWorkspace` has several accessor functions, we will use the `RooWorkspace::data` one. 
There are also `RooWorkspace::var`, `RooWorkspace::function` and `RooWorkspace::pdf` with (hopefully) obvious purposes.

```c++
RooDataSet *hgg_data = (RooDataSet*) wspace->data("dataset");
RooRealVar *hgg_mass = (RooRealVar*) wspace->var("CMS_hgg_mass");

plot = hgg_mass->frame();

hgg_data->plotOn(plot,RooFit::Binning(160)); 
// Here we've picked a certain number of bins just for plotting purposes 

plot->Draw();
can->Update();
can->Draw();
```


# 2. Likelihoods and Fitting to data 

The data we have in our file doesn't look like a Gaussian distribution. Instead, we could probably use something like an exponential to describe it. 

There is an exponential pdf already in RooFit (yep you guessed it) `RooExponential`. For a pdf, we only need one parameter which is the exponential slope $\alpha$ so our pdf is,  

![f(m|\alpha) = \dfrac{1}{N} e^{-\alpha m}](https://render.githubusercontent.com/render/math?math=f(m%7C%5Calpha)%20%3D%20%5Cdfrac%7B1%7D%7BN%7D%20e%5E%7B-%5Calpha%20m%7D)

Where of course, ![N = \int_{110}^{150} e^{-\alpha m} dm](https://render.githubusercontent.com/render/math?math=N%20%3D%20%5Cint_%7B110%7D%5E%7B150%7D%20e%5E%7B-%5Calpha%20m%7D%20dm)is the normalisation constant.

You can fund a bunch of available RooFit functions here: [https://root.cern.ch/root/html/ROOFIT_ROOFIT_Index.html](https://root.cern.ch/root/html/ROOFIT_ROOFIT_Index.html)

There is also support for a generic pdf in the form of a `RooGenericPdf`, check this link: [https://root.cern.ch/doc/v608/classRooGenericPdf.html](https://root.cern.ch/doc/v608/classRooGenericPdf.html)

```c++
RooRealVar alpha("alpha","#alpha",-0.05,-0.2,0.01);
RooExponential expo("exp","exponential function",*hgg_mass,alpha);
```

We can use RooFit to tell us to estimate the value of $\alpha$ using this dataset. You will learn more about parameter estimation but for now we will just assume you know about maximising likelihoods. This <b>maximum likelihood estimator</b> is common in HEP and is known to give unbiased estimates for things like distribution means etc. 

This also introduces the other main use of PDFs in RooFit. They can be used to construct <b>likelihoods</b> easily.

The likelihood $\mathcal{L}$ is defined for a particluar dataset (and model) as being proportional to the probability to observe the data assuming some pdf. For our data, the probability to observe an event with a value in an interval bounded by a and b is given by,

![P\left(m \epsilon \[a,b\] \right) = \int_{a}^{b} f(m|\alpha)dm](https://render.githubusercontent.com/render/math?math=P%5Cleft(m%20%5Cepsilon%20%5Ba%2Cb%5D%20%5Cright)%20%3D%20%5Cint_%7Ba%7D%5E%7Bb%7D%20f(m%7C%5Calpha)dm)


As that interval shrinks we can say this probability just becomes equal to ![f(m|\alpha)dm](https://render.githubusercontent.com/render/math?math=f(m%7C%5Calpha)dm)

The probability to observe the dataset we have is given by the product of such probabilities for each of our data points, so that 

![\mathcal{L}(\alpha) \propto \prod_{i} f(m_{i}|\alpha)](https://render.githubusercontent.com/render/math?math=%5Cmathcal%7BL%7D(%5Calpha)%20%5Cpropto%20%5Cprod_%7Bi%7D%20f(m_%7Bi%7D%7C%5Calpha))

Note that for a specific dataset, the $dm$ factors which should be there are constnant. They can therefore be absorbed into the constant of proportionality!

The maximum likelihood esitmator for $\alpha$, usually written as $\hat{\alpha}$, is found by maximising $\mathcal{L}(\alpha)$.

Note that this won't depend on the value of the constant of proportionality so we can ignore it. This is true in most scenarios because usually only the <b>ratio</b> of likelihoods is needed, in which the constant factors out. 

Obviously this multiplication of exponentials can lead to very large (or very small) numbers which can lead to numerical instabilities. To avoid this, we can take logs of the likelihood. Its also common to multiply this by -1 and minimize the resulting <b>N</b>egative <b>L</b>og <b>L</b>ikelihood</b> : $\mathrm{-Log}\mathcal{L}(\alpha)$.

RooFit can construct the <b>NLL</b> for us.

```c++
RooNLLVar *nll = (RooNLLVar*) expo.createNLL(*hgg_data);
nll->Print("v");
```

Notice that the NLL object knows which RooRealVar is the parameter because it doesn't find that one in the dataset. This is how RooFit distiguishes between <it>observables</it> and <it>parameters</it>.

RooFit has an interface to Minuit via the `RooMinimizer` class which takes the NLL as an argument. To minimize, we just call the `RooMinimizer::minimize()` function. "Minuit2" is the program and "migrad" is the minimization routine which uses gradient descent.

```c++
RooMinimizer minim(*nll);
minim.minimize("Minuit2","migrad");  
```

RooFit has found the best fit value of alpha for this dataset. It also estimates an uncertainty on alpha using the Hessian matrix from the fit.

```c++
alpha.Print("v");
```

Lets plot the resulting exponential on the data. Notice that the value of $\hat{\alpha}$ is used for the exponential. 

```c++
expo.plotOn(plot);
expo.paramOn(plot);
plot->Draw();
can->Update();
can->Draw();
```

It looks like there could be a small region near 125 GeV for which our fit doesn't quite go through the points. Maybe our hypothetical H-boson isn't so hypothetical after all!

Let's see what happens if we include some resonant signal into the fit. We can take our Gaussian function again and use that as a signal model. A reasonable value for the resolution of a resonant signal with a mass around 125 GeV decaying to a pair of photons is around a GeV.

```c++
sigma.setVal(1.);
sigma.setConstant();

MH.setVal(125);
MH.setConstant();

RooGaussian hgg_signal("signal","Gaussian PDF",*hgg_mass,MH,sigma);
```

By setting these parameters constant, RooFit knows (either when creating the NLL by hand or when using `fitTo`) that there is not need to fit for these parameters. 

We need to add this to our exponential model and fit a "Sigmal+Background model" by creating a `RooAddPdf`. In RooFit there are two ways to add PDFs, recursively where the fraction of yields for the signal and background is a parameter or absolutely where each PDF has its own normalisation. We're going to use the second one.

```c++
RooRealVar norm_s("norm_s","N_{s}",10,100);
RooRealVar norm_b("norm_b","N_{b}",0,1000);

const RooArgList components(hgg_signal,expo);
const RooArgList coeffs(norm_s,norm_b);

RooAddPdf model("model","f_{s+b}",components,coeffs);
model.Print("v");
```

Ok now lets fit the model. Note this time we add the option `Extended()` which tells RooFit that we care about the overall number of observed events in the data $n$ too. It will add an additional Poisson term in the likelihood to account for this so our likelihood this time looks like,

![L_{s+b}(N_{s},N_{b},\alpha) = \dfrac{ N_{s}+N_{b}^{n} e^{N_{s}+N_{b}} }{n!} \cdot \prod_{i}^{n} \left\[ c f_{s}(m_{i}|M_{H},\sigma)+ (1-c)f_{b}(m_{i}|\alpha)  \right\]](https://render.githubusercontent.com/render/math?math=L_%7Bs%2Bb%7D(N_%7Bs%7D%2CN_%7Bb%7D%2C%5Calpha)%20%3D%20%5Cdfrac%7B%20N_%7Bs%7D%2BN_%7Bb%7D%5E%7Bn%7D%20e%5E%7BN_%7Bs%7D%2BN_%7Bb%7D%7D%20%7D%7Bn!%7D%20%5Ccdot%20%5Cprod_%7Bi%7D%5E%7Bn%7D%20%5Cleft%5B%20c%20f_%7Bs%7D(m_%7Bi%7D%7CM_%7BH%7D%2C%5Csigma)%2B%20(1-c)f_%7Bb%7D(m_%7Bi%7D%7C%5Calpha)%20%20%5Cright%5D)

where $c = \dfrac{ N_{s} }{ N_{s} + N_{b} }$,   $f_{s}(m|M_{H},\sigma)$ is the Gaussian signal pdf and $f_{b}(m|\alpha)$ is the exponential pdf. Remember that $M_{H}$ and $\sigma$ are fixed so that they are no longer parameters of the likelihood.

There is a simpler interface for maximum likelihood fits which is the `RooAbsPdf::fitTo` method. With this simple method, RooFit will construct the negative log-likelihood function, from the pdf, and minimize all of the free parameters in one step.

```c++
model.fitTo(*hgg_data,RooFit::Extended());

model.plotOn(plot,RooFit::Components("exp"),RooFit::LineColor(kGreen));
model.plotOn(plot,RooFit::LineColor(kRed));
model.paramOn(plot);

can->Clear();
plot->Draw();
can->Update();
can->Draw();
```

What about if we also fit for the mass ($M_{H}$)? we can easily do this by removing the constant setting on MH.

```c++
MH.setConstant(false);
model.fitTo(*hgg_data,RooFit::Extended());
```

Notice now the result for the fitted MH is not 125 and gets added to the fitted parameters since now it is floating.
We can get more information about the fit, via the `RooFitResult`, using the option `Save()`. 

```c++
RooFitResult *fit_res = (RooFitResult*) model.fitTo(*hgg_data,RooFit::Extended(),RooFit::Save());
fit_res->Print("v");
```

For example, we can get the Correlation Matrix from the fit result... Note that the order of the parameters are the same as listed in the "Floating Parameter" list above

```c++
TMatrixDSym cormat = fit_res->correlationMatrix();
cormat.Print();
```

And we can also visualise these results using some built-in RooFit visualisation tools 

```c++
TCanvas *can2 = new TCanvas("c","c",1000,460);
can2->Divide(2);

can2->cd(1);

RooPlot plot_err_mat(norm_b,norm_s,123,126,0,80);
fit_res->plotOn(&plot_err_mat,MH,norm_s,"MEVH12");
plot_err_mat.Draw();

can2->cd(2);

plot = hgg_mass->frame();
model.plotOn(plot,RooFit::VisualizeError(*fit_res,1),RooFit::FillColor(kOrange));
model.plotOn(plot);
plot->Draw();

can2->Draw();
```

A nice feature of RooFit is that once we have a pdf, data and results like this, we can import this new model into our `RooWorkspace` and show off our new discovery to our LHC friends (if we weren't about 5 years too late!). We can also save the "state" of our parameters for later, by creating a snapshot of the current values. 

```c++
wspace->import(model);  
RooArgSet *params = model.getParameters(*hgg_data);
wspace->saveSnapshot("nominal_values",*params);

wspace->Print("V");
```

# 3. Nuisance Parameters 


In HEP, we often have the case where there are some parameters of the model which are physics parameters of interest, while others are known as nuisance parameters. In some cases, those parameters may have external constraints, coming from other data or some theoretical reasoning. This is how we often treat <b>systematic uncertainties</b> but we won't cover those today.

There are two schools of thought for removing nuisance parameters 

   * Frequentists use profiling 
   * Bayesians use marginalisation

In our case, the hypothesis mass $M_{H}$ might be the thing which we can write papers on and win Nobel prizes for while we're not nearly so interested in the values of $N_{s}$, $\alpha$ or $N_{b}$. 

For simplicity, we will assume that the values of $\alpha$ and $N_{b}$ are fixed (maybe from theory or from a large Monte Carlo simulation) and so the only remaining nuisance paraemeter is $N_{s}$. 

```c++
alpha.setConstant(true);
norm_b.setConstant(true);
```

## Profiling nuisance parameters

The <b>profile likelihood</b> is one which removes the nuisances parameters by fitting them away. Note this is a major divide for Bayesians and Freqeuentist since removing nuisance parameters for Bayesians involves integrating (or marginalising) over them. 

The profile likeihood is written as, 

![\mathcal{L}_{p}(M_{H})= \mathcal{L}_{s+b}(M_{H},\hat{\hat{N}}_{s})](https://render.githubusercontent.com/render/math?math=%5Cmathcal%7BL%7D_%7Bp%7D(M_%7BH%7D)%3D%20%5Cmathcal%7BL%7D_%7Bs%2Bb%7D(M_%7BH%7D%2C%5Chat%7B%5Chat%7BN%7D%7D_%7Bs%7D))

where the double hat notation, $\hat{\hat{\cdot}}$, denotes the values of the nuisance parameter which maximises  $L_{s+b}$ at a given value of $M_{H}$.

RooFit has a useful class under the RooStats libraries for profile likelihoods: `ProfileLikelihoodCalculator` : [https://root.cern.ch/doc/v606/classRooStats_1_1ProfileLikelihoodCalculator.html](https://root.cern.ch/doc/v606/classRooStats_1_1ProfileLikelihoodCalculator.html)

We will set the range of the mass parameter to something reasonable for the scan 

```c++
MH.setRange(123,126);
RooStats::ProfileLikelihoodCalculator plc(*hgg_data,model,RooArgSet(MH));
```

According to Wilks' theorem, -2 times the log of profile likelihood ratio, 

![-2\log \lambda = -2\log \dfrac{\mathcal{L}(M_{H})}{\mathcal{L}(\hat{M}_{H})}](https://render.githubusercontent.com/render/math?math=-2%5Clog%20%5Clambda%20%3D%20-2%5Clog%20%5Cdfrac%7B%5Cmathcal%7BL%7D(M_%7BH%7D)%7D%7B%5Cmathcal%7BL%7D(%5Chat%7BM%7D_%7BH%7D)%7D)

should be distributed as a $\chi^{2}$ with the number degrees of freedom equal to the number of parameters (in our case 1). This means that we can obtain the $1\sigma$ (or 68%) uncertainty by finding the interval for which 

![-\log \lambda(M_{H})\lt0.5](https://render.githubusercontent.com/render/math?math=-%5Clog%20%5Clambda(M_%7BH%7D)%5Clt0.5)

We can tell RooFit to find that interval for us, 

```c++
plc.SetConfidenceLevel(0.68);
RooStats::LikelihoodInterval *interval = plc.GetInterval();

std::cout << "\n\t hat{MH} = " 
    << ((RooRealVar*)interval->GetBestFitParameters()->find("MH"))->getVal() 
    << std::endl;

std::cout << "\t 68% interval = (" 
    << interval->LowerLimit(MH) << "," << interval->UpperLimit(MH) << ")" 
    << std::endl;
```

We can see where these values come from by drawing the negative of the profile likelihood ratio.

```c++
RooStats::LikelihoodIntervalPlot plotLI(interval);

TCanvas *canLI = new TCanvas(); 
plotLI.Draw();
canLI->Draw();
```

## Bayesian marginalisation 

We can also obtain a Bayesian <b>posterior</b> density by using Bayes' theorem ...

![P(M_{H}|\mathrm{data}) = \frac{ \int P(\mathrm{data}|M_{H},N_{s}) \pi(N_{s}|M_{H}) dN_{s}}{P(\mathrm{data})}](https://render.githubusercontent.com/render/math?math=P(M_%7BH%7D%7C%5Cmathrm%7Bdata%7D)%20%3D%20%5Cfrac%7B%20%5Cint%20P(%5Cmathrm%7Bdata%7D%7CM_%7BH%7D%2CN_%7Bs%7D)%20%5Cpi(N_%7Bs%7D%7CM_%7BH%7D)%20dN_%7Bs%7D%7D%7BP(%5Cmathrm%7Bdata%7D)%7D)

where $\pi(N_{s}|M_{H})$ is the (condidtional) <b>prior</b> probability density for our parameters. You might notice that  $P(\mathrm{data}|M_{H},N_{s})$ is nothing more than our likelihood function $\mathcal{L}(M_{H},N_{s})$.

Notice that I can use "$=$" here because we normalise according to $P(\mathrm{data})$, which is the integral over the likelihood (multiplied by the prior) with respect to <b>all</b> of its parameters. 

We can get RooFit to calculate this posterior using the `BayesianCalculator` class but first, for this time, we are going to create a `ModelConfig` which keeps track of which parameters are POIs and which are nuisances/observables. This is useful when sharing your model between collegues. 

```c++
RooStats::ModelConfig mconfig("mymodel");
mconfig.SetWorkspace(*wspace);

mconfig.SetPdf("model");
mconfig.SetParametersOfInterest("MH");
mconfig.SetObservables("CMS_hgg_mass");
mconfig.SetNuisanceParameters("norm_s");
```

There are a few things to set up since we're using variables inside the workspace. First, lets set our two background  parameters to constant, this time, directly in the workspace

```c++
wspace->var("norm_b")->setConstant(true);
wspace->var("alpha")->setConstant(true);
```

And we will load the nominal values from the fit before...

```c++
wspace->loadSnapshot("nominal_values");
wspace->allVars().Print("V");
```

For Bayesian results, we need to set the prior density function ($\pi$). For now, let's assume no prior knowledge of the parameters - i.e the prior is constant within the range of the parameters and 0 elsewhere. 

Note that choosing a suitable prior is a debated subject amongst Bayesians since the results can depend on what you choose as a prior. Worse still, if a flat prior is chosen, say for $M_{H}$, should it be flat in $M_{H}$, or flat in $M_{H}^{2}$ (eg since $M_{H}=2\lambda v^{2}$ and maybe $\lambda$ is the fundamental thing)?

```c++
wspace->var("MH")->setRange(123,126);

RooUniform flat_p("flat_prior","flat_prior",RooArgSet(MH,norm_s));
wspace->import(flat_p);
mconfig.SetPriorPdf("flat_prior");

mconfig.Print("v");
```

Now we can create the calculator and plot the posterior pdf...

```c++
RooStats::BayesianCalculator bCalculator(*hgg_data,mconfig);
bCalculator.SetConfidenceLevel(0.68);
bCalculator.Print("v");
RooPlot *plotBC = bCalculator.GetPosteriorPlot();

TCanvas *cBC = new TCanvas();
plotBC->Draw();
cBC->Draw();
```

This was the "shortest" interval, but we can also find the other valid intervals $[a,b]$ which satisfy $P(M_{H}~\epsilon~[a,b]) =0.68$. Let's compare the Bayesian 68% credible interval with our 65% confidence interval from the profile likelihood scan.

```c++
RooStats::SimpleInterval *intervalBayes 
    = (RooStats::SimpleInterval*)bCalculator.GetInterval();

std::cout << "\n\t 68% (Bayes shortest) interval \t= (" 
    << intervalBayes->LowerLimit() << "," << intervalBayes->UpperLimit() << ")" 
    << std::endl;

std::cout << "\t 68% (frequentist) interval \t= (" 
    << interval->LowerLimit(MH) << "," << interval->UpperLimit(MH) << ")" 
    << std::endl;
```

They agree pretty well for our analysis but beware that this is not always the case! 

## Feldman-Cousins 

Now that we have seen intervals constructed from profile likelihoods and Bayesian marginalisation (credible intervals), have a go at constructing the Feldman-Cousins interval. 

Try the following procedure for computing Feldman-cousins confidence intervals for $m_{H}$,

-   use the profile likelihood as the test-statistic $q(x) = - 2 \ln \mathcal{L}(\mathrm{data}|x,\hat{\theta}_{x})/\mathcal{L}(\mathrm{data}|\hat{x},\hat{\theta})$ where $x$ is a point particular value of $m_{H}$, and $\hat{x}$ is the point corresponding to the best fit. In this test-statistic, the nuisance parameters are profiled, separately both in the numerator and denominator.
-   for each point $x$:
    -   compute the observed test statistic $q_{\mathrm{obs}}(x)$
    -   compute the expected distribution of $q(x)$ under the hypothesis of $x$ as the true value.
    -   accept the point in the region if $p_{x}=P\left[q(x) > q_{\mathrm{obs}}(x)| x\right] > \alpha$

With a critical value $\alpha$. A particular value of $x$ belongs to your confidence region if $p_{x}$ is larger than $\alpha$ For example for a 68% CL confidence interval, use $\alpha=0.32$ (since $1-\alpha=0.68$).

You should have learned enough so far to do this as you know now how to set parameters, freeze or unfreeze them, throw toy data and perform likelihood fits - Ask any of the instructors for help if you get stuck!
