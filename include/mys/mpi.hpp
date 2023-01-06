/* https://gist.github.com/2b-t/50d85115db8b12ed263f8231abf07fa2?permalink_comment_id=4274936#gistcomment-4274936 */
#pragma once

#include <type_traits>
#include <complex>
#include <map>

#include "config.h"
#include "macro.h"
#include "mpi.h"

/**
 * @brief Return corresponding MPI_Datatype of given type
 * 
 * @details
 * This function is replaced by constant MPI_Datatype with -O3 optimization.
 * See https://rookiehpc.github.io/mpi/docs/mpi_datatype/index.html
 * for datatype and other constants in C (Click FORTRAN-2008 for Fortran).
 * 
 * @tparam T type
 * @return MPI_Datatype of T
 * 
 */
template <typename T>
static MPI_Datatype MPI_TYPE() noexcept {
  using std::is_same;
  if (is_same<T, char>::value) return MPI_CHAR;
  if (is_same<T, signed char>::value) return MPI_SIGNED_CHAR;
  if (is_same<T, unsigned char>::value) return MPI_UNSIGNED_CHAR;
  if (is_same<T, wchar_t>::value) return MPI_WCHAR;
  if (is_same<T, signed short>::value) return MPI_SHORT;
  if (is_same<T, unsigned short>::value) return MPI_UNSIGNED_SHORT;
  if (is_same<T, signed int>::value) return MPI_INT;
  if (is_same<T, unsigned int>::value) return MPI_UNSIGNED;
  if (is_same<T, signed long int>::value) return MPI_LONG;
  if (is_same<T, unsigned long int>::value) return MPI_UNSIGNED_LONG;
  if (is_same<T, signed long long int>::value) return MPI_LONG_LONG;
  if (is_same<T, unsigned long long int>::value) return MPI_UNSIGNED_LONG_LONG;
  if (is_same<T, float>::value) return MPI_FLOAT;
  if (is_same<T, double>::value) return MPI_DOUBLE;
  if (is_same<T, long double>::value) return MPI_LONG_DOUBLE;
  if (is_same<T, std::int8_t>::value) return MPI_INT8_T;
  if (is_same<T, std::int16_t>::value) return MPI_INT16_T;
  if (is_same<T, std::int32_t>::value) return MPI_INT32_T;
  if (is_same<T, std::int64_t>::value) return MPI_INT64_T;
  if (is_same<T, std::uint8_t>::value) return MPI_UINT8_T;
  if (is_same<T, std::uint16_t>::value) return MPI_UINT16_T;
  if (is_same<T, std::uint32_t>::value) return MPI_UINT32_T;
  if (is_same<T, std::uint64_t>::value) return MPI_UINT64_T;
  if (is_same<T, bool>::value) return MPI_C_BOOL;
  if (is_same<T, std::complex<float>>::value) return MPI_C_COMPLEX;
  if (is_same<T, std::complex<double>>::value) return MPI_C_DOUBLE_COMPLEX;
  if (is_same<T, std::complex<long double>>::value) return MPI_C_LONG_DOUBLE_COMPLEX;
#if defined(ARCH_X64)
  if (is_same<T, __float128>::value) {
    static MPI_Datatype float128_type = MPI_DATATYPE_NULL;
    if (float128_type == MPI_DATATYPE_NULL) {
        MPI_Type_contiguous(sizeof(__float128), MPI_BYTE, &float128_type);
        MPI_Type_commit(&float128_type);
    }
    return float128_type;
  }
#endif
}

#include <map>
#include <string>

