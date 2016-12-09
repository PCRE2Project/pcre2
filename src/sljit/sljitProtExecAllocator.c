/*
 *    Stack-less Just-In-Time compiler
 *
 *    Copyright 2009-2012 Zoltan Herczeg (hzmester@freemail.hu). All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this list of
 *      conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this list
 *      of conditions and the following disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER(S) OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
   This file contains a simple executable memory allocator where the
   allocated regions are not writable and executable in the same time.

   This allocator usually uses more memory than sljitExecAllocator.
*/

#ifdef _WIN32

static SLJIT_INLINE void* alloc_chunk(sljit_uw size)
{
	return VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static SLJIT_INLINE void free_chunk(void* chunk, sljit_uw size)
{
	SLJIT_UNUSED_ARG(size);
	VirtualFree(chunk, 0, MEM_RELEASE);
}

static SLJIT_INLINE void enable_exec_permission(void* chunk, sljit_uw size)
{
	sljit_uw *uw_ptr = (sljit_uw *)ptr;

	VirtualProtect(chunk, size, PAGE_EXECUTE_READ, NULL);
}

#else

static SLJIT_INLINE void* alloc_chunk(sljit_uw size)
{
	void* retval;

#ifdef MAP_ANON
	retval = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
#else
	if (dev_zero < 0) {
		if (open_dev_zero())
			return NULL;
	}
	retval = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, dev_zero, 0);
#endif

	return (retval != MAP_FAILED) ? retval : NULL;
}

static SLJIT_INLINE void free_chunk(void* chunk, sljit_uw size)
{
	munmap(chunk, size);
}

static SLJIT_INLINE void enable_exec_permission(void* chunk, sljit_uw size)
{
	sljit_uw *uw_ptr = (sljit_uw *)chunk;

	mprotect(uw_ptr - 1, size + sizeof(sljit_uw), PROT_READ | PROT_EXEC);
}

#endif

/* --------------------------------------------------------------------- */
/*  Common functions                                                     */
/* --------------------------------------------------------------------- */

SLJIT_API_FUNC_ATTRIBUTE void* sljit_malloc_exec(sljit_uw size)
{
	sljit_uw *ptr = (sljit_uw *)alloc_chunk(size + sizeof (sljit_uw));

	*ptr = size;
	return (void*)(ptr + 1);
}

SLJIT_API_FUNC_ATTRIBUTE void sljit_free_exec(void* ptr)
{
	sljit_uw *uw_ptr = (sljit_uw *)ptr;

	free_chunk(uw_ptr - 1, uw_ptr[-1]);
}

SLJIT_API_FUNC_ATTRIBUTE void sljit_enable_exec(void* from, void *to)
{
	enable_exec_permission(from, ((sljit_u8 *)to) - ((sljit_u8 *)from));
}

SLJIT_API_FUNC_ATTRIBUTE void sljit_free_unused_memory_exec(void)
{
}
