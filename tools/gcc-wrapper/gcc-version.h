#if !defined(GCC_VERSIONS_H)
#define GCC_VERSIONS_H

#include "biggestint.h"

struct GCCVersion {
	biguint_t version; /* The major GCC release version. */
	biguint_t min_abi_version; /* The C++ ABI version used in GCC when this major release came out. */
	biguint_t max_abi_version; /* The C++ ABI version that major release had as the default when it reached EOL. */
	biguint_t stdc_version; /* The default C standard version GCC assumed for the -std= arg. */
	biguint_t stdc_max_version; /* The maximum C standard supported by that particular GCC major release. */
	biguint_t stdcxx_version; /* The default C++ standard version GCC assumed for the -std= arg. */
	biguint_t stdcxx_max_version; /* The maximum C++ standard supported by that particular GCC major release. */
};

enum GCCMajorVersion {
	GCC_4_9 = 49,
	GCC_5 = 5,
	GCC_6 = 6,
	GCC_7 = 7,
	GCC_8 = 8,
	GCC_9 = 9,
	GCC_10 = 10,
	GCC_11 = 11,
	GCC_12 = 12,
	GCC_13 = 13,
	GCC_14 = 14,
	GCC_15 = 15,
	GCC_16 = 16
};

enum GxxAbiVersion {
	GXX_ABI_2 = 2,
	GXX_ABI_8 = 8,
	GXX_ABI_9 = 9,
	GXX_ABI_10 = 10,
	GXX_ABI_11 = 11,
	GXX_ABI_12 = 12,
	GXX_ABI_13 = 13,
	GXX_ABI_14 = 14,
	GXX_ABI_16 = 16,
	GXX_ABI_17 = 17,
	GXX_ABI_18 = 18,
	GXX_ABI_19 = 19,
	GXX_ABI_20 = 20,
	GXX_ABI_21 = 21
};

enum GccStdVersion {
	GCC_STD_90 = 90,
	GCC_STD_11 = 11,
	GCC_STD_17 = 17,
	GCC_STD_23 = 23
};

enum GxxStdVersion {
	GXX_STD_98 = 98,
	GXX_STD_11 = 11,
	GXX_STD_14 = 14,
	GXX_STD_17 = 17,
	GXX_STD_20 = 20,
	GXX_STD_23 = 23,
	GXX_STD_26 = 26
};

static const struct GCCVersion GCC_VERSIONS[] = {
	{
		.version = GCC_4_9,
		.min_abi_version = GXX_ABI_2,
		.max_abi_version = GXX_ABI_2,
		.stdc_version = GCC_STD_90,
		.stdc_max_version = GCC_STD_11,
		.stdcxx_version = GXX_STD_11,
		.stdcxx_max_version = GXX_STD_11
	},
	{
		.version = GCC_5,
		.min_abi_version = GXX_ABI_8,
		.max_abi_version = GXX_ABI_9,
		.stdc_version = GCC_STD_11,
		.stdc_max_version = GCC_STD_11,
		.stdcxx_version = GXX_STD_98,
		.stdcxx_max_version = GXX_STD_17
	},
	{
		.version = GCC_6,
		.min_abi_version = GXX_ABI_10,
		.max_abi_version = GXX_ABI_10,
		.stdc_version = GCC_STD_11,
		.stdc_max_version = GCC_STD_11,
		.stdcxx_version = GXX_STD_14,
		.stdcxx_max_version = GXX_STD_17
	},
	{
		.version = GCC_7,
		.min_abi_version = GXX_ABI_11,
		.max_abi_version = GXX_ABI_11,
		.stdc_version = GCC_STD_11,
		.stdc_max_version = GCC_STD_11,
		.stdcxx_version = GXX_STD_14,
		.stdcxx_max_version = GXX_STD_17
	},
	{
		.version = GCC_8,
		.min_abi_version = GXX_ABI_12,
		.max_abi_version = GXX_ABI_13,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_14,
		.stdcxx_max_version = GXX_STD_17
	},
	{
		.version = GCC_9,
		.min_abi_version = GXX_ABI_13,
		.max_abi_version = GXX_ABI_13,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_14,
		.stdcxx_max_version = GXX_STD_17
	},
	{
		.version = GCC_10,
		.min_abi_version = GXX_ABI_14,
		.max_abi_version = GXX_ABI_14,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_14,
		.stdcxx_max_version = GXX_STD_20
	},
	{
		.version = GCC_11,
		.min_abi_version = GXX_ABI_16,
		.max_abi_version = GXX_ABI_16,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_17,
		.stdcxx_max_version = GXX_STD_23
	},
	{
		.version = GCC_12,
		.min_abi_version = GXX_ABI_17,
		.max_abi_version = GXX_ABI_17,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_17,
		.stdcxx_max_version = GXX_STD_23
	},
	{
		.version = GCC_13,
		.min_abi_version = GXX_ABI_18,
		.max_abi_version = GXX_ABI_18,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_17,
		.stdcxx_version = GXX_STD_17,
		.stdcxx_max_version = GXX_STD_23
	},
	{
		.version = GCC_14,
		.min_abi_version = GXX_ABI_19,
		.max_abi_version = GXX_ABI_19,
		.stdc_version = GCC_STD_17,
		.stdc_max_version = GCC_STD_23,
		.stdcxx_version = GXX_STD_17,
		.stdcxx_max_version = GXX_STD_26
	},
	{
		.version = GCC_15,
		.min_abi_version = GXX_ABI_20,
		.max_abi_version = GXX_ABI_20,
		.stdc_version = GCC_STD_23,
		.stdc_max_version = GCC_STD_23,
		.stdcxx_version = GXX_STD_17,
		.stdcxx_max_version = GXX_STD_26
	},
	{
		.version = GCC_16,
		.min_abi_version = GXX_ABI_21,
		.max_abi_version = GXX_ABI_21,
		.stdc_version = GCC_STD_23,
		.stdc_max_version = GCC_STD_23,
		.stdcxx_version = GXX_STD_20,
		.stdcxx_max_version = GXX_STD_26
	}
};

#endif /* GCC_VERSIONS_H */