// static std::string MPI_TYPENAME(MPI_Datatype dtype) noexcept {
//     static std::map<MPI_Datatype, std::string> mpi_typename {
//         // https://rookiehpc.github.io/mpi/docs/mpi_datatype/index.html
//         // C
//         {MPI_SIGNED_CHAR, "MPI_SIGNED_CHAR"},
//         {MPI_UNSIGNED_CHAR, "MPI_UNSIGNED_CHAR"},
//         {MPI_SHORT, "MPI_SHORT"},
//         {MPI_UNSIGNED_SHORT, "MPI_UNSIGNED_SHORT"},
//         {MPI_INT, "MPI_INT"},
//         {MPI_UNSIGNED, "MPI_UNSIGNED"},
//         {MPI_LONG, "MPI_LONG"},
//         {MPI_UNSIGNED_LONG, "MPI_UNSIGNED_LONG"},
//         {MPI_LONG_LONG_INT, "MPI_LONG_LONG_INT"},
//         {MPI_LONG_LONG, "MPI_LONG_LONG"},
//         {MPI_UNSIGNED_LONG_LONG, "MPI_UNSIGNED_LONG_LONG"},
//         {MPI_CHAR, "MPI_CHAR"},
//         {MPI_WCHAR, "MPI_WCHAR"},
//         {MPI_FLOAT, "MPI_FLOAT"},
//         {MPI_DOUBLE, "MPI_DOUBLE"},
//         {MPI_LONG_DOUBLE, "MPI_LONG_DOUBLE"},
//         {MPI_INT8_T, "MPI_INT8_T"},
//         {MPI_UINT8_T, "MPI_UINT8_T"},
//         {MPI_INT16_T, "MPI_INT16_T"},
//         {MPI_UINT16_T, "MPI_UINT16_T"},
//         {MPI_INT32_T, "MPI_INT32_T"},
//         {MPI_UINT32_T, "MPI_UINT32_T"},
//         {MPI_INT64_T, "MPI_INT64_T"},
//         {MPI_UINT64_T, "MPI_UINT64_T"},
//         {MPI_C_BOOL, "MPI_C_BOOL"},
//         {MPI_C_COMPLEX, "MPI_C_COMPLEX"},
//         {MPI_C_FLOAT_COMPLEX, "MPI_C_FLOAT_COMPLEX"},
//         {MPI_C_DOUBLE_COMPLEX, "MPI_C_DOUBLE_COMPLEX"},
//         {MPI_C_LONG_DOUBLE_COMPLEX, "MPI_C_LONG_DOUBLE_COMPLEX"},
//         {MPI_AINT, "MPI_AINT"},
//         {MPI_COUNT, "MPI_COUNT"},
//         {MPI_OFFSET, "MPI_OFFSET"},
//         {MPI_BYTE, "MPI_BYTE"},
//         {MPI_PACKED, "MPI_PACKED"},
//         {MPI_MINLOC, "MPI_MINLOC"},
//         {MPI_MAXLOC, "MPI_MAXLOC"},
//         {MPI_SHORT_INT, "MPI_SHORT_INT"},
//         {MPI_LONG_INT, "MPI_LONG_INT"},
//         {MPI_FLOAT_INT, "MPI_FLOAT_INT"},
//         {MPI_DOUBLE_INT, "MPI_DOUBLE_INT"},
//         {MPI_LONG_DOUBLE_INT, "MPI_LONG_DOUBLE_INT"},
//         {MPI_2INT, "MPI_2INT"},
//         // Fortran
//         {MPI_INTEGER, "MPI_INTEGER"},
//         {MPI_REAL, "MPI_REAL"},
//         {MPI_DOUBLE_PRECISION, "MPI_DOUBLE_PRECISION"},
//         {MPI_COMPLEX, "MPI_COMPLEX"},
//         {MPI_LOGICAL, "MPI_LOGICAL"},
//         {MPI_CHARACTER, "MPI_CHARACTER"},
//         // {MPI_ADDRESS_KIND, "MPI_ADDRESS_KIND"},
//         // {MPI_COUNT_KIND, "MPI_COUNT_KIND"},
//         // {MPI_OFFSET_KIND, "MPI_OFFSET_KIND"},
//         {MPI_AINT, "MPI_AINT"},
//         {MPI_BYTE, "MPI_BYTE"},
//         {MPI_PACKED, "MPI_PACKED"},
//         {MPI_INTEGER1, "MPI_INTEGER1"},
//         {MPI_INTEGER2, "MPI_INTEGER2"},
//         {MPI_INTEGER4, "MPI_INTEGER4"},
//         {MPI_INTEGER8, "MPI_INTEGER8"},
// #ifdef MPI_INTEGER16
//         {MPI_INTEGER16, "MPI_INTEGER16"},
// #endif
// #ifdef MPI_REAL2
//         {MPI_REAL2, "MPI_REAL2"},
// #endif
//         {MPI_REAL4, "MPI_REAL4"},
//         {MPI_REAL8, "MPI_REAL8"},
//         {MPI_REAL16, "MPI_REAL16"},
// #ifdef MPI_COMPLEX4
//         {MPI_COMPLEX4, "MPI_COMPLEX4"},
// #endif
//         {MPI_COMPLEX8, "MPI_COMPLEX8"},
//         {MPI_COMPLEX16, "MPI_COMPLEX16"},
//         {MPI_COMPLEX32, "MPI_COMPLEX32"},
//         {MPI_MINLOC, "MPI_MINLOC"},
//         {MPI_MAXLOC, "MPI_MAXLOC"},
//         {MPI_2REAL, "MPI_2REAL"},
//         {MPI_2DOUBLE_PRECISION, "MPI_2DOUBLE_PRECISION"},
//         {MPI_2INTEGER, "MPI_2INTEGER"},
//     };
//     static size_t user_count = 0;
//     try {
//         return mpi_typename.at(dtype);
//     } catch (std::out_of_range const &) {
//         char name[128];
//         snprintf(name, sizeof(name), "TYPE_%llu", user_count);
//         user_count += 1;
//         mpi_typename[dtype] = std::string(name);
//         return std::string(name);
//     }
// }

MYS_API static std::string MPI_COMMNAME(MPI_Comm comm) noexcept {
    static std::map<MPI_Comm, std::string> mpi_commname {
        {MPI_COMM_NULL, "MPI_COMM_NULL"},
        {MPI_COMM_SELF, "MPI_COMM_SELF"},
        {MPI_COMM_WORLD, "MPI_COMM_WORLD"},
    };
    static size_t user_count = 0;
    try {
        return mpi_commname.at(comm);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "COMM_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_commname[comm] = std::string(name);
        return std::string(name);
    }
}

