#pragma once

#define RMIDX(row, col, nrow, ncol) ((row) * (ncol) + (col)) /* row major index */
#define CMIDX(row, col, nrow, ncol) ((row) + (nrow) * (col)) /* col major index */
#define IDX2(x, y, nx, ny) ((x) + (y) * (nx))
#define IDX3(x, y, z, nx, ny, nz) ((x) + (y) * (nx) + (z) * (nx) * (ny))

#define REP2(inst) do { inst; inst; } while (0)
#define REP4(inst) do { REP2(inst); REP2(inst); } while (0)
#define REP8(inst) do { REP4(inst); REP4(inst); } while (0)
#define REP16(inst) do { REP8(inst); REP8(inst); } while (0)
#define REP32(inst) do { REP16(inst); REP16(inst); } while (0)
#define REP64(inst) do { REP32(inst); REP32(inst); } while (0)
#define REP128(inst) do { REP64(inst); REP64(inst); } while (0)
#define REP256(inst) do { REP128(inst); REP128(inst); } while (0)
#define REP512(inst) do { REP256(inst); REP256(inst); } while (0)
#define REP1024(inst) do { REP512(inst); REP512(inst); } while (0)
#define REP5(inst) do { inst; inst; inst; inst; inst; } while (0)
#define REP10(inst) do { REP5(inst); REP5(inst); } while (0)
#define REP50(inst) do { REP10(inst); REP10(inst); REP10(inst); REP10(inst); REP10(inst); } while (0)
#define REP100(inst) do { REP50(inst); REP50(inst); } while (0)
#define REP500(inst) do { REP100(inst); REP100(inst); REP100(inst); REP100(inst); REP100(inst); } while (0)
#define REP1000(inst) do { REP500(inst); REP500(inst); } while (0)
