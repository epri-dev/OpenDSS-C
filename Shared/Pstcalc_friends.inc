
friend void FlickerMeter(int n, double FBase, double VBase, Arraydef::pSingleArray Pt, Arraydef::pSingleArray(&pRms)[100/*# range 1..100*/], Arraydef::pSingleArray(&pPst)[100/*# range 1..100*/]);
friend int PstRMS(Arraydef::pDoubleArray(&PstResult)[100/*# range 1..100*/], Arraydef::pDoubleArray pVoltages, double Freqbase, int NcyclesperSample, int npts, int Lamp);
friend double CalcPst();
friend void Fhp(int n, float TS, float whp, Arraydef::pSingleArray X, Arraydef::pSingleArray(&Y)[100/*# range 1..100*/]);
friend void Flp(int n, float TS, float tau, Arraydef::pSingleArray X, Arraydef::pSingleArray(&Y)[100/*# range 1..100*/]);
friend void Fw1(int n, float TS, float W1, float k, float lam, Arraydef::pSingleArray X, Arraydef::pSingleArray(&Y)[100/*# range 1..100*/]);
friend void Fw2(int n, float TS, float W2, float W3, float W4, Arraydef::pSingleArray X, Arraydef::pSingleArray(&Y)[100/*# range 1..100*/]);
friend void Gather_Bins(double X10_value, pBinArray bins);
friend void Get_Pinst();
friend void Init6Array(Double6Array(&Y)[6/*# range 0..5*/], double V1, double V2, double V3, double V4, double V5, double V6);
friend float Percentile(OpenArrayRef<float> List, int iLo, int iHi, float pctExceeded);
friend void QuickSort(OpenArrayRef<float> List, int iLo, int iHi);
friend double SB(double Y, pBinArray bins);
friend void Sample_Shift();
friend void Set_Filter_Coefficients(int input_type);
friend void ZeroOutBins();
friend int _Pst(Arraydef::pDoubleArray(&PstResult)[100/*# range 1..100*/], Arraydef::pDoubleArray VArray, int npts);



