#ifndef FU_STD_INCLUDED
#define FU_STD_INCLUDED

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif (defined(__APPLE__) && defined(__MACH__))
#define PLATFORM_MAC
#else
#define PLATFORM_UNIX
#endif

#if defined(__cplusplus)
#define FU_EXTERN extern "C"
#else
#define FU_EXTERN extern
#endif

#if defined(_WIN32)
#if defined(__TINYC__)
#define __declspec(x) __attribute__((x))
#endif
#if defined(BUILD_LIBTYPE_SHARED)
#define FU_EXPORT FU_EXTERN __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
#elif defined(USE_LIBTYPE_SHARED)
#define FU_EXPORT FU_EXTERN __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
#endif
#else
#if defined(BUILD_LIBTYPE_SHARED)
#define FU_EXPORT FU_EXTERN __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
#endif
#endif

#ifndef FU_EXPORT
#define FU_EXPORT FU_EXTERN
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#if !defined(__cplusplus)
#if (defined(_MSC_VER) && _MSC_VER < 1800) || (!defined(_MSC_VER) && !defined(__STDC_VERSION__))
#ifndef true
#define true  (0 == 0)
#endif
#ifndef false
#define false (0 != 0)
#endif
typedef unsigned char bool;
#else
#include <stdbool.h>
#endif
#endif

#include <assert.h>

// Memory allocators
#ifndef FU_MEM_ALIGNMENT
#define FU_MEM_ALIGNMENT (2*sizeof(void *))
#endif

// Arenas
typedef struct fuArena {
	unsigned char* buffer;
	size_t         buffer_len;
	size_t         offset;
} fuArena;

void fu_arena_init_mem(fuArena* arena, void* buffer, size_t buffer_len);

void* fu_arena_alloc_aligned(fuArena* arena, size_t size, size_t align);
void* fu_arena_alloc(fuArena* arena, size_t size);

void* fu_arena_resize_aligned(fuArena* arena, void* old_memory, size_t old_size, size_t new_size, size_t align);
void* fu_arena_resize(fuArena* arena, void* old_memory, size_t old_size, size_t new_size);

void fu_arena_free_all(fuArena* arena);

// Dynamic library management
typedef void* fuDynLibHandle;
typedef void (*fuDynLibSymbol)(void);

fuDynLibHandle fu_dyn_lib_load(const char* path);
void fu_dyn_lib_free(fuDynLibHandle lib);
fuDynLibSymbol fu_dyn_lib_get_symbol_address(fuDynLibHandle lib, const char* symbol_name);

// File operations
int64_t fu_file_get_last_write_time(const char* path);
bool fu_file_copy(const char* src, const char* dst);
bool fu_file_delete(const char* path);


#ifdef FU_IMPLEMENTATION

#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif
#if defined(PLATFORM_MAC) || defined(PLATFORM_UNIX)
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#endif

bool fu_is_power_of_two(uintptr_t ptr) {
	if (ptr <= 0) return false;
	return !(ptr & (ptr - 1));
}

uintptr_t fu_align_forward(uintptr_t ptr, size_t align) {
	assert(fu_is_power_of_two(align));
	return (ptr + (align - 1)) & ~(align - 1);
}

// Arenas
void fu_arena_init_mem(fuArena* arena, void* buffer, size_t buffer_len) {
	arena->buffer = (unsigned char*)buffer;
	arena->buffer_len = buffer_len;
	arena->offset = 0;
}

void* fu_arena_alloc_aligned(fuArena* arena, size_t size, size_t align) {
	uintptr_t unaligned_offset = (uintptr_t)arena->buffer + (uintptr_t)arena->offset;
	uintptr_t offset = fu_align_forward(unaligned_offset, align);
	offset -= (uintptr_t)arena->buffer;

	if (offset + size <= arena->buffer_len) {
		void* ptr = &arena->buffer[offset];
		arena->offset = offset + size;

		// TODO: Use virtual mem with so we get default 0s?
		memset(ptr, 0, size);
		return ptr;
	}

	// No space, returning NULL for now
	return NULL;
}

void* fu_arena_alloc(fuArena* arena, size_t size) {
	return fu_arena_alloc_aligned(arena, size, FU_MEM_ALIGNMENT);
}

