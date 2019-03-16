#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

// File Format:
// 1 HEADER
// N COMMANDS
//
// Header
// ------
//
// 4 byte magic
// 1 byte version
// 1 byte incomplete flag
// 4 bytes chunk size
// 8 bytes file1 size
// 8 bytes file2 size
// 32 bytes file1 hash
// 32 bytes file2 hash
// 6 bytes padding
// = 96 bytes
//
// Data records
// --------
//
// 8 bytes position
// 8 bytes length
// {length} bytes payload

// 0xe2, 0xaa, 0xac, 0xfe (first 4 bytes of SHA256("bluedelta"))
#define HEADER_MAGIC 0xe2aaacfe
#define HEADER_VERSION 1
#define HEADER_INCOMPLETE_YES 1
#define HEADER_INCOMPLETE_NO 0

typedef struct file_header {
  uint32_t magic;           // see HEADER_MAGIC
  uint8_t  version;         // see HEADER_VERSION
  uint8_t  incomplete;      // see HEADER_INCOMPLETE_*
  uint32_t chunk_size;      // chunk size used for the file
  uint64_t file1_size;      // size of original file
  uint64_t file2_size;      // size of resulting file
  uint8_t  file1_hash[32];  // SHA256 hash of the original file
  uint8_t  file2_hash[32];  // SHA256 hash of the resulting file
  uint8_t  padding[6];      // padding to bring it to 96 bytes
} file_header_t;

// Records the differences between file1 and file2
typedef struct data_record {
  uint64_t position; // where to patch file1
  uint64_t length;   // how many bytes to overwrite
  uint8_t payload[]; // those bytes
} data_record_t;
#endif