MYS_API static std::string MPI_OPNAME(MPI_Op op) noexcept {
    static std::map<MPI_Op, std::string> mpi_opname {
        {MPI_OP_NULL, "MPI_OP_NULL"},
        {MPI_MAX, "MPI_MAX"},
        {MPI_MIN, "MPI_MIN"},
        {MPI_SUM, "MPI_SUM"},
        {MPI_PROD, "MPI_PROD"},
        {MPI_LAND, "MPI_LAND"},
        {MPI_BAND, "MPI_BAND"},
        {MPI_LOR, "MPI_LOR"},
        {MPI_BOR, "MPI_BOR"},
        {MPI_LXOR, "MPI_LXOR"},
        {MPI_BXOR, "MPI_BXOR"},
        {MPI_MINLOC, "MPI_MINLOC"},
        {MPI_MAXLOC, "MPI_MAXLOC"},
        {MPI_REPLACE, "MPI_REPLACE"},
    };
    static size_t user_count = 0;
    try {
        return mpi_opname.at(op);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "OP_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_opname[op] = std::string(name);
        return std::string(name);
    }
}

MYS_API static std::string MPI_REQUESTNAME(MPI_Request request) noexcept {
    static std::map<MPI_Request, std::string> mpi_requestname {
        {MPI_REQUEST_NULL, "MPI_REQUEST_NULL"},
    };
    static size_t user_count = 0;
    try {
        return mpi_requestname.at(request);
    } catch (std::out_of_range const &) {
        char name[128];
        snprintf(name, sizeof(name), "REQUEST_%llu", (unsigned long long)user_count);
        user_count += 1;
        mpi_requestname[request] = std::string(name);
        return std::string(name);
    }
}

