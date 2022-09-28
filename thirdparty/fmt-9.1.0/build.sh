cmake -B /tmp/fmt-build -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE .
cmake --build /tmp/fmt-build --target fmt
cmake --install /tmp/fmt-build --prefix /tmp/fmt-install
