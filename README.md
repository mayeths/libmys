# LibMYS

Mayeths' header only library (wow@mayeths.com or huanghp22@mails.tsinghua.edu.cn).

- **mys**: **M**a**y**eth**s**' basis headers
    - ***.h**: Headers that work under C99
    - ***.hpp**: Headers that work under C++11
- **myss**: **M**a**y**eth**s**' **S**olver
    - **iss**: **I**terative **S**parse **S**olver
    - **dss**: **D**irect **S**parse **S**olver (Nothing currently)
    - **pc**: **P**re**c**onditioner
    - **mat**: **Mat**rix structures
    - **vec**: **Vec**tor structures

### Usage of myss

```c++
    MPetsc A(nnz, Ia, Ja, Va);
    VPetsc x, b;
    PCPetsc pc(A); // or PCNone<MPetsc, VPetsc> pc;
    A.CreateVecs(x, b);
    x.SetValues(xv);
    b.SetValues(bv);
    double res = A.Norm();
    double xnorm = x.Norm();
    double bnorm = b.Norm();
    DEBUG(0, "A norm %.17E x norm %.17E b norm %.17E", res, xnorm, bnorm);
    PIPECG<MPetsc, VPetsc> solver(A, pc); // or CG<MPetsc, VPetsc> solver(A);
    solver.atol = 1.085397E+03;
    solver.rtol = 1e-12;
    solver.maxiter = 124;
    solver.Apply(b, x);
    solver.View("\n"); // Add a blank line before view
```

Some result:

```
...
Iteration   51 ||r|| 1.29834863654358901e+03 ||r||/||b|| 7.30696054936851875e-08
Iteration   52 ||r|| 1.11964403235359759e+03 ||r||/||b|| 6.30123107420767324e-08
Iteration   53 ||r|| 9.47525549864544701e+02 ||r||/||b|| 5.33256755351204192e-08

PIPECG @0xffffcf867038
  Matrix: MPetsc @0xffffcf867008
  Preconditioner: PCPetsc @0xffffcf866f70
  Status: ConvergedByAtol
  > abs 947.5255498645447 (atol 1085.3969999999999)
    rel 5.3325675535120419e-08 (rtol 9.9999999999999998e-13 dtol 1000000)
    iter 53 (maxiter 124)
```