#define ID_MPI_Abort	1
#define ID_MPI_Accumulate	2
#define ID_MPI_Add_error_class	3
#define ID_MPI_Add_error_code	4
#define ID_MPI_Add_error_string	5
#define ID_MPI_Address	6
#define ID_MPI_Aint_add	7
#define ID_MPI_Aint_diff	8
#define ID_MPI_Allgather	9
#define ID_MPI_Allgatherv	10
#define ID_MPI_Alloc_mem	11
#define ID_MPI_Allreduce	12
#define ID_MPI_Alltoall	13
#define ID_MPI_Alltoallv	14
#define ID_MPI_Alltoallw	15
#define ID_MPI_Attr_delete	16
#define ID_MPI_Attr_get	17
#define ID_MPI_Attr_put	18
#define ID_MPI_Barrier	19
#define ID_MPI_Bcast	20
#define ID_MPI_Bsend	21
#define ID_MPI_Bsend_init	22
#define ID_MPI_Buffer_attach	23
#define ID_MPI_Buffer_detach	24
#define ID_MPI_Cancel	25
#define ID_MPI_Cart_coords	26
#define ID_MPI_Cart_create	27
#define ID_MPI_Cart_get	28
#define ID_MPI_Cart_map	29
#define ID_MPI_Cart_rank	30
#define ID_MPI_Cart_shift	31
#define ID_MPI_Cart_sub	32
#define ID_MPI_Cartdim_get	33
#define ID_MPI_Close_port	34
#define ID_MPI_Comm_accept	35
#define ID_MPI_Comm_call_errhandler	36
#define ID_MPI_Comm_compare	37
#define ID_MPI_Comm_connect	38
#define ID_MPI_Comm_create	39
#define ID_MPI_Comm_create_errhandler	40
#define ID_MPI_Comm_create_group	41
#define ID_MPI_Comm_create_keyval	42
#define ID_MPI_Comm_delete_attr	43
#define ID_MPI_Comm_disconnect	44
#define ID_MPI_Comm_dup	45
#define ID_MPI_Comm_dup_with_info	46
#define ID_MPI_Comm_free	47
#define ID_MPI_Comm_free_keyval	48
#define ID_MPI_Comm_get_attr	49
#define ID_MPI_Comm_get_errhandler	50
#define ID_MPI_Comm_get_info	51
#define ID_MPI_Comm_get_name	52
#define ID_MPI_Comm_get_parent	53
#define ID_MPI_Comm_group	54
#define ID_MPI_Comm_idup	55
#define ID_MPI_Comm_join	56
#define ID_MPI_Comm_rank	57
#define ID_MPI_Comm_remote_group	58
#define ID_MPI_Comm_remote_size	59
#define ID_MPI_Comm_set_attr	60
#define ID_MPI_Comm_set_errhandler	61
#define ID_MPI_Comm_set_info	62
#define ID_MPI_Comm_set_name	63
#define ID_MPI_Comm_size	64
#define ID_MPI_Comm_split	65
#define ID_MPI_Comm_split_type	66
#define ID_MPI_Comm_test_inter	67
#define ID_MPI_Compare_and_swap	68
#define ID_MPI_Dims_create	69
#define ID_MPI_Dist_graph_create	70
#define ID_MPI_Dist_graph_create_adjacent	71
#define ID_MPI_Dist_graph_neighbors	72
#define ID_MPI_Dist_graph_neighbors_count	73
#define ID_MPI_Errhandler_create	74
#define ID_MPI_Errhandler_free	75
#define ID_MPI_Errhandler_get	76
#define ID_MPI_Errhandler_set	77
#define ID_MPI_Error_class	78
#define ID_MPI_Error_string	79
#define ID_MPI_Exscan	80
#define ID_MPI_Fetch_and_op	81
#define ID_MPI_File_call_errhandler	82
#define ID_MPI_File_close	83
#define ID_MPI_File_create_errhandler	84
#define ID_MPI_File_delete	85
#define ID_MPI_File_get_amode	86
#define ID_MPI_File_get_atomicity	87
#define ID_MPI_File_get_byte_offset	88
#define ID_MPI_File_get_errhandler	89
#define ID_MPI_File_get_group	90
#define ID_MPI_File_get_info	91
#define ID_MPI_File_get_position	92
#define ID_MPI_File_get_position_shared	93
#define ID_MPI_File_get_size	94
#define ID_MPI_File_get_type_extent	95
#define ID_MPI_File_get_view	96
#define ID_MPI_File_iread	97
#define ID_MPI_File_iread_all	98
#define ID_MPI_File_iread_at	99
#define ID_MPI_File_iread_at_all	100
#define ID_MPI_File_iread_shared	101
#define ID_MPI_File_iwrite	102
#define ID_MPI_File_iwrite_all	103
#define ID_MPI_File_iwrite_at	104
#define ID_MPI_File_iwrite_at_all	105
#define ID_MPI_File_iwrite_shared	106
#define ID_MPI_File_open	107
#define ID_MPI_File_preallocate	108
#define ID_MPI_File_read	109
#define ID_MPI_File_read_all	110
#define ID_MPI_File_read_all_begin	111
#define ID_MPI_File_read_all_end	112
#define ID_MPI_File_read_at	113
#define ID_MPI_File_read_at_all	114
#define ID_MPI_File_read_at_all_begin	115
#define ID_MPI_File_read_at_all_end	116
#define ID_MPI_File_read_ordered	117
#define ID_MPI_File_read_ordered_begin	118
#define ID_MPI_File_read_ordered_end	119
#define ID_MPI_File_read_shared	120
#define ID_MPI_File_seek	121
#define ID_MPI_File_seek_shared	122
#define ID_MPI_File_set_atomicity	123
#define ID_MPI_File_set_errhandler	124
#define ID_MPI_File_set_info	125
#define ID_MPI_File_set_size	126
#define ID_MPI_File_set_view	127
#define ID_MPI_File_sync	128
#define ID_MPI_File_write	129
#define ID_MPI_File_write_all	130
#define ID_MPI_File_write_all_begin	131
#define ID_MPI_File_write_all_end	132
#define ID_MPI_File_write_at	133
#define ID_MPI_File_write_at_all	134
#define ID_MPI_File_write_at_all_begin	135
#define ID_MPI_File_write_at_all_end	136
#define ID_MPI_File_write_ordered	137
#define ID_MPI_File_write_ordered_begin	138
#define ID_MPI_File_write_ordered_end	139
#define ID_MPI_File_write_shared	140
#define ID_MPI_Finalized	141
#define ID_MPI_Free_mem	142
#define ID_MPI_Gather	143
#define ID_MPI_Gatherv	144
#define ID_MPI_Get	145
#define ID_MPI_Get_accumulate	146
#define ID_MPI_Get_address	147
#define ID_MPI_Get_count	148
#define ID_MPI_Get_elements	149
#define ID_MPI_Get_elements_x	150
#define ID_MPI_Get_library_version	151
#define ID_MPI_Get_processor_name	152
#define ID_MPI_Get_version	153
#define ID_MPI_Graph_create	154
#define ID_MPI_Graph_get	155
#define ID_MPI_Graph_map	156
#define ID_MPI_Graph_neighbors	157
#define ID_MPI_Graph_neighbors_count	158
#define ID_MPI_Graphdims_get	159
#define ID_MPI_Grequest_complete	160
#define ID_MPI_Grequest_start	161
#define ID_MPI_Group_compare	162
#define ID_MPI_Group_difference	163
#define ID_MPI_Group_excl	164
#define ID_MPI_Group_free	165
#define ID_MPI_Group_incl	166
#define ID_MPI_Group_intersection	167
#define ID_MPI_Group_range_excl	168
#define ID_MPI_Group_range_incl	169
#define ID_MPI_Group_rank	170
#define ID_MPI_Group_size	171
#define ID_MPI_Group_translate_ranks	172
#define ID_MPI_Group_union	173
#define ID_MPI_Iallgather	174
#define ID_MPI_Iallgatherv	175
#define ID_MPI_Iallreduce	176
#define ID_MPI_Ialltoall	177
#define ID_MPI_Ialltoallv	178
#define ID_MPI_Ialltoallw	179
#define ID_MPI_Ibarrier	180
#define ID_MPI_Ibcast	181
#define ID_MPI_Ibsend	182
#define ID_MPI_Iexscan	183
#define ID_MPI_Igather	184
#define ID_MPI_Igatherv	185
#define ID_MPI_Improbe	186
#define ID_MPI_Imrecv	187
#define ID_MPI_Ineighbor_allgather	188
#define ID_MPI_Ineighbor_allgatherv	189
#define ID_MPI_Ineighbor_alltoall	190
#define ID_MPI_Ineighbor_alltoallv	191
#define ID_MPI_Ineighbor_alltoallw	192
#define ID_MPI_Info_create	193
#define ID_MPI_Info_delete	194
#define ID_MPI_Info_dup	195
#define ID_MPI_Info_free	196
#define ID_MPI_Info_get	197
#define ID_MPI_Info_get_nkeys	198
#define ID_MPI_Info_get_nthkey	199
#define ID_MPI_Info_get_valuelen	200
#define ID_MPI_Info_set	201
#define ID_MPI_Init	202
#define ID_MPI_Init_thread	203
#define ID_MPI_Initialized	204
#define ID_MPI_Intercomm_create	205
#define ID_MPI_Intercomm_merge	206
#define ID_MPI_Iprobe	207
#define ID_MPI_Irecv	208
#define ID_MPI_Ireduce	209
#define ID_MPI_Ireduce_scatter	210
#define ID_MPI_Ireduce_scatter_block	211
#define ID_MPI_Irsend	212
#define ID_MPI_Is_thread_main	213
#define ID_MPI_Iscan	214
#define ID_MPI_Iscatter	215
#define ID_MPI_Iscatterv	216
#define ID_MPI_Isend	217
#define ID_MPI_Issend	218
#define ID_MPI_Keyval_create	219
#define ID_MPI_Keyval_free	220
#define ID_MPI_Lookup_name	221
#define ID_MPI_Mprobe	222
#define ID_MPI_Mrecv	223
#define ID_MPI_Neighbor_allgather	224
#define ID_MPI_Neighbor_allgatherv	225
#define ID_MPI_Neighbor_alltoall	226
#define ID_MPI_Neighbor_alltoallv	227
#define ID_MPI_Neighbor_alltoallw	228
#define ID_MPI_Op_commutative	229
#define ID_MPI_Op_create	230
#define ID_MPI_Op_free	231
#define ID_MPI_Open_port	232
#define ID_MPI_Pack	233
#define ID_MPI_Pack_external	234
#define ID_MPI_Pack_external_size	235
#define ID_MPI_Pack_size	236
#define ID_MPI_Pcontrol	237
#define ID_MPI_Probe	238
#define ID_MPI_Publish_name	239
#define ID_MPI_Put	240
#define ID_MPI_Query_thread	241
#define ID_MPI_Raccumulate	242
#define ID_MPI_Recv	243
#define ID_MPI_Recv_init	244
#define ID_MPI_Reduce	245
#define ID_MPI_Reduce_local	246
#define ID_MPI_Reduce_scatter	247
#define ID_MPI_Reduce_scatter_block	248
#define ID_MPI_Register_datarep	249
#define ID_MPI_Request_free	250
#define ID_MPI_Request_get_status	251
#define ID_MPI_Rget	252
#define ID_MPI_Rget_accumulate	253
#define ID_MPI_Rput	254
#define ID_MPI_Rsend	255
#define ID_MPI_Rsend_init	256
#define ID_MPI_Scan	257
#define ID_MPI_Scatter	258
#define ID_MPI_Scatterv	259
#define ID_MPI_Send	260
#define ID_MPI_Send_init	261
#define ID_MPI_Sendrecv	262
#define ID_MPI_Sendrecv_replace	263
#define ID_MPI_Ssend	264
#define ID_MPI_Ssend_init	265
#define ID_MPI_Start	266
#define ID_MPI_Startall	267
#define ID_MPI_Status_set_cancelled	268
#define ID_MPI_Status_set_elements	269
#define ID_MPI_Status_set_elements_x	270
#define ID_MPI_Test	271
#define ID_MPI_Test_cancelled	272
#define ID_MPI_Testall	273
#define ID_MPI_Testany	274
#define ID_MPI_Testsome	275
#define ID_MPI_Topo_test	276
#define ID_MPI_Type_commit	277
#define ID_MPI_Type_contiguous	278
#define ID_MPI_Type_create_darray	279
#define ID_MPI_Type_create_f90_complex	280
#define ID_MPI_Type_create_f90_integer	281
#define ID_MPI_Type_create_f90_real	282
#define ID_MPI_Type_create_hindexed	283
#define ID_MPI_Type_create_hindexed_block	284
#define ID_MPI_Type_create_hvector	285
#define ID_MPI_Type_create_indexed_block	286
#define ID_MPI_Type_create_keyval	287
#define ID_MPI_Type_create_resized	288
#define ID_MPI_Type_create_struct	289
#define ID_MPI_Type_create_subarray	290
#define ID_MPI_Type_delete_attr	291
#define ID_MPI_Type_dup	292
#define ID_MPI_Type_extent	293
#define ID_MPI_Type_free	294
#define ID_MPI_Type_free_keyval	295
#define ID_MPI_Type_get_attr	296
#define ID_MPI_Type_get_contents	297
#define ID_MPI_Type_get_envelope	298
#define ID_MPI_Type_get_extent	299
#define ID_MPI_Type_get_extent_x	300
#define ID_MPI_Type_get_name	301
#define ID_MPI_Type_get_true_extent	302
#define ID_MPI_Type_get_true_extent_x	303
#define ID_MPI_Type_hindexed	304
#define ID_MPI_Type_hvector	305
#define ID_MPI_Type_indexed	306
#define ID_MPI_Type_lb	307
#define ID_MPI_Type_match_size	308
#define ID_MPI_Type_set_attr	309
#define ID_MPI_Type_set_name	310
#define ID_MPI_Type_size	311
#define ID_MPI_Type_size_x	312
#define ID_MPI_Type_struct	313
#define ID_MPI_Type_ub	314
#define ID_MPI_Type_vector	315
#define ID_MPI_Unpack	316
#define ID_MPI_Unpack_external	317
#define ID_MPI_Unpublish_name	318
#define ID_MPI_Wait	319
#define ID_MPI_Waitall	320
#define ID_MPI_Waitany	321
#define ID_MPI_Waitsome	322
#define ID_MPI_Win_allocate	323
#define ID_MPI_Win_allocate_shared	324
#define ID_MPI_Win_attach	325
#define ID_MPI_Win_call_errhandler	326
#define ID_MPI_Win_complete	327
#define ID_MPI_Win_create	328
#define ID_MPI_Win_create_dynamic	329
#define ID_MPI_Win_create_errhandler	330
#define ID_MPI_Win_create_keyval	331
#define ID_MPI_Win_delete_attr	332
#define ID_MPI_Win_detach	333
#define ID_MPI_Win_fence	334
#define ID_MPI_Win_flush	335
#define ID_MPI_Win_flush_all	336
#define ID_MPI_Win_flush_local	337
#define ID_MPI_Win_flush_local_all	338
#define ID_MPI_Win_free	339
#define ID_MPI_Win_free_keyval	340
#define ID_MPI_Win_get_attr	341
#define ID_MPI_Win_get_errhandler	342
#define ID_MPI_Win_get_group	343
#define ID_MPI_Win_get_info	344
#define ID_MPI_Win_get_name	345
#define ID_MPI_Win_lock	346
#define ID_MPI_Win_lock_all	347
#define ID_MPI_Win_post	348
#define ID_MPI_Win_set_attr	349
#define ID_MPI_Win_set_errhandler	350
#define ID_MPI_Win_set_info	351
#define ID_MPI_Win_set_name	352
#define ID_MPI_Win_shared_query	353
#define ID_MPI_Win_start	354
#define ID_MPI_Win_sync	355
#define ID_MPI_Win_test	356
#define ID_MPI_Win_unlock	357
#define ID_MPI_Win_unlock_all	358
#define ID_MPI_Win_wait	359
#define ID_MPI_Wtick	360
#define ID_MPI_Wtime	361
#define ID_MPI_Finalize	362
#define ID_COUNT	(ID_MPI_Finalize)+1

