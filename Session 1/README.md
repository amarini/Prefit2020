# RooFit
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

