# Microsoft runtime libraries

The package carries app-local release CRT DLLs from the build compiler's
`VC/Redist/MSVC` directory. These are Microsoft distributable code, not covered
by the Civic 89 GPL grant. Redistribution is subject to the licensed Visual
Studio user's Microsoft Software License Terms and the distributable-code list
referenced in `Redist.txt`. The build/publishing operator must hold those rights.

Windows 11 supplies the Universal CRT. Debug CRTs, compiler binaries and SDK
files are not redistributed. App-local CRT servicing requires a new package.

See [Microsoft's redistribution documentation](https://learn.microsoft.com/en-us/cpp/windows/redistributing-visual-cpp-files).
