
friend void AB02Phase(Ucomplex::pComplexArray Vph, Ucomplex::pComplexArray VaB0);
friend Ucomplex::complex Bessel_I0(const Ucomplex::complex& A);
friend Ucomplex::complex Bessel_I1(const Ucomplex::complex& X);
friend void CalcKPowers(Ucomplex::pComplexArray kWkvar, Ucomplex::pComplexArray V, Ucomplex::pComplexArray i, int n);
friend void CurveMeanAndStdDev(Arraydef::pDoubleArray pY, Arraydef::pDoubleArray pX, int n, double& Mean, double& StdDev);
friend void DblInc(double& X, double Y);//#inline
friend void ETKInvert(Arraydef::pDoubleArray A, int Norder, int& Error);
friend double Gauss(double Mean, double StdDev);
friend double GetXR(const Ucomplex::complex& A);
friend Ucomplex::complex ParallelZ(const Ucomplex::complex& Z1, const Ucomplex::complex& Z2);
friend double PctNemaUnbalance(Ucomplex::pComplexArray Vph);
friend void Phase2AB0(Ucomplex::pComplexArray Vph, Ucomplex::pComplexArray VaB0);
friend void Phase2SymComp(Ucomplex::pComplexArray Vph, Ucomplex::pComplexArray V012);
friend double QuasiLogNormal(double Mean);
friend void RCDMeanAndStdDev(void* pData, int Ndata, double& Mean, double& StdDev);
friend void SymComp2Phase(Ucomplex::pComplexArray Vph, Ucomplex::pComplexArray V012);
friend Ucomplex::complex TerminalPowerIn(Ucomplex::pComplexArray V, Ucomplex::pComplexArray i, int NPhases);
friend long double /*register*/ RCDSUM(void* Data, int Count);
friend void SetAMatrix(Ucmatrix::TcMatrix* Amat);
friend void SetClarkeMatrices();