MYS_API static const char *MPI_NAME(int id)
{
    static const char* names[] = {
        "<undefined_placeholder>",
        "MPI_Abort",
        "MPI_Accumulate",
        "MPI_Add_error_class",
        "MPI_Add_error_code",
        "MPI_Add_error_string",
        "MPI_Address",
        "MPI_Aint_add",
        "MPI_Aint_diff",
        "MPI_Allgather",
        "MPI_Allgatherv",
        "MPI_Alloc_mem",
        "MPI_Allreduce",
        "MPI_Alltoall",
        "MPI_Alltoallv",
        "MPI_Alltoallw",
        "MPI_Attr_delete",
        "MPI_Attr_get",
        "MPI_Attr_put",
        "MPI_Barrier",
        "MPI_Bcast",
        "MPI_Bsend",
        "MPI_Bsend_init",
        "MPI_Buffer_attach",
        "MPI_Buffer_detach",
        "MPI_Cancel",
        "MPI_Cart_coords",
        "MPI_Cart_create",
        "MPI_Cart_get",
        "MPI_Cart_map",
        "MPI_Cart_rank",
        "MPI_Cart_shift",
        "MPI_Cart_sub",
        "MPI_Cartdim_get",
        "MPI_Close_port",
        "MPI_Comm_accept",
        "MPI_Comm_call_errhandler",
        "MPI_Comm_compare",
        "MPI_Comm_connect",
        "MPI_Comm_create",
        "MPI_Comm_create_errhandler",
        "MPI_Comm_create_group",
        "MPI_Comm_create_keyval",
        "MPI_Comm_delete_attr",
        "MPI_Comm_disconnect",
        "MPI_Comm_dup",
        "MPI_Comm_dup_with_info",
        "MPI_Comm_free",
        "MPI_Comm_free_keyval",
        "MPI_Comm_get_attr",
        "MPI_Comm_get_errhandler",
        "MPI_Comm_get_info",
        "MPI_Comm_get_name",
        "MPI_Comm_get_parent",
        "MPI_Comm_group",
        "MPI_Comm_idup",
        "MPI_Comm_join",
        "MPI_Comm_rank",
        "MPI_Comm_remote_group",
        "MPI_Comm_remote_size",
        "MPI_Comm_set_attr",
        "MPI_Comm_set_errhandler",
        "MPI_Comm_set_info",
        "MPI_Comm_set_name",
        "MPI_Comm_size",
        "MPI_Comm_split",
        "MPI_Comm_split_type",
        "MPI_Comm_test_inter",
        "MPI_Compare_and_swap",
        "MPI_Dims_create",
        "MPI_Dist_graph_create",
        "MPI_Dist_graph_create_adjacent",
        "MPI_Dist_graph_neighbors",
        "MPI_Dist_graph_neighbors_count",
        "MPI_Errhandler_create",
        "MPI_Errhandler_free",
        "MPI_Errhandler_get",
        "MPI_Errhandler_set",
        "MPI_Error_class",
        "MPI_Error_string",
        "MPI_Exscan",
        "MPI_Fetch_and_op",
        "MPI_File_call_errhandler",
        "MPI_File_close",
        "MPI_File_create_errhandler",
        "MPI_File_delete",
        "MPI_File_get_amode",
        "MPI_File_get_atomicity",
        "MPI_File_get_byte_offset",
        "MPI_File_get_errhandler",
        "MPI_File_get_group",
        "MPI_File_get_info",
        "MPI_File_get_position",
        "MPI_File_get_position_shared",
        "MPI_File_get_size",
        "MPI_File_get_type_extent",
        "MPI_File_get_view",
        "MPI_File_iread",
        "MPI_File_iread_all",
        "MPI_File_iread_at",
        "MPI_File_iread_at_all",
        "MPI_File_iread_shared",
        "MPI_File_iwrite",
        "MPI_File_iwrite_all",
        "MPI_File_iwrite_at",
        "MPI_File_iwrite_at_all",
        "MPI_File_iwrite_shared",
        "MPI_File_open",
        "MPI_File_preallocate",
        "MPI_File_read",
        "MPI_File_read_all",
        "MPI_File_read_all_begin",
        "MPI_File_read_all_end",
        "MPI_File_read_at",
        "MPI_File_read_at_all",
        "MPI_File_read_at_all_begin",
        "MPI_File_read_at_all_end",
        "MPI_File_read_ordered",
        "MPI_File_read_ordered_begin",
        "MPI_File_read_ordered_end",
        "MPI_File_read_shared",
        "MPI_File_seek",
        "MPI_File_seek_shared",
        "MPI_File_set_atomicity",
        "MPI_File_set_errhandler",
        "MPI_File_set_info",
        "MPI_File_set_size",
        "MPI_File_set_view",
        "MPI_File_sync",
        "MPI_File_write",
        "MPI_File_write_all",
        "MPI_File_write_all_begin",
        "MPI_File_write_all_end",
        "MPI_File_write_at",
        "MPI_File_write_at_all",
        "MPI_File_write_at_all_begin",
        "MPI_File_write_at_all_end",
        "MPI_File_write_ordered",
        "MPI_File_write_ordered_begin",
        "MPI_File_write_ordered_end",
        "MPI_File_write_shared",
        "MPI_Finalized",
        "MPI_Free_mem",
        "MPI_Gather",
        "MPI_Gatherv",
        "MPI_Get",
        "MPI_Get_accumulate",
        "MPI_Get_address",
        "MPI_Get_count",
        "MPI_Get_elements",
        "MPI_Get_elements_x",
        "MPI_Get_library_version",
        "MPI_Get_processor_name",
        "MPI_Get_version",
        "MPI_Graph_create",
        "MPI_Graph_get",
        "MPI_Graph_map",
        "MPI_Graph_neighbors",
        "MPI_Graph_neighbors_count",
        "MPI_Graphdims_get",
        "MPI_Grequest_complete",
        "MPI_Grequest_start",
        "MPI_Group_compare",
        "MPI_Group_difference",
        "MPI_Group_excl",
        "MPI_Group_free",
        "MPI_Group_incl",
        "MPI_Group_intersection",
        "MPI_Group_range_excl",
        "MPI_Group_range_incl",
        "MPI_Group_rank",
        "MPI_Group_size",
        "MPI_Group_translate_ranks",
        "MPI_Group_union",
        "MPI_Iallgather",
        "MPI_Iallgatherv",
        "MPI_Iallreduce",
        "MPI_Ialltoall",
        "MPI_Ialltoallv",
        "MPI_Ialltoallw",
        "MPI_Ibarrier",
        "MPI_Ibcast",
        "MPI_Ibsend",
        "MPI_Iexscan",
        "MPI_Igather",
        "MPI_Igatherv",
        "MPI_Improbe",
        "MPI_Imrecv",
        "MPI_Ineighbor_allgather",
        "MPI_Ineighbor_allgatherv",
        "MPI_Ineighbor_alltoall",
        "MPI_Ineighbor_alltoallv",
        "MPI_Ineighbor_alltoallw",
        "MPI_Info_create",
        "MPI_Info_delete",
        "MPI_Info_dup",
        "MPI_Info_free",
        "MPI_Info_get",
        "MPI_Info_get_nkeys",
        "MPI_Info_get_nthkey",
        "MPI_Info_get_valuelen",
        "MPI_Info_set",
        "MPI_Init",
        "MPI_Init_thread",
        "MPI_Initialized",
        "MPI_Intercomm_create",
        "MPI_Intercomm_merge",
        "MPI_Iprobe",
        "MPI_Irecv",
        "MPI_Ireduce",
        "MPI_Ireduce_scatter",
        "MPI_Ireduce_scatter_block",
        "MPI_Irsend",
        "MPI_Is_thread_main",
        "MPI_Iscan",
        "MPI_Iscatter",
        "MPI_Iscatterv",
        "MPI_Isend",
        "MPI_Issend",
        "MPI_Keyval_create",
        "MPI_Keyval_free",
        "MPI_Lookup_name",
        "MPI_Mprobe",
        "MPI_Mrecv",
        "MPI_Neighbor_allgather",
        "MPI_Neighbor_allgatherv",
        "MPI_Neighbor_alltoall",
        "MPI_Neighbor_alltoallv",
        "MPI_Neighbor_alltoallw",
        "MPI_Op_commutative",
        "MPI_Op_create",
        "MPI_Op_free",
        "MPI_Open_port",
        "MPI_Pack",
        "MPI_Pack_external",
        "MPI_Pack_external_size",
        "MPI_Pack_size",
        "MPI_Pcontrol",
        "MPI_Probe",
        "MPI_Publish_name",
        "MPI_Put",
        "MPI_Query_thread",
        "MPI_Raccumulate",
        "MPI_Recv",
        "MPI_Recv_init",
        "MPI_Reduce",
        "MPI_Reduce_local",
        "MPI_Reduce_scatter",
        "MPI_Reduce_scatter_block",
        "MPI_Register_datarep",
        "MPI_Request_free",
        "MPI_Request_get_status",
        "MPI_Rget",
        "MPI_Rget_accumulate",
        "MPI_Rput",
        "MPI_Rsend",
        "MPI_Rsend_init",
        "MPI_Scan",
        "MPI_Scatter",
        "MPI_Scatterv",
        "MPI_Send",
        "MPI_Send_init",
        "MPI_Sendrecv",
        "MPI_Sendrecv_replace",
        "MPI_Ssend",
        "MPI_Ssend_init",
        "MPI_Start",
        "MPI_Startall",
        "MPI_Status_set_cancelled",
        "MPI_Status_set_elements",
        "MPI_Status_set_elements_x",
        "MPI_Test",
        "MPI_Test_cancelled",
        "MPI_Testall",
        "MPI_Testany",
        "MPI_Testsome",
        "MPI_Topo_test",
        "MPI_Type_commit",
        "MPI_Type_contiguous",
        "MPI_Type_create_darray",
        "MPI_Type_create_f90_complex",
        "MPI_Type_create_f90_integer",
        "MPI_Type_create_f90_real",
        "MPI_Type_create_hindexed",
        "MPI_Type_create_hindexed_block",
        "MPI_Type_create_hvector",
        "MPI_Type_create_indexed_block",
        "MPI_Type_create_keyval",
        "MPI_Type_create_resized",
        "MPI_Type_create_struct",
        "MPI_Type_create_subarray",
        "MPI_Type_delete_attr",
        "MPI_Type_dup",
        "MPI_Type_extent",
        "MPI_Type_free",
        "MPI_Type_free_keyval",
        "MPI_Type_get_attr",
        "MPI_Type_get_contents",
        "MPI_Type_get_envelope",
        "MPI_Type_get_extent",
        "MPI_Type_get_extent_x",
        "MPI_Type_get_name",
        "MPI_Type_get_true_extent",
        "MPI_Type_get_true_extent_x",
        "MPI_Type_hindexed",
        "MPI_Type_hvector",
        "MPI_Type_indexed",
        "MPI_Type_lb",
        "MPI_Type_match_size",
        "MPI_Type_set_attr",
        "MPI_Type_set_name",
        "MPI_Type_size",
        "MPI_Type_size_x",
        "MPI_Type_struct",
        "MPI_Type_ub",
        "MPI_Type_vector",
        "MPI_Unpack",
        "MPI_Unpack_external",
        "MPI_Unpublish_name",
        "MPI_Wait",
        "MPI_Waitall",
        "MPI_Waitany",
        "MPI_Waitsome",
        "MPI_Win_allocate",
        "MPI_Win_allocate_shared",
        "MPI_Win_attach",
        "MPI_Win_call_errhandler",
        "MPI_Win_complete",
        "MPI_Win_create",
        "MPI_Win_create_dynamic",
        "MPI_Win_create_errhandler",
        "MPI_Win_create_keyval",
        "MPI_Win_delete_attr",
        "MPI_Win_detach",
        "MPI_Win_fence",
        "MPI_Win_flush",
        "MPI_Win_flush_all",
        "MPI_Win_flush_local",
        "MPI_Win_flush_local_all",
        "MPI_Win_free",
        "MPI_Win_free_keyval",
        "MPI_Win_get_attr",
        "MPI_Win_get_errhandler",
        "MPI_Win_get_group",
        "MPI_Win_get_info",
        "MPI_Win_get_name",
        "MPI_Win_lock",
        "MPI_Win_lock_all",
        "MPI_Win_post",
        "MPI_Win_set_attr",
        "MPI_Win_set_errhandler",
        "MPI_Win_set_info",
        "MPI_Win_set_name",
        "MPI_Win_shared_query",
        "MPI_Win_start",
        "MPI_Win_sync",
        "MPI_Win_test",
        "MPI_Win_unlock",
        "MPI_Win_unlock_all",
        "MPI_Win_wait",
        "MPI_Wtick",
        "MPI_Wtime",
        "MPI_Finalize",
        "<count_placeholder>",
    };
    if (id < 1 || id >= ID_COUNT) {
        return "<error>";
    }
    return names[id];
}
