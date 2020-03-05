// Copyright 2011-2020, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "PsdPch.h"
#include "PsdMallocAllocator.h"

#include <malloc.h>


PSD_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void* MallocAllocator::DoAllocate(size_t size, size_t alignment)
{
#if defined(__GNUG__)
	return memalign(alignment, size);
#else
	return _aligned_malloc(size, alignment);
#endif
}


// ---------------------------------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------------------------------
void MallocAllocator::DoFree(void* ptr)
{
#if defined(__GNUG__)
	free(ptr);
#else
	_aligned_free(ptr);
#endif
}

PSD_NAMESPACE_END