void* fu_arena_resize_aligned(fuArena* arena, void* old_memory, size_t old_size, size_t new_size, size_t align) {
	assert(fu_is_power_of_two(align));

	unsigned char* old_mem = (unsigned char*)old_memory;

	if (old_mem == NULL || old_size == 0) {
		return fu_arena_alloc_aligned(arena, new_size, align);
	}

	if (new_size == old_size)
		return old_memory;

	if (arena->buffer <= old_mem && old_mem < arena->buffer + arena->buffer_len) {
		void* new_memory = fu_arena_alloc_aligned(arena, new_size, align);
		if (!new_memory) return NULL;
		size_t copy_size = old_size < new_size ? old_size : new_size;
		memmove(new_memory, old_memory, copy_size);
		free(old_memory);
		return new_memory;
	}
	else {
		assert(0 && "Out of bounds of the arena");
		return NULL;
	}
}

void* fu_arena_resize(fuArena* arena, void* old_memory, size_t old_size, size_t new_size) {
	return fu_arena_resize_aligned(arena, old_memory, old_size, new_size, FU_MEM_ALIGNMENT);
}

void fu_arena_free_all(fuArena* arena) {
	arena->offset = 0;
}

// Dynamic library management
#ifdef PLATFORM_WINDOWS
fuDynLibHandle fu_dyn_lib_load(const char* path) {
	return (fuDynLibHandle)LoadLibraryA(path);
}

void fu_dyn_lib_free(fuDynLibHandle lib) {
	FreeLibrary((HMODULE)lib);
}

fuDynLibSymbol fu_dyn_lib_get_symbol_address(fuDynLibHandle lib, const char* symbol_name) {
	return (fuDynLibSymbol)GetProcAddress((HMODULE)lib, symbol_name);
}
#else
fuDynLibHandle fu_dyn_lib_load(const char* path) {
	return (fuDynLibHandle)dlopen(path, RTLD_LAZY);
}

void fu_dyn_lib_free(fuDynLibHandle lib) {
	dlclose(lib);
}

fuDynLibSymbol fu_dyn_lib_get_symbol_address(fuDynLibHandle lib, const char* symbol_name) {
	return (fuDynLibSymbol)dlsym(lib, symbol_name);
}
#endif

// File operations
#ifdef PLATFORM_WINDOWS
int64_t fu_file_get_last_write_time(const char* path) {
	WIN32_FILE_ATTRIBUTE_DATA file_info;
	if (!GetFileAttributesExA(path, GetFileExInfoStandard, &file_info)) {
		return 0;
	}

	ULARGE_INTEGER uli;
	uli.LowPart = file_info.ftLastWriteTime.dwLowDateTime;
	uli.HighPart = file_info.ftLastWriteTime.dwHighDateTime;
	return (int64_t)uli.QuadPart;
}

bool fu_file_copy(const char* src, const char* dst) {
	return CopyFileA(src, dst, FALSE) != 0;
}

bool fu_file_delete(const char* path) {
	return DeleteFileA(path) != 0;
}
#else
int64_t fu_file_get_last_write_time(const char* path) {
	struct stat file_stat;
	if (stat(path, &file_stat) != 0) {
		return 0;
	}

	// Return modification time in nanoseconds since epoch
	#ifdef PLATFORM_MAC
	return (int64_t)file_stat.st_mtimespec.tv_sec * 1000000000LL + file_stat.st_mtimespec.tv_nsec;
	#else
	return (int64_t)file_stat.st_mtim.tv_sec * 1000000000LL + file_stat.st_mtim.tv_nsec;
	#endif
}

bool fu_file_copy(const char* src, const char* dst) {
	int src_fd = open(src, O_RDONLY);
	if (src_fd < 0) return false;

	struct stat src_stat;
	if (fstat(src_fd, &src_stat) != 0) {
		close(src_fd);
		return false;
	}

	int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
	if (dst_fd < 0) {
		close(src_fd);
		return false;
	}

	char buffer[8192];
	ssize_t bytes_read;
	bool success = true;

	while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
		ssize_t bytes_written = write(dst_fd, buffer, bytes_read);
		if (bytes_written != bytes_read) {
			success = false;
			break;
		}
	}

	if (bytes_read < 0) {
		success = false;
	}

	close(src_fd);
	close(dst_fd);

	return success;
}

bool fu_file_delete(const char* path) {
	return unlink(path) == 0;
}
#endif

#endif // FU_IMPLEMENTATION
#endif // FU_STD_INCLUDED
