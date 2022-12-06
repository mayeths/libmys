### Usage of myss

```c++
    MPetsc A(nnz, Ia, Ja, Va);
    VPetsc x, b;
    PCPetsc pc(A); // PCNone<MPetsc> pc;
    A.CreateVecs(x, b);
    x.SetValues(xv);
    b.SetValues(bv);
    double Anorm = A.Norm();
    double xnorm = x.Norm();
    double bnorm = b.Norm();
    DEBUG(0, "A norm %.17E x norm %.17E b norm %.17E", Anorm, xnorm, bnorm);
    PIPECG<MPetsc> solver(A, pc); // CG<MPetsc> solver(A, pc);
    solver.atol = 1.085397E+03;
    solver.rtol = 1e-12;
    solver.maxiter = 124;
    solver.Apply(b, x);
    solver.View("\n"); // Add a blank line before view
```

Some result:

```
...
Iteration   50 ||r|| 1.50924748729640214e+03 ||r||/||b|| 8.49387563441103166e-08
Iteration   51 ||r|| 1.29834863654358901e+03 ||r||/||b|| 7.30696054936851875e-08
Iteration   52 ||r|| 1.11964403235359759e+03 ||r||/||b|| 6.30123107420767324e-08
Iteration   53 ||r|| 9.47525549864544701e+02 ||r||/||b|| 5.33256755351204192e-08

PIPECG @0xffffd2e920f8
  Matrix: MPetsc @0xffffd2e920c8
  Preconditioner: PCPetsc @0xffffd2e92030
  Status: ConvergedByAtol
  > abs 947.5255498645447 (atol 1085.3969999999999)
    rel 5.3325675535120419e-08 (rtol 9.9999999999999998e-13 dtol 1000000)
    iter 53 (maxiter 124)
```
