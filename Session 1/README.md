# RooFit
<b>RooFit</b> is a OO analysis environment built on ROOT. It is essentially a collection of classes designed to augment root for data modeling whose aim is summarised below (shamelessly stolen from Wouter Verkerke)...

![](files/roofit.png)

We will cover a few of the basics in the session today but note there are many more tutorials available at this link: [https://root.cern.ch/root/html600/tutorials/roofit/index.html](https://root.cern.ch/root/html600/tutorials/roofit/index.html)


## Objects
In Roofit, any variable, data point, function, PDF ... is represented by a c++ object
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

Test for equation next

![P(M_{H}|\mathrm{data}) = \frac{ \int P(\mathrm{data}|M_{H},N_{s}) \pi(N_{s}|M_{H}) dN_{s}}{P(\mathrm{data})}](https://render.githubusercontent.com/render/math?math=P(M_%7BH%7D%7C%5Cmathrm%7Bdata%7D)%20%3D%20%5Cfrac%7B%20%5Cint%20P(%5Cmathrm%7Bdata%7D%7CM_%7BH%7D%2CN_%7Bs%7D)%20%5Cpi(N_%7Bs%7D%7CM_%7BH%7D)%20dN_%7Bs%7D%7D%7BP(%5Cmathrm%7Bdata%7D)%7D)
