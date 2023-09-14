## Compile time records

On Kunpeng920, GCC-9.3.1

```bash
time gcc -I/storage/hpcauser/huanghp/project/libmys/include -Wall -Wextra -Werror  basic-gcc.c
time g++ -std=c++11 -I/storage/hpcauser/huanghp/project/libmys/include -Wall -Wextra -Werror  basic-g++.cpp
time mpicc -I/storage/hpcauser/huanghp/project/libmys/include -Wall -Wextra -Werror  basic-mpicc.c
time mpicxx  -std=c++11 -I/storage/hpcauser/huanghp/project/libmys/include -Wall -Wextra -Werror  basic-mpic++.cpp
```

| Commit  | gcc            | g++            | mpicc          | mpicxx         | Reason to record |
|---------|----------------|----------------|----------------|----------------|------------------|
| 404bc82 | 0.322          | 0.923          | 0.367          | 1.614          | First timing record |
| 150f1c6 | 0.371 (-15.2%) | 0.954 (-3.40%) | 0.416 (-13.4%) | 1.654 (-2.50%) | Add utlist & mys_guard |
