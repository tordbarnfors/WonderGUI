---
title: 'Stream format'
weight: 10
---

# Stream format

*This document describes the format of this graphics stream.*



## Chunks and chunk headers

The stream is built up as a series of individual chunks of data. Each chunk starts with a header that states the type of chunk, size of the chunk and a number of flags determining how data in the chunk is stored.

The chunk header is 2-4 bytes long, depending on the size of the chunk and has the following format:



### First two bytes of header

| Bit    | Content                                                      |
| ------ | ------------------------------------------------------------ |
| 0..7   | Identifier (enum GfxChunkId)                                 |
| 8..10  | SPX storage format. An SPX can be stored as full 32 bits or 16 or 8 bits with optionally discarded binals to make it fit. See separate table for explanation |
| 11..15 | Size of chunk, excluding the header, in the range of 0-30 bytes. If value is set to 31 it means that the header is extended to 4 bytes and the value should be discarded. Instead the size of the chunk is saved in the next 2 bytes and in the range of 0-65535 bytes. |



### SPX storage format

| Value in Header | Meaning                               |
| :-------------- | ------------------------------------- |
| 0               | 32 bit signed values with binals.     |
| 1               | 16 bit unsigned values with binals.   |
| 2               | 16 bit signed values without binals.  |
| 3               | 8 bit unsigned values without binals. |
| 4-7             | Reserved for future use.              |

Any SPX storage format can be used in any header and affects all the data stored in SPX format, this means all CoordSPX, SizeSPX, RectSPX, BorderSPX and anything just saved as an SPX.

Also, most data in a chunk needs to be aligned on word-bundary, so a single SPX saved in an 8-bit format will need to be padded with an additional byte.

