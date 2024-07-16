
friend struct complex;
friend struct polar;
friend complex CLn(const complex& A);
friend double cabs(const complex& A);//#inline
friend double cabs2(const complex& A);//#inline
friend void caccum(complex& A, const complex& B);//#inline
friend void caccumarray(pComplexArray A, pComplexArray B, short int n);
friend complex cadd(const complex& A, const complex& B);//#inline
friend double cang(const complex& A);
friend double cdang(const complex& A);
friend complex cdiv(const complex& A, const complex& B);//#inline
friend complex cdivreal(const complex& A, double B);//#inline
friend complex cinv(const complex& A);//#inline
friend complex cmplx(double A, double B);//#inline
friend complex cmul(const complex& A, const complex& B);//#inline
friend complex cmulreal(const complex& A, double B);//#inline
friend void cmulrealaccum(complex& A, double B);//#inline
friend complex cnegate(const complex& A);//#inline
friend complex conjg(const complex& A);//#inline
friend complex csqrt(const complex& A);
friend complex csub(const complex& A, const complex& B);//#inline
friend polar ctopolar(const complex& A);
friend polar ctopolardeg(const complex& A);
friend polar padd(const polar& A, const polar& B);
friend complex pclx(double magn, double Angle);
friend complex pdegtocomplex(double magn, double Angle);
friend polar pdiv(const polar& A, const polar& B);
friend double pimg(const polar& A);
friend polar pmul(const polar& A, const polar& B);
friend double prel(const polar& A);
friend polar psub(const polar& A, const polar& B);
friend complex ptocomplex(const polar& A);
friend polar topolar(double A, double B);//#inline
friend double ATAN2(double X, double IY);



