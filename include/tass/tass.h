#pragma once
#ifndef TASS_H_
#define TASS_H_

#include "vec/VAbstract.hpp"
#include "vec/VDense.hpp"
#include "vec/VPetsc.hpp"

#include "mat/MAbstract.hpp"
#include "mat/MPetsc.hpp"

#include "pc/PCAbstract.hpp"
#include "pc/PCNone.hpp"
#include "pc/PCPetsc.hpp"
#include "pc/PCPetscHpss.hpp"

#include "iss/ISSAbstract.hpp"
#include "iss/CG.hpp"
#include "iss/PIPECG.hpp"

#endif /*TASS_H_*/