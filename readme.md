This project attempts to compare two directories (two SD cards) from a Nikon D750 to make sure all files are identical.

Nikon does not actually write identical files to the memory card, each card gets its own version of files. Therefore, this diff ignores a single byte that changes from 0x00 to 0x01. Unfortunately, it turns out, some files have more differences, so more work is required.
