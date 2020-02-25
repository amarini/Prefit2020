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

The main difference between PDFs and Functions in RooFit is that PDFs are <b>automatically normalised to unitiy</b>, hence they represent a probability density, you don't need to normalise yourself. Lets plot it for the current values of m and sigma.

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

As before, we have to tell RooFit which variables to generate in (e.g which are the observables for an experiment). In this case, each of our events will be a single value of "mass" *m*.

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

![ P\left(m~\epsilon~\[a,b\] \right) = \int_{a}^{b} f(m|\alpha)dm ](https://render.githubusercontent.com/render/math?math=%20P%5Cleft(m~%5Cepsilon~%5Ba%2Cb%5D%20%5Cright)%20%3D%20%5Cint_%7Ba%7D%5E%7Bb%7D%20f(m%7C%5Calpha)dm%20)

As that interval shrinks we can say this probability just becomes equal to ![f(m|\alpha)dm](https://render.githubusercontent.com/render/math?math=f(m%7C%5Calpha)dm)

The probability to observe the dataset we have is given by the product of such probabilities for each of our data points, so that 

![\mathcal{L}(\alpha) \propto \prod_{i} f(m_{i}|\alpha)](https://render.githubusercontent.com/render/math?math=%5Cmathcal%7BL%7D(%5Calpha)%20%5Cpropto%20%5Cprod_%7Bi%7D%20f(m_%7Bi%7D%7C%5Calpha))

Note that for a specific dataset, the $dm$ factors which should be there are constnant. They can therefore be absorbed into the constant of proportionality!

The maximum likelihood esitmator for $\alpha$, usually written as $\hat{\alpha}$, is found by maximising $\mathcal{L}(\alpha)$.

Note that this won't depend on the value of the constant of proportionality so we can ignore it. This is true in most scenarios because usually only the <b>ratio</b> of likelihoods is needed, in which the constant factors out. 

Obviously this multiplication of exponentials can lead to very large (or very small) numbers which can lead to numerical instabilities. To avoid this, we can take logs of the likelihood. Its also common to multiply this by -1 and minimize the resulting <b>N</b>egative <b>L</b>og <b>L</b>ikelihood</b> : $-Log \mathcal{L}(\alpha)$.

RooFit can construct the <b>NLL</b> for us.
